#include <StdInc.h>
#include <Hooking.h>

#include <netInterface.h>
#include <netObjectMgr.h>
#include <netObject.h>

#include <CrossBuildRuntime.h>
#include <ICoreGameInit.h>
#include <NetLibrary.h>
#include <MinHook.h>

#include <Error.h>

static ICoreGameInit* icgi;

// Original GetMaxBits functions
static uint64_t(*g_origSyncedUnk5180_GetMaxBits)(void*);
static uint64_t(*g_origSyncedUnk1038_GetMaxBits)(void*);
static uint64_t(*g_origSyncedUnkA450_GetMaxBits)(void*);
static uint64_t(*g_origSyncedUnk03B8_GetMaxBits)(void*);
static uint64_t(*g_origSyncedTarget_GetMaxBits)(void*);
static uint64_t(*g_origSyncedAI_GetMaxBits)(void*);
static uint64_t(*g_origSyncedTargetAI_GetMaxBits)(void*);
static uint64_t(*g_origSyncedAbstractTargetAI_GetMaxBits)(void*);
static uint64_t(*g_origSyncedObjectId_GetMaxBits)(void*);
static uint64_t(*g_origSyncedUnk11F8_GetMaxBits)(void*);
static uint64_t(*g_origSyncedTask_GetMaxBits)(void*);
static uint64_t(*g_origSyncedEntity_GetMaxBits)(void*);
static uint64_t(*g_origSyncedConstPed_GetMaxBits)(void*);
static uint64_t(*g_origSyncedVarGroup_GetMaxBits)(void*);
static uint64_t(*g_origSyncedXTargetData_GetMaxBits)(void*);

static bool (*g_origSyncedObjectId_UsesIdMappings)(void*);
static bool (*g_origSyncedEntity_UsesIdMappings)(void*);
static bool (*g_origSyncedConstPed_UsesIdMappings)(void*);
static bool (*g_origSyncedVarGroup_UsesIdMappings)(void*);
static bool (*g_origSyncedAI_UsesIdMappings)(void*);
static bool (*g_origSyncedTargetAI_UsesIdMappings)(void*);
static bool (*g_origSyncedTarget_UsesIdMappings)(void*);

template<bool onesyncVal, bool defaultVal>
static bool ReturnUsesIdMappings(void*)
{
	return icgi->OneSyncEnabled ? onesyncVal : defaultVal;
}

template<int Extension = 3>
static uint64_t ExtendMaxBits(uint64_t(*orig)(void*), void* self)
{
	auto value = orig(self);

	if (!icgi->OneSyncEnabled) 
	{
		return value;
	}

	// x extra bits for 16 bit object ids.
	return value + (icgi->OneSyncBigIdEnabled ? Extension : 0);
}

static inline uint64_t OverrideMaxBits(uint64_t(*orig)(void*), void* self, bool padding = true)
{
	if (!icgi->OneSyncEnabled)
	{
		return orig(self);
	}

	// 1 bit for bool value.
	// 16 or 13 bits for the object id.
	return (icgi->OneSyncBigIdEnabled ? 16 : 13) + (padding ? 1 : 0);
}

static uint64_t SyncedObjectId_GetMaxBits(void* self)
{
	return OverrideMaxBits(g_origSyncedObjectId_GetMaxBits, self);
}

static uint64_t SyncedEntity_GetMaxBits(void* self)
{
	return OverrideMaxBits(g_origSyncedEntity_GetMaxBits, self);
}

static uint64_t CSyncUnk11F8_GetMaxBits(void* self)
{
	return OverrideMaxBits(g_origSyncedUnk11F8_GetMaxBits, self);
}

static uint64_t SyncedConstPed_GetMaxBits(void* self)
{
	return OverrideMaxBits(g_origSyncedConstPed_GetMaxBits, self);
}

static uint64_t CSyncUnk5180_GetMaxBits(void* self)
{
	return ExtendMaxBits(g_origSyncedUnk5180_GetMaxBits, self);
}

