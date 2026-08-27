/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#include <GamePrimitives.h>
#include <HostSharedData.h>
#include <Hooking.h>
#include <ICoreGameInit.h>
#include <NuiGameState.h>
#include <nutsnbolts.h>

static constexpr float kRadToDeg = 57.29577951308232f;

static float* g_actorPos;

static HostSharedData<NuiGameState> g_nuiGameState("CfxNuiGameState");

static void StoreVector(float (&out)[3], float x, float y, float z)
{
	out[0] = x;
	out[1] = y;
	out[2] = z;
}

static void StoreEulerFromBasis(float (&out)[3], const float (&right)[3], const float (&forward)[3], const float (&up)[3])
{
	auto pitch = std::asin(std::clamp(forward[2], -1.0f, 1.0f));
	float yaw, roll;

	if (std::abs(forward[2]) > 0.99999f)
	{
		yaw = std::atan2(right[1], right[0]);
		roll = 0.0f;
	}
	else
	{
		yaw = std::atan2(-forward[0], forward[1]);
		roll = std::atan2(-right[2], up[2]);
	}

	StoreVector(out, pitch * kRadToDeg, roll * kRadToDeg, yaw * kRadToDeg);
}

static void NuiGameState_RunFrame()
{
	if (!Instance<ICoreGameInit>::Get()->HasVariable("gameSettled"))
	{
		return;
	}

	if (!g_actorPos || !g_viewportGame || !*g_viewportGame)
	{
		return;
	}

	auto& state = *g_nuiGameState;

	// take the seqlock: an odd sequence tells readers this frame is mid-write
	auto sequence = state.sequence.load(std::memory_order_relaxed);
	state.sequence.store(sequence + 1, std::memory_order_relaxed);

	std::atomic_thread_fence(std::memory_order_release);

	auto& data = state.data;

	const auto& viewport = (*g_viewportGame)->viewport;
	const auto& camMatrix = viewport.m_inverseView;

	float right[3] = { camMatrix[0], camMatrix[1], camMatrix[2] };
	float up[3] = { camMatrix[4], camMatrix[5], camMatrix[6] };
	float forward[3] = { -camMatrix[8], -camMatrix[9], -camMatrix[10] };

	StoreVector(data.camPos, camMatrix[12], camMatrix[13], camMatrix[14]);
	StoreVector(data.camForward, forward[0], forward[1], forward[2]);
	StoreVector(data.camUp, up[0], up[1], up[2]);
	StoreEulerFromBasis(data.camRot, right, forward, up);

	data.camFov = (viewport.m_projection[5] > 0.0f)
		? 2.0f * std::atan(1.0f / viewport.m_projection[5]) * kRadToDeg
		: 0.0f;

	StoreVector(data.pedPos, g_actorPos[0], g_actorPos[1], g_actorPos[2]);

	// release the seqlock, back to an even (and different) sequence
	state.sequence.store(sequence + 2, std::memory_order_release);
}

static HookFunction hookFunction([]()
{
#ifdef GTA_FIVE
	g_actorPos = hook::get_address<float*>(hook::get_pattern("BB 00 00 40 00 48 89 7D F8 89 1D", -4)) + 12;
#elif IS_RDR3
	g_actorPos = hook::get_address<float*>(hook::get_pattern("45 33 C9 48 89 5D E0 8D 53 01", 63)) + 16;
#endif
});

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		NuiGameState_RunFrame();
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_nuiGameState->sequence.store(0, std::memory_order_release);
	});
});
