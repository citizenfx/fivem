#include <StdInc.h>

#include <CoreConsole.h>
#include <Hooking.h>
#include <Hooking.FlexStruct.h>
#include <Hooking.Stubs.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

static bool g_wheelFpsFixEnabled = false;

static uint32_t g_wheelCompressionOffset = 0;
static uint32_t g_wheelCompressionOldOffset = 0;
static uint32_t g_wheelFlagsOffset = 0;

static constexpr float kPhysicsStep = 1.0f / 60.0f;
static constexpr float kMaxCompressionWindow = 0.1f;
static constexpr float kMinCompressionWindow = kPhysicsStep * 0.25f;
static constexpr float kTeleportMargin = 1.0f;
static constexpr float kTeleportStepFactor = 4.0f;
static constexpr uint32_t kMaxFrameGap = 1;
static constexpr uint32_t kSweepIntervalFrames = 120;
static constexpr uint32_t kStateExpiryFrames = 600;
static constexpr size_t kStateSweepThreshold = 512;
static constexpr uint32_t kWheelResetFlag = 0x80000;
static constexpr uint32_t kWheelDummyTransitionFlag = 0x800000;
static constexpr int kMaxTrackedWheels = 10;
static constexpr size_t kLocalsSize = 496;

static constexpr int32_t kColliderInvInertia = 32;
static constexpr int32_t kColliderInvMass = 44;
static constexpr int32_t kColliderAxisX = 48;
static constexpr int32_t kColliderAxisY = 64;
static constexpr int32_t kColliderAxisZ = 80;
static constexpr int32_t kColliderPosition = 96;
static constexpr int32_t kColliderVelocity = 112;
static constexpr int32_t kColliderAngVelocity = 128;
static constexpr int32_t kColliderMass = 300;

static constexpr int32_t kLocalsVelocity = 0;
static constexpr int32_t kLocalsAngVelocity = 80;

namespace
{
struct IntegrationState
{
	bool hasRun = false;
	int numWheels = 0;
	const void* firstWheel = nullptr;
	float mass = 0.0f;
	uint32_t lastFrame = 0;
	uint32_t runCounter = 0;
	uint32_t callsThisFrame = 0;
	uint32_t callsPrevFrame = 0;

	float accum = kPhysicsStep;
	float sinceLastRun = 0.0f;

	float lastPosition[3] = {};
	float lastCompression[kMaxTrackedWheels] = {};
	float heldAccel[3] = {};
	float heldAngAccel[3] = {};

	unsigned char snapshot[kLocalsSize] = {};

	void Reset()
	{
		hasRun = false;
		numWheels = 0;
		accum = kPhysicsStep;
		sinceLastRun = 0.0f;
	}
};

struct RunContext
{
	IntegrationState* state = nullptr;
	const hook::FlexStruct* locals = nullptr;
	float frameStep = 0.0f;
	float internalAccel[3] = {};
	float internalAngAccel[3] = {};
};
}

static uint32_t g_lastSweepFrame = 0;

static std::mutex g_stateMutex;
static std::unordered_map<const void*, std::shared_ptr<IntegrationState>> g_state;

static thread_local RunContext* t_runContext = nullptr;

static void (*g_origProcessIntegrationTask)(hook::FlexStruct* collider, const float* internalForce, const float* internalTorque, hook::FlexStruct* locals, hook::FlexStruct** wheels, int numWheels, float maxVelInGear, int gear, float timeStep, uint32_t frameCount, float gravity);
static void (*g_origProcessResultInternal)(hook::FlexStruct* collider, hook::FlexStruct* locals, hook::FlexStruct** wheels, int numWheels, float maxVelInGear, int gear, float timeStep, uint32_t frameCount, float* forceApplied, float gravity);

namespace
{
class RunScope
{
public:
	explicit RunScope(RunContext* context)
		: m_previous(t_runContext)
	{
		t_runContext = context;
	}