static uint64_t SyncedAI_GetMaxBits(void* self)
{
	auto val = g_origSyncedAI_GetMaxBits(self);
	trace("%s: %i %i\n", __func__, val, val + 12);
	return ExtendMaxBits<12>(g_origSyncedAI_GetMaxBits, self);
}

static uint64_t SyncedTarget_GetMaxBits(void* self)
{
	return ExtendMaxBits<12>(g_origSyncedTarget_GetMaxBits, self);
}	

static uint64_t SyncedTargetAI_GetMaxBits(void* self)
{
	return ExtendMaxBits(g_origSyncedTargetAI_GetMaxBits, self);
}

static uint64_t SyncedAbstractTargetAI_GetMaxBits(void* self)
{
	auto val = g_origSyncedAbstractTargetAI_GetMaxBits(self);
	return ExtendMaxBits(g_origSyncedAbstractTargetAI_GetMaxBits, self);
}

static uint64_t SyncedX_TargetData_GetMaxBits(void* self)
{
	return ExtendMaxBits(g_origSyncedXTargetData_GetMaxBits, self);
}

static uint64_t SyncedVarGroup_GetMaxBits(void* self)
{
	trace("GetMaxBits (vtable %p) %i, %i\n", (void*)hook::get_unadjusted(*(uint64_t**)self), g_origSyncedVarGroup_GetMaxBits(self), g_origSyncedVarGroup_GetMaxBits(self) + 16 + 1);
	//Expand all groups by 16 bits to give overhead to any syncvars that may be effected by length hack
	return ExtendMaxBits<16>(g_origSyncedVarGroup_GetMaxBits, self);
}

static uint64_t SyncedUnk1038_GetMaxBits(void* self)
{
	return ExtendMaxBits(g_origSyncedUnk1038_GetMaxBits, self);
}

static uint64_t SyncedUnkA450_GetMaxBits(void* self)
{
	return ExtendMaxBits(g_origSyncedUnkA450_GetMaxBits, self);
}

static uint64_t SyncedUnk03B8_GetMaxBits(void* self)
{
	return OverrideMaxBits(g_origSyncedUnk03B8_GetMaxBits, self, false);
}

static bool SyncedConstPed_UsesIdMappings(void* self)
{
	return (icgi->OneSyncEnabled) ? false : g_origSyncedConstPed_UsesIdMappings(self);
}

static bool SyncedVarGroup_UsesIdMappings(void* self)
{
	return (icgi->OneSyncEnabled) ? false : g_origSyncedVarGroup_UsesIdMappings(self);
}

static bool SyncedAI_UsesIdMappings(void* self)
{
	return (icgi->OneSyncEnabled) ? false : g_origSyncedAI_UsesIdMappings(self);
}

static bool SyncedTargetAI_UsesIdMappings(void* self)
{
	return (icgi->OneSyncEnabled) ? false : g_origSyncedTargetAI_UsesIdMappings(self);
}

static bool SyncedTarget_UsesIdMappings(void* self)
{
	return (icgi->OneSyncEnabled) ? false : g_origSyncedTarget_UsesIdMappings(self);
}

static bool SyncedObjectId_UsesIdMappings(void* self)
{
	return (icgi->OneSyncEnabled) ? false : g_origSyncedObjectId_UsesIdMappings(self);
}

static bool SyncedEntity_UsesIdMappings(void* self)
{
	return (icgi->OneSyncEnabled) ? false : g_origSyncedEntity_UsesIdMappings(self);
}

