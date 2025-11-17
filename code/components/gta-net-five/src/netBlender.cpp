#include <StdInc.h>
#include <Hooking.h>
#include <netBlender.h>
#include <CoreConsole.h>
#include "nutsnbolts.h"


static hook::cdecl_stub<void(rage::netBlender*, uint32_t timeStamp)> netBlender_SetTimestamp([]()
{
	return hook::get_pattern("48 8B D9 39 79 18 74 76 48", -0x13);
});

namespace rage
{
void netBlender::SetTimestamp(uint32_t timestamp)
{
	return netBlender_SetTimestamp(this, timestamp);
}
}

static float* orientationDiffPtr = nullptr;

static constexpr float kDefaultMinOrientationDelta = 0.05f;   // ~2.86 degrees (original)
static constexpr float kRaisedMinOrientationDelta  = 0.08f;   // ~4.58 degrees (patched)

static void MinBlendDiffChanged(internal::ConsoleVariableEntry<bool>* var)
{
	if (!orientationDiffPtr)
	{
		return;
	}

	if (var->GetRawValue())
	{
		*orientationDiffPtr = kRaisedMinOrientationDelta;
	}
	else
	{
		*orientationDiffPtr = kDefaultMinOrientationDelta;
	}
}

static HookFunction hookFunction([]()
{
	// This patch increases the minimum orientation delta required to start an orientation blend
	// in CNetBlenderPhysical. The original value (≈0.05 rad ≈ 2.8°) was tuned for high client tick
	// rates in peer-to-peer networking, where entity snapshots are received very frequently
	// (often 100–200 Hz). In that environment the angle difference between updates is extremely
	// small, so even tiny deviations should trigger blending.
	//
	// In OneSync, however, the effective update tickrate is much lower (~20 Hz). This causes
	// orientation changes from remote vehicles to arrive with larger deltas between frames,
	// making the original low threshold too sensitive. As a result, vehicles frequently trigger
	// orientation corrections, leading to visible “snaps” or jitter when racing or following
	// other players.
	//
	// By raising the threshold to ~0.08 rad (~4.58°), small changes will be blended smoothly,
	// meaningful orientation deviations causes correction.
	{
		static ConVar<bool> enableHigherOrientationCorrectionDelta("game_forceHigherOrientationCorrectionDelta", ConVar_Replicated, true, &MinBlendDiffChanged);
		uint32_t orientationDiffOffset = *hook::get_pattern<uint32_t>("48 8D 44 24 ? 48 8B CB 48 89 44 24 ? E8 ? ? ? ? 48 8D 54 24", 48);
		void* g_vehicleBlenderData = *hook::get_address<void**>(hook::get_pattern("4C 8B 05 ? ? ? ? 48 8B D6 48 8B C8 E8 ? ? ? ? 48 8B D8 EB ? 33 DB", 0x3));
		orientationDiffPtr = reinterpret_cast<float*>(
			reinterpret_cast<uint8_t*>(g_vehicleBlenderData) + orientationDiffOffset
		);

		*orientationDiffPtr = kRaisedMinOrientationDelta;
	}
});
