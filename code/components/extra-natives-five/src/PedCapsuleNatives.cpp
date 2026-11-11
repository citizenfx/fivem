#include "StdInc.h"

#include <ScriptEngine.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <EntitySystem.h>
#include <ICoreGameInit.h>
#include <Local.h>
#include <NetBitVersion.h>

#include "EntityExtensions.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <unordered_map>

//
// A ped's mover capsule (its collision with the world) is sized from its capsule info: it spans from CapsuleZOffset
// up to HeadHeight, relative to the ped's position. The game rebuilds it whenever the capsule's radius or vertical
// offset changes, passing the head height to use (HeadHeightCrouched when ducking, which on foot never happens).
//

class PedCapsuleHeightOverrideExtension : public rage::fwExtension
{
public:
	virtual int GetExtensionId() const override
	{
		return GetClassId();
	}

	static int GetClassId()
	{
		return (int)EntityExtensionClassId::PedCapsuleHeightOverride;
	}

	// 0.0 when there's no override
	float headHeight = 0.0f;
};

// CPed field caching the vertical offset the mover capsule was last built with
static uint32_t g_pedCapsuleZOffsetOffset;

static void (*g_origRebuildMoverCapsule)(fwEntity* ped, void* capsuleInfo, float headHeight, float zOffset, float radius, float radiusBlendRate);

static void RebuildMoverCapsule(fwEntity* ped, void* capsuleInfo, float headHeight, float zOffset, float radius, float radiusBlendRate)
{
	if (auto extension = ped->GetExtension<PedCapsuleHeightOverrideExtension>(); extension && extension->headHeight > 0.0f)
	{
		headHeight = extension->headHeight;
	}

	g_origRebuildMoverCapsule(ped, capsuleInfo, headHeight, zOffset, radius, radiusBlendRate);
}

static void SetCapsuleHeightOverride(fwEntity* ped, float headHeight)
{
	auto extension = ped->GetExtension<PedCapsuleHeightOverrideExtension>();

	if (!extension)
	{
		if (headHeight <= 0.0f)
		{
			return;
		}

		extension = new PedCapsuleHeightOverrideExtension();
		ped->AddExtension(extension);
	}

	if (extension->headHeight == headHeight)
	{
		return;
	}

	extension->headHeight = headHeight;

	// the game rebuilds the capsule on its next update when the cached offset doesn't match the current one
	reinterpret_cast<hook::FlexStruct*>(ped)->Set<float>(g_pedCapsuleZOffsetOffset, -FLT_MAX);
}

//
// With net version 6, the override is synced in CPedGameStateDataNode, so other players collide with a ped's lowered
// capsule too. The node's data is extracted from the ped, serialised (in read, write, size and log modes alike) and
// applied to the ped by separate functions, so the height is kept per node in between.
//

static std::unordered_map<void*, float> g_nodeCapsuleHeights;

// offset from the ped game state interface of a CNetObjPed to its ped
static int32_t g_pedGameStateInterfacePedOffset;

static fwEntity* GetPedFromGameStateInterface(void* netObjInterface)
{
	return reinterpret_cast<hook::FlexStruct*>(netObjInterface)->Get<fwEntity*>(g_pedGameStateInterfacePedOffset);
}

static void (*g_origExtractPedGameStateData)(void* node, void* netObjInterface);

static void ExtractPedGameStateData(void* node, void* netObjInterface)
{
	g_origExtractPedGameStateData(node, netObjInterface);

	float headHeight = 0.0f;

	if (fwEntity* ped = GetPedFromGameStateInterface(netObjInterface))
	{
		if (auto extension = ped->GetExtension<PedCapsuleHeightOverrideExtension>())
		{
			headHeight = extension->headHeight;
		}
	}

	g_nodeCapsuleHeights[node] = headHeight;
}

static void (*g_origSerialisePedGameStateData)(void* node, hook::FlexStruct* serialiser);