	~RunScope()
	{
		t_runContext = m_previous;
	}

	RunScope(const RunScope&) = delete;
	RunScope& operator=(const RunScope&) = delete;

private:
	RunContext* m_previous;
};

class CompressionHistoryScope
{
public:
	CompressionHistoryScope(hook::FlexStruct** wheels, int numWheels, const float* lastCompression, float windowScale)
		: m_wheels(wheels), m_numWheels(numWheels)
	{
		for (int i = 0; i < m_numWheels; i++)
		{
			const float compression = m_wheels[i]->Get<float>(g_wheelCompressionOffset);
			float& compressionOld = m_wheels[i]->At<float>(g_wheelCompressionOldOffset);

			m_saved[i] = compressionOld;
			compressionOld = compression - (compression - lastCompression[i]) * windowScale;
		}
	}

	~CompressionHistoryScope()
	{
		for (int i = 0; i < m_numWheels; i++)
		{
			m_wheels[i]->Set<float>(g_wheelCompressionOldOffset, m_saved[i]);
		}
	}

	CompressionHistoryScope(const CompressionHistoryScope&) = delete;
	CompressionHistoryScope& operator=(const CompressionHistoryScope&) = delete;

private:
	hook::FlexStruct** m_wheels;
	int m_numWheels;
	float m_saved[kMaxTrackedWheels] = {};
};
}

static void ComputeInternalAccel(hook::FlexStruct* collider, const float* internalForce, const float* internalTorque, float* accel, float* angAccel)
{
	const float invMass = collider->Get<float>(kColliderInvMass);

	for (int i = 0; i < 3; i++)
	{
		accel[i] = internalForce[i] * invMass;
	}

	const float* invInertia = &collider->At<float>(kColliderInvInertia);
	const float* axisX = &collider->At<float>(kColliderAxisX);
	const float* axisY = &collider->At<float>(kColliderAxisY);
	const float* axisZ = &collider->At<float>(kColliderAxisZ);

	const float projX = (internalTorque[0] * axisX[0] + internalTorque[1] * axisX[1] + internalTorque[2] * axisX[2]) * invInertia[0];
	const float projY = (internalTorque[0] * axisY[0] + internalTorque[1] * axisY[1] + internalTorque[2] * axisY[2]) * invInertia[1];
	const float projZ = (internalTorque[0] * axisZ[0] + internalTorque[1] * axisZ[1] + internalTorque[2] * axisZ[2]) * invInertia[2];

	for (int i = 0; i < 3; i++)
	{
		angAccel[i] = projX * axisX[i] + projY * axisY[i] + projZ * axisZ[i];
	}
}

static bool HasTeleported(const IntegrationState& state, hook::FlexStruct* collider, float timeStep)
{
	const float* position = &collider->At<float>(kColliderPosition);
	const float* velocity = &collider->At<float>(kColliderVelocity);

	const float dx = position[0] - state.lastPosition[0];
	const float dy = position[1] - state.lastPosition[1];
	const float dz = position[2] - state.lastPosition[2];
	const float speed = std::sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2]);
	const float limit = speed * timeStep * kTeleportStepFactor + kTeleportMargin;

	return dx * dx + dy * dy + dz * dz > limit * limit;
}

