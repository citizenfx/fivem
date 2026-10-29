#include "StdInc.h"

#include <ScriptEngine.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <GameInit.h>
#include <EntitySystem.h>
#include <Local.h>

#include <cfloat>
#include <cmath>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

//
// A ped's mover capsule (its collision with the world) is sized from its capsule info: it spans from CapsuleZOffset
// up to HeadHeight, relative to the ped's position. The game rebuilds it whenever the capsule's radius or vertical
// offset changes, passing the head height to use (HeadHeightCrouched when ducking, which on foot never happens).
//

struct CapsuleHeightOverride
{
	int guid;
	float headHeight;
};

static std::mutex g_capsuleOverridesMutex;
static std::unordered_map<fwEntity*, CapsuleHeightOverride> g_capsuleOverrides;

// CPed field caching the vertical offset the mover capsule was last built with
static uint32_t g_pedCapsuleZOffsetOffset;

static void (*g_origRebuildMoverCapsule)(fwEntity* ped, void* capsuleInfo, float headHeight, float zOffset, float radius, float radiusBlendRate);

static void RebuildMoverCapsule(fwEntity* ped, void* capsuleInfo, float headHeight, float zOffset, float radius, float radiusBlendRate)
{
	{
		std::lock_guard _(g_capsuleOverridesMutex);

		if (auto it = g_capsuleOverrides.find(ped); it != g_capsuleOverrides.end())
		{
			// the ped this override was set for may have been deleted, and its memory reused by another ped
			if (rage::fwScriptGuid::GetBaseFromGuid(it->second.guid) == ped)
			{
				headHeight = it->second.headHeight;
			}
			else
			{
				g_capsuleOverrides.erase(it);
			}
		}
	}

	g_origRebuildMoverCapsule(ped, capsuleInfo, headHeight, zOffset, radius, radiusBlendRate);
}

static void RequestMoverCapsuleRebuild(fwEntity* ped)
{
	// the game rebuilds the capsule on its next update when the cached offset doesn't match the current one
	reinterpret_cast<hook::FlexStruct*>(ped)->Set<float>(g_pedCapsuleZOffsetOffset, -FLT_MAX);
}

static HookFunction hookFunction([]()
{
	auto rebuildMoverCapsule = hook::pattern("48 85 D2 0F 84 ? ? ? ? 48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 56 41 57 48 8D 68 B1");
	auto zOffsetStore = hook::pattern("F3 44 0F 11 83 ? ? ? ? 41 83 C8 FF");

	if (rebuildMoverCapsule.size() == 1 && zOffsetStore.size() == 1)
	{
		g_pedCapsuleZOffsetOffset = *zOffsetStore.get_first<uint32_t>(5);
		g_origRebuildMoverCapsule = hook::trampoline(rebuildMoverCapsule.get_first(), RebuildMoverCapsule);
	}

	OnKillNetworkDone.Connect([]()
	{
		std::lock_guard _(g_capsuleOverridesMutex);
		g_capsuleOverrides.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PED_CAPSULE_HEIGHT_OVERRIDE", [](fx::ScriptContext& context)
	{
		if (!g_origRebuildMoverCapsule)
		{
			throw std::runtime_error("SET_PED_CAPSULE_HEIGHT_OVERRIDE is not supported on this game build.");
		}

		int handle = context.GetArgument<int>(0);
		float height = context.GetArgument<float>(1);

		fwEntity* ped = rage::fwScriptGuid::GetBaseFromGuid(handle);

		if (!ped || !ped->IsOfType<CPed>())
		{
			return;
		}

		{
			std::lock_guard _(g_capsuleOverridesMutex);

			if (std::isfinite(height) && height > 0.0f)
			{
				g_capsuleOverrides[ped] = { handle, height };
			}
			else
			{
				g_capsuleOverrides.erase(ped);
			}
		}

		RequestMoverCapsuleRebuild(ped);
	});
});
