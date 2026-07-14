#include <StdInc.h>

#include <CoreConsole.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <algorithm>
#include <mutex>
#include <unordered_map>

#include "CrossBuildRuntime.h"

static bool g_wheelFpsFixEnabled = false;

static constexpr float kPhysicsStep = 1.0f / 60.0f;

namespace
{
struct WheelIntegrationLocals
{
	float outputVel[4];
};

constexpr int kMaxIntegrationWheels = 10;

struct IntegrationState
{
	float accum = kPhysicsStep; // full step so the first call always runs

	// Time and per-wheel compression at the last step, to rebuild the damper delta over the
	// real window (else compression speed is under-measured at high FPS and the car bounces)
	float sinceLastRun = 0.0f;
	float lastCompression[kMaxIntegrationWheels];

	// Last step's output force; skipped frames report this held value
	float lastOutput[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// m_Force/m_Torque deltas the last run added to a rigid collider; skipped frames re-apply them
	float heldForce[8] = {};
	uint32_t heldForceFrame = 0;

	// Audio/vfx slip readouts (eff/fwd/side)
	float slipTarget[kMaxIntegrationWheels][3] = {};
	float slipDisplay[kMaxIntegrationWheels][3] = {};

	bool hasRunOnce = false;
	uint32_t lastFrame = 0;
};

constexpr uint32_t kSlipFieldOffsets[3] = { 0, 8, 12 };
}

static uint32_t g_wheelCompressionOffset;

static char kColliderTypeOffset;
static uint32_t kColliderForceOffset;
static uint32_t kWheelDynamicFlagsOffset;
static uint32_t g_wheelEffSlipOffset;

static std::mutex g_stateMutex;
static std::unordered_map<void*, IntegrationState> g_state;

static void (*g_origProcessIntegrationTask)(void* collider, void* internalForce, void* internalTorque, WheelIntegrationLocals* locals, void* ppWheels, int numWheels, float maxVelInGear, int gear, float timeStep, uint32_t frameCount, float gravity);

static void ProcessIntegrationTask(void* collider, void* internalForce, void* internalTorque, WheelIntegrationLocals* locals, void* ppWheels, int numWheels, float maxVelInGear, int gear, float timeStep, uint32_t frameCount, float gravity)
{
	if (!g_wheelFpsFixEnabled || timeStep <= 0.0f)
	{
		g_origProcessIntegrationTask(collider, internalForce, internalTorque, locals, ppWheels, numWheels, maxVelInGear, gear, timeStep, frameCount, gravity);
		return;
	}

	int stepsToRun;
	IntegrationState* state;

	{
		std::lock_guard<std::mutex> lock(g_stateMutex);

		// Drop entries for colliders not seen in a while (deleted vehicles)
		if (g_state.size() > 512)
		{
			for (auto it = g_state.begin(); it != g_state.end();)
			{
				if ((frameCount - it->second.lastFrame) > 600)
				{
					it = g_state.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		state = &g_state[collider];
		state->lastFrame = frameCount;

		// Cap at two steps so a hitch can't queue a burst against one stale contact sample
		state->accum = std::min(state->accum + timeStep, 2.0f * kPhysicsStep);
		state->sinceLastRun += timeStep;

		stepsToRun = static_cast<int>(state->accum / kPhysicsStep);
		state->accum -= static_cast<float>(stepsToRun) * kPhysicsStep;
	}

	if (stepsToRun == 0)
	{
		// Skipped frame: hold the last step's output (the only thing read back from locals)
		locals->outputVel[0] = state->lastOutput[0];
		locals->outputVel[1] = state->lastOutput[1];
		locals->outputVel[2] = state->lastOutput[2];
		locals->outputVel[3] = state->lastOutput[3];

		// Re-apply the last run's force so the step's impulse spreads across the window
		if ((frameCount - state->heldForceFrame) <= 4 && *reinterpret_cast<const int*>(static_cast<const char*>(collider) + kColliderTypeOffset) == 0)
		{
			auto forceAccum = reinterpret_cast<float*>(static_cast<char*>(collider) + kColliderForceOffset);

			for (int lane = 0; lane < 8; lane++)
			{
				if (lane != 3 && lane != 7)
				{
					forceAccum[lane] += state->heldForce[lane];
				}
			}
		}

		auto wheels = static_cast<char* const*>(ppWheels);
		int trackedWheels = std::min(numWheels, kMaxIntegrationWheels);

		// Contact-flag shuffle the skipped run would have done (WF_HIT_PREV = WF_HIT, WF_HIT
		// cleared); stale WF_HIT makes the still-vehicle contact filter reject ground contacts
		for (int i = 0; i < trackedWheels; i++)
		{
			auto flags = reinterpret_cast<uint32_t*>(wheels[i] + kWheelDynamicFlagsOffset);
			uint32_t value = *flags;
			*flags = (value & ~3u) | ((value & 1u) << 1);
		}

		// Chase the slip readouts toward the last runs values
		if (state->hasRunOnce)
		{
			float alpha = std::min(timeStep / (2.0f * kPhysicsStep), 1.0f);

			for (int i = 0; i < trackedWheels; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					float& display = state->slipDisplay[i][j];
					display += (state->slipTarget[i][j] - display) * alpha;
					*reinterpret_cast<float*>(wheels[i] + g_wheelEffSlipOffset + kSlipFieldOffsets[j]) = display;
				}
			}
		}

		return;
	}

	auto wheels = static_cast<char* const*>(ppWheels);
	int trackedWheels = std::min(numWheels, kMaxIntegrationWheels);

	// Rebuild m_fCompressionOld so the damper delta spans the true window since the last step
	// (at 60fps sinceLastRun == kPhysicsStep and this is a no-op). Skip after long gaps or
	// teleports, where stale history would inject a fake compression spike.
	if (state->hasRunOnce && state->sinceLastRun > 0.0f && state->sinceLastRun <= 0.1f)
	{
		for (int i = 0; i < trackedWheels; i++)
		{
			auto compression = reinterpret_cast<float*>(wheels[i] + g_wheelCompressionOffset);
			float trueDelta = compression[0] - state->lastCompression[i];
			float damperDelta = trueDelta * (kPhysicsStep / state->sinceLastRun);

			compression[1] = compression[0] - damperDelta;
		}
	}

	// Rigid colliders accumulate into m_Force/m_Torque, integrated by the simulator with each
	// slice's real dt; snapshot to capture what the run adds (other types apply immediately)
	int colliderType = *reinterpret_cast<const int*>(static_cast<const char*>(collider) + kColliderTypeOffset);
	auto forceAccum = reinterpret_cast<float*>(static_cast<char*>(collider) + kColliderForceOffset);

	float forceBefore[8];

	if (colliderType == 0)
	{
		memcpy(forceBefore, forceAccum, sizeof(forceBefore));
	}

	// At most two steps; each rereads collider state, chaining like native <= 60fps slices
	for (int i = 0; i < stepsToRun; i++)
	{
		g_origProcessIntegrationTask(collider, internalForce, internalTorque, locals, ppWheels, numWheels, maxVelInGear, gear, kPhysicsStep, frameCount, gravity);
	}

	if (colliderType == 0)
	{
		// xyz of m_Force (0-2) and m_Torque (4-6); leave the W lanes alone.
		for (int lane = 0; lane < 8; lane++)
		{
			state->heldForce[lane] = (lane != 3 && lane != 7) ? forceAccum[lane] - forceBefore[lane] : 0.0f;
		}

		state->heldForceFrame = frameCount;
	}

	for (int i = 0; i < trackedWheels; i++)
	{
		state->lastCompression[i] = *reinterpret_cast<float*>(wheels[i] + g_wheelCompressionOffset);
	}

	for (int i = 0; i < trackedWheels; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			auto field = reinterpret_cast<float*>(wheels[i] + g_wheelEffSlipOffset + kSlipFieldOffsets[j]);
			state->slipTarget[i][j] = *field;

			if (state->hasRunOnce)
			{
				float alpha = std::min(timeStep / (2.0f * kPhysicsStep), 1.0f);
				float& display = state->slipDisplay[i][j];
				display += (state->slipTarget[i][j] - display) * alpha;
				*field = display;
			}
			else
			{
				state->slipDisplay[i][j] = *field;
			}
		}
	}

	state->lastOutput[0] = locals->outputVel[0];
	state->lastOutput[1] = locals->outputVel[1];
	state->lastOutput[2] = locals->outputVel[2];
	state->lastOutput[3] = locals->outputVel[3];

	state->hasRunOnce = true;
	state->sinceLastRun = 0.0f;
}

static HookFunction hookFunction([]()
{
	static ConVar<bool> wheelFpsFixEnable("game_enableFrameRateIndependentWheelPhysics", ConVar_Replicated, true, &g_wheelFpsFixEnabled);

	g_wheelCompressionOffset = *hook::get_pattern<uint32_t>("45 0F 57 ? F3 0F 11 ? ? ? 00 00 F3 0F 5C", 8);
	kColliderTypeOffset = *hook::get_pattern<char>("8B 4B ? 8D 41 ? 83 F8 ? 76 ? 0F 28 43", 2);
	kColliderForceOffset = *hook::get_pattern<uint32_t>("0F 28 AB ? ? ? ? F3 0F 10 B3", 3);
	kWheelDynamicFlagsOffset = *hook::get_pattern<uint32_t>(xbr::IsGameBuildOrGreater<2802>() ? "83 A3 ? ? ? ? ? F6 83 ? ? ? ? ? 74 ? 83 8B" : "83 A3 ? ? ? ? ? 44 84 B3", 2);
	g_wheelEffSlipOffset = *hook::get_pattern<uint32_t>("0F 2F 81 ? ? ? ? 0F 28 F1", 3);
	g_origProcessIntegrationTask = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 4C 8D 9C 24 ? ? ? ? B0 ? F3 0F 10 45")), ProcessIntegrationTask);
});