static HookFunction hookSyncedExtensions([]()
{
	// Patch CSyncVars* vtable methods for Size calculator and ID Mapping to support length hack in onesync.
	static constexpr int kSyncedVarGetMaxBitsIndex = 9;
	static constexpr int kSyncedVarUsesIdMappingsIndex = 19;

	// CSyncedEntity / CSyncedConstEntity
	{
		const auto syncedEntityVtable = hook::get_address<uintptr_t*>(hook::get_pattern("89 AB 84 06 00 00 48 8D 05 ? ? ? ? 48 89", 9));

		g_origSyncedEntity_GetMaxBits = (decltype(g_origSyncedEntity_GetMaxBits))syncedEntityVtable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedEntityVtable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedEntity_GetMaxBits);

		g_origSyncedEntity_UsesIdMappings = (decltype(g_origSyncedEntity_UsesIdMappings))syncedEntityVtable[kSyncedVarUsesIdMappingsIndex];
		hook::put(&syncedEntityVtable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)SyncedEntity_UsesIdMappings);
	}

	// CSyncedConstPed
	{
		auto syncedConstPedVtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 66 89 AB", 3));

		g_origSyncedConstPed_GetMaxBits = (decltype(g_origSyncedConstPed_GetMaxBits))syncedConstPedVtable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedConstPedVtable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedConstPed_GetMaxBits);

		g_origSyncedConstPed_UsesIdMappings = (decltype(g_origSyncedConstPed_UsesIdMappings))syncedConstPedVtable[kSyncedVarUsesIdMappingsIndex];
		hook::put(&syncedConstPedVtable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)SyncedConstPed_UsesIdMappings);
	}

	// CSyncedObjectId
	{
		auto syncedObjectIdVtable = hook::get_address<uintptr_t*>(hook::get_pattern("44 89 77 0C 48 89 07", -4));

		g_origSyncedObjectId_GetMaxBits = (decltype(g_origSyncedObjectId_GetMaxBits))syncedObjectIdVtable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedObjectIdVtable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedObjectId_GetMaxBits);

		g_origSyncedObjectId_UsesIdMappings = (decltype(g_origSyncedObjectId_UsesIdMappings))syncedObjectIdVtable[kSyncedVarUsesIdMappingsIndex];
		hook::put(&syncedObjectIdVtable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)SyncedObjectId_UsesIdMappings);
	}

	// CSyncedUnk11F8
	{
		auto syncedUnkVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 49 89 00 4D 89 50", 3));
		g_origSyncedUnk11F8_GetMaxBits = (decltype(g_origSyncedUnk11F8_GetMaxBits))syncedUnkVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedUnkVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)CSyncUnk11F8_GetMaxBits);
	}

	// CSyncedUnk5180
	{
		auto syncedUnkVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 03 E8 ? ? ? ? 8B 87", 3));
		g_origSyncedUnk5180_GetMaxBits = (decltype(g_origSyncedUnk5180_GetMaxBits))syncedUnkVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedUnkVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)CSyncUnk5180_GetMaxBits);
		hook::put(&syncedUnkVTable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)ReturnUsesIdMappings<false, true>);
	}

	// CSyncedAITarget
	{
		auto syncedTargetAIVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 69 ? 49 8B D0", 3));
		g_origSyncedAI_GetMaxBits = (decltype(g_origSyncedAI_GetMaxBits))syncedTargetAIVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedTargetAIVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedAI_GetMaxBits);
		g_origSyncedAI_UsesIdMappings = (decltype(g_origSyncedAI_UsesIdMappings))syncedTargetAIVTable[kSyncedVarUsesIdMappingsIndex];
		hook::put(&syncedTargetAIVTable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)SyncedAI_UsesIdMappings);
	}

	// CSyncedTarget<CAITarget>
	{
		auto syncedAITargetVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 45 33 C0 48 8B D6", 3));
		g_origSyncedTargetAI_GetMaxBits = (decltype(g_origSyncedTargetAI_GetMaxBits))syncedAITargetVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedAITargetVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedTargetAI_GetMaxBits);
		g_origSyncedTargetAI_UsesIdMappings = (decltype(g_origSyncedTargetAI_UsesIdMappings))syncedAITargetVTable[kSyncedVarUsesIdMappingsIndex];
		hook::put(&syncedAITargetVTable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)SyncedTargetAI_UsesIdMappings);
	}
	
	// CSyncedTarget<CAIAbstractTarget>
	{
		auto address = hook::get_pattern("48 8D 05 ? ? ? ? 48 8B D6 48 8D 4F ? 48 89 07 E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 48 8B 5C 24 ? 48 8B C7 48 8B 74 24 ? 48 83 C4 ? 5F C3 CC", 3);
		auto aiAbstractTargetVTable = hook::get_address<uintptr_t*>((void*)address);

		g_origSyncedAbstractTargetAI_GetMaxBits = (decltype(g_origSyncedAbstractTargetAI_GetMaxBits))aiAbstractTargetVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&aiAbstractTargetVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedAbstractTargetAI_GetMaxBits);
		hook::put(&aiAbstractTargetVTable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)ReturnUsesIdMappings<false, true>);
	}

	// CSyncedTarget2. Similar to CSyncedTargetAI
	{
		auto syncedTargetVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 49 8D 4D", 3));
		g_origSyncedTarget_GetMaxBits = (decltype(g_origSyncedTarget_GetMaxBits))syncedTargetVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedTargetVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedTarget_GetMaxBits);
		g_origSyncedTarget_UsesIdMappings = (decltype(g_origSyncedTarget_UsesIdMappings))syncedTargetVTable[kSyncedVarUsesIdMappingsIndex];
		hook::put(&syncedTargetVTable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)SyncedTarget_UsesIdMappings);
	}

	// CSyncedX<TargetData>
	{
		auto syncedTargetXVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 44 89 76 ? 48 8D 4E", 3));
		g_origSyncedXTargetData_GetMaxBits = (decltype(g_origSyncedXTargetData_GetMaxBits))syncedTargetXVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedTargetXVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedX_TargetData_GetMaxBits);
		hook::put(&syncedTargetXVTable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)ReturnUsesIdMappings<false, true>);
	}

	// CSyncedPed
	{
		auto syncedUnkVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 89 AF ? ? ? ? 48 8D 8F ? ? ? ? 48 89 87 ? ? ? ? E8 ? ? ? ? 0F 57 F6", 3));
		g_origSyncedUnk1038_GetMaxBits = (decltype(g_origSyncedUnk1038_GetMaxBits))syncedUnkVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedUnkVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedUnk1038_GetMaxBits);
		hook::put(&syncedUnkVTable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)ReturnUsesIdMappings<false, true>);
	}

	// CSyncedUnkA450
	{
		auto syncedUnkVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 0D ? ? ? ? 89 6E", 3));
		g_origSyncedUnkA450_GetMaxBits = (decltype(g_origSyncedUnkA450_GetMaxBits))syncedUnkVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedUnkVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedUnkA450_GetMaxBits);
		hook::put(&syncedUnkVTable[kSyncedVarUsesIdMappingsIndex], (uintptr_t)ReturnUsesIdMappings<false, true>);
	}

	// CSyncedUnk03B8, Length hack extends the bitset to 16 bits but doesn't adjust the max bits
	{
		auto syncedUnkVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 02 66 89 5A ? 66 85 DB 74 ? 48 8B CA E8 ? ? ? ? 65 48 8B 04 25", 3));
		g_origSyncedUnk03B8_GetMaxBits = (decltype(g_origSyncedUnk03B8_GetMaxBits))syncedUnkVTable[kSyncedVarGetMaxBitsIndex];
		hook::put(&syncedUnkVTable[kSyncedVarGetMaxBitsIndex], (uintptr_t)SyncedUnk03B8_GetMaxBits);
	}

	// Patch `CSyncedVarGroup` to not use id mappings and extend max bits to support 16 bit object id
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("FF 90 ? ? ? ? 48 8B C8 48 8B 10 FF 92 ? ? ? ? 84 C0 75", -0x24), SyncedVarGroup_UsesIdMappings, (void**)&g_origSyncedVarGroup_UsesIdMappings);
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 83 79 ? ? 48 8B D9 0F 85 ? ? ? ? 48 8B 01"), SyncedVarGroup_GetMaxBits, (void**)&g_origSyncedVarGroup_GetMaxBits);
	MH_EnableHook(MH_ALL_HOOKS);
});

static InitFunction initFunction([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		icgi = Instance<ICoreGameInit>::Get();
	});
});