static void SerialisePedGameStateData(void* node, hook::FlexStruct* serialiser)
{
	g_origSerialisePedGameStateData(node, serialiser);

	// rage::CSyncDataBase virtuals
	bool isMaximumSize = serialiser->CallVirtual<bool>(0xC8);

	if (!isMaximumSize && !Instance<ICoreGameInit>::Get()->IsNetVersionOrHigher(net::NetBitVersion::netVersion6))
	{
		return;
	}

	float& headHeight = g_nodeCapsuleHeights[node];

	bool hasOverride = headHeight > 0.0f;
	serialiser->CallVirtual<void>(0x38, &hasOverride, (const char*)nullptr);

	// in centimeters, up to 10.23m
	uint32_t value = std::clamp<uint32_t>(uint32_t(std::lround(headHeight * 100.0f)), 1, 1023);

	if (hasOverride || isMaximumSize)
	{
		serialiser->CallVirtual<void>(0x68, &value, 10, (const char*)nullptr);
	}

	if (!isMaximumSize)
	{
		headHeight = hasOverride ? value / 100.0f : 0.0f;
	}
}

static void (*g_origApplyPedGameStateData)(void* netObjInterface, void* node);

static void ApplyPedGameStateData(void* netObjInterface, void* node)
{
	g_origApplyPedGameStateData(netObjInterface, node);

	if (!Instance<ICoreGameInit>::Get()->IsNetVersionOrHigher(net::NetBitVersion::netVersion6))
	{
		return;
	}

	if (fwEntity* ped = GetPedFromGameStateInterface(netObjInterface))
	{
		SetCapsuleHeightOverride(ped, g_nodeCapsuleHeights[node]);
	}
}

static fwEntity* GetPedFromHandle(int handle)
{
	fwEntity* ped = rage::fwScriptGuid::GetBaseFromGuid(handle);
	return (ped && ped->IsOfType<CPed>()) ? ped : nullptr;
}

static HookFunction hookFunction([]()
{
	g_pedCapsuleZOffsetOffset = *hook::get_pattern<uint32_t>("F3 44 0F 11 83 ? ? ? ? 41 83 C8 FF", 5);
	g_origRebuildMoverCapsule = hook::trampoline(hook::get_pattern("48 85 D2 0F 84 ? ? ? ? 48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 56 41 57 48 8D 68 B1"), RebuildMoverCapsule);

	// CPedGameStateDataNode sync
	auto apply = hook::get_pattern<char>("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 4C 89 60 20 55 41 56 41 57 48 8D 68 A1 48 81 EC D0 00 00 00 48 8B B9 ? ? ? ? 45 33 E4");
	g_pedGameStateInterfacePedOffset = *(int32_t*)(apply + 38);

	g_origExtractPedGameStateData = hook::trampoline(hook::get_pattern("40 53 48 83 EC 20 48 8B 02 4C 8B C2 48 8B D9 48 8B D1 49 8B C8 FF 50 60"), ExtractPedGameStateData);
	g_origSerialisePedGameStateData = hook::trampoline(hook::get_pattern("48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 30 48 8B 02 48 8B FA 48 8D 91 55 01 00 00"), SerialisePedGameStateData);
	g_origApplyPedGameStateData = hook::trampoline(apply, ApplyPedGameStateData);

	fx::ScriptEngine::RegisterNativeHandler("SET_PED_CAPSULE_HEIGHT_OVERRIDE", [](fx::ScriptContext& context)
	{
		fwEntity* ped = GetPedFromHandle(context.GetArgument<int>(0));
		float height = context.GetArgument<float>(1);

		if (ped && std::isfinite(height) && height > 0.0f)
		{
			SetCapsuleHeightOverride(ped, height);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("CLEAR_PED_CAPSULE_HEIGHT_OVERRIDE", [](fx::ScriptContext& context)
	{
		if (fwEntity* ped = GetPedFromHandle(context.GetArgument<int>(0)))
		{
			SetCapsuleHeightOverride(ped, 0.0f);
		}
	});
});