static std::shared_ptr<IntegrationState> AcquireState(const void* collider, uint32_t frameCount, int numWheels, const void* firstWheel, float mass)
{
	std::lock_guard<std::mutex> lock(g_stateMutex);

	if (g_state.size() > kStateSweepThreshold && (frameCount - g_lastSweepFrame) > kSweepIntervalFrames)
	{
		g_lastSweepFrame = frameCount;

		for (auto it = g_state.begin(); it != g_state.end();)
		{
			if ((frameCount - it->second->lastFrame) > kStateExpiryFrames)
			{
				it = g_state.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	auto& entry = g_state[collider];

	if (!entry)
	{
		entry = std::make_shared<IntegrationState>();
		entry->lastFrame = frameCount;
	}

	IntegrationState& state = *entry;

	if ((frameCount - state.lastFrame) > kMaxFrameGap)
	{
		state.Reset();
		state.callsThisFrame = 0;
		state.callsPrevFrame = 0;
	}
	else if (state.hasRun && (state.numWheels != numWheels || state.firstWheel != firstWheel || state.mass != mass))
	{
		state.Reset();
	}

	if (frameCount != state.lastFrame)
	{
		state.callsPrevFrame = state.callsThisFrame;
		state.callsThisFrame = 0;
	}

	state.callsThisFrame++;
	state.lastFrame = frameCount;

	return entry;
}

static void ProcessResultInternal(hook::FlexStruct* collider, hook::FlexStruct* locals, hook::FlexStruct** wheels, int numWheels, float maxVelInGear, int gear, float timeStep, uint32_t frameCount, float* forceApplied, float gravity)
{
	RunContext* run = t_runContext;

	if (!run || run->locals != locals)
	{
		g_origProcessResultInternal(collider, locals, wheels, numWheels, maxVelInGear, gear, timeStep, frameCount, forceApplied, gravity);
		return;
	}

	IntegrationState* state = run->state;

	const float* velocity = &collider->At<float>(kColliderVelocity);
	const float* angVelocity = &collider->At<float>(kColliderAngVelocity);
	float* localsVelocity = &locals->At<float>(kLocalsVelocity);
	float* localsAngVelocity = &locals->At<float>(kLocalsAngVelocity);

	const float invWindow = 1.0f / timeStep;
	const float scale = run->frameStep * invWindow;

	for (int i = 0; i < 3; i++)
	{
		state->heldAccel[i] = (localsVelocity[i] - velocity[i]) * invWindow - run->internalAccel[i];
		state->heldAngAccel[i] = (localsAngVelocity[i] - angVelocity[i]) * invWindow - run->internalAngAccel[i];
	}

	std::memcpy(state->snapshot, locals, kLocalsSize);

	for (int i = 0; i < 3; i++)
	{
		localsVelocity[i] = velocity[i] + (localsVelocity[i] - velocity[i]) * scale;
		localsAngVelocity[i] = angVelocity[i] + (localsAngVelocity[i] - angVelocity[i]) * scale;
	}

	g_origProcessResultInternal(collider, locals, wheels, numWheels, maxVelInGear, gear, run->frameStep, frameCount, forceApplied, gravity);
}

static void ProcessIntegrationTask(hook::FlexStruct* collider, const float* internalForce, const float* internalTorque, hook::FlexStruct* locals, hook::FlexStruct** wheels, int numWheels, float maxVelInGear, int gear, float timeStep, uint32_t frameCount, float gravity)
{
	auto callOriginal = [&]()
	{
		g_origProcessIntegrationTask(collider, internalForce, internalTorque, locals, wheels, numWheels, maxVelInGear, gear, timeStep, frameCount, gravity);
	};

	if (!g_wheelFpsFixEnabled || !(timeStep > 0.0f) || timeStep >= kPhysicsStep || numWheels <= 0 || numWheels > kMaxTrackedWheels)
	{
		callOriginal();
		return;
	}

	bool dummyTransition = false;
	bool wheelsReset = false;

	for (int i = 0; i < numWheels; i++)
	{
		const uint32_t flags = wheels[i]->Get<uint32_t>(g_wheelFlagsOffset);

		dummyTransition = dummyTransition || (flags & kWheelDummyTransitionFlag) != 0;
		wheelsReset = wheelsReset || (flags & kWheelResetFlag) != 0;
	}

	if (dummyTransition)
	{
		callOriginal();
		return;
	}

	const float mass = collider->Get<float>(kColliderMass);

	const std::shared_ptr<IntegrationState> state = AcquireState(collider, frameCount, numWheels, wheels[0], mass);

	if (state->callsThisFrame >= 2 || state->callsPrevFrame >= 2)
	{
		state->Reset();
		callOriginal();
		return;
	}

	if (wheelsReset || (state->hasRun && HasTeleported(*state, collider, timeStep)))
	{
		state->Reset();
	}

	std::memcpy(state->lastPosition, &collider->At<float>(kColliderPosition), sizeof(state->lastPosition));

	const float elapsedSinceRun = state->sinceLastRun;

	state->accum += timeStep;

	RunContext run;
	run.state = state.get();
	run.locals = locals;
	run.frameStep = timeStep;
	ComputeInternalAccel(collider, internalForce, internalTorque, run.internalAccel, run.internalAngAccel);

	if (state->hasRun && state->accum < kPhysicsStep)
	{
		state->sinceLastRun += timeStep;

		const float* velocity = &collider->At<float>(kColliderVelocity);
		const float* angVelocity = &collider->At<float>(kColliderAngVelocity);
		float* localsVelocity = &locals->At<float>(kLocalsVelocity);
		float* localsAngVelocity = &locals->At<float>(kLocalsAngVelocity);

		std::memcpy(locals, state->snapshot, kLocalsSize);

		for (int i = 0; i < 3; i++)
		{
			localsVelocity[i] = velocity[i] + (state->heldAccel[i] + run.internalAccel[i]) * timeStep;
			localsAngVelocity[i] = angVelocity[i] + (state->heldAngAccel[i] + run.internalAngAccel[i]) * timeStep;
		}

		g_origProcessResultInternal(collider, locals, wheels, numWheels, maxVelInGear, gear, timeStep, frameCount, localsVelocity, gravity);
		return;
	}

	state->accum = std::max(state->accum - kPhysicsStep, 0.0f);

	{
		std::optional<CompressionHistoryScope> history;

		if (state->hasRun && elapsedSinceRun >= kMinCompressionWindow && elapsedSinceRun <= kMaxCompressionWindow)
		{
			history.emplace(wheels, numWheels, state->lastCompression, kPhysicsStep / elapsedSinceRun);
		}

		for (int i = 0; i < numWheels; i++)
		{
			state->lastCompression[i] = wheels[i]->Get<float>(g_wheelCompressionOffset);
		}

		RunScope scope(&run);
		g_origProcessIntegrationTask(collider, internalForce, internalTorque, locals, wheels, numWheels, maxVelInGear, gear, kPhysicsStep, state->runCounter++, gravity);
	}

	state->hasRun = true;
	state->numWheels = numWheels;
	state->firstWheel = wheels[0];
	state->mass = mass;
	state->sinceLastRun = timeStep;
}

static HookFunction hookFunction([]()
{
	static ConVar<bool> wheelFpsFixEnable("game_enableFrameRateIndependentWheelPhysics", ConVar_Replicated, true, &g_wheelFpsFixEnabled);

	auto compressionLoc = hook::get_pattern<char>("F3 0F 10 83 ? ? ? ? 45 0F 57 ? F3 0F 11 ? ? ? 00 00 F3 0F 5C");
	g_wheelCompressionOffset = *reinterpret_cast<uint32_t*>(compressionLoc + 4);
	g_wheelCompressionOldOffset = *reinterpret_cast<uint32_t*>(compressionLoc + 16);
	g_wheelFlagsOffset = *hook::get_pattern<uint32_t>("8B 8E ? ? ? ? 8B C1 C1 E8 13 A8 01", 2);

	g_origProcessResultInternal = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 4C 8D 9C 24 C8 05 00 00 41 0F 28 73 E8")), ProcessResultInternal);
	g_origProcessIntegrationTask = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 4C 8D 9C 24 ? ? ? ? B0 ? F3 0F 10 45")), ProcessIntegrationTask);
});
