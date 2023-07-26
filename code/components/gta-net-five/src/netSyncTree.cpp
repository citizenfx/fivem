#include <StdInc.h>
#include <Hooking.h>

#include <netSyncTree.h>
#include <CrossBuildRuntime.h>
#include <netObject.h>
#include <MinHook.h>

static hook::cdecl_stub<bool(uint32_t hash)> IS_THIS_MODEL_A_HELI_internal([]()
{
	return hook::get_pattern("83 B9 ? ? ? ? 06 0F B6", -0x2E);
});

static hook::cdecl_stub<rage::netSyncTree*(void*, int)> getSyncTreeForType([]()
{
	return hook::get_pattern("0F B7 CA 83 F9 07 7F 5E");
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, int flags, int flags2, rage::datBitBuffer* buffer, void* netLogStub)> netSyncTree_ReadFromBuffer([]()
{
	return xbr::IsGameBuildOrGreater<2802>() ? hook::get_pattern("44 89 40 18 57 48 83 EC 30 44 8B 05 ? ? ? ? 65", -15) : hook::get_pattern("45 89 43 18 57 48 83 EC 30 48 83 79 10 00 49", -15);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_CanApplyToObject([]()
{
	return xbr::IsGameBuildOrGreater<2802>() ? hook::get_pattern("49 8B 06 49 8B CE FF 90 A0 00 00 00 84 C0", -0x29) : hook::get_pattern("49 8B CE FF 50 70 84 C0 74 31 33 FF", -0x2C);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_PrepareObject([]()
{
	return xbr::IsGameBuildOrGreater<2545>() ? hook::get_pattern("48 85 D2 74 45 48 8B 02 48 8B CA FF 50", -0x21) : hook::get_pattern("48 85 D2 74 4E 48 8B 02 48 8B CA", -0x21);
});

namespace rage
{
	bool netSyncTree::CanApplyToObject(netObject * object)
	{
		return netSyncTree_CanApplyToObject(this, object);
	}

	bool netSyncTree::ReadFromBuffer(int flags, int flags2, rage::datBitBuffer* buffer, void* netLogStub)
	{
		return netSyncTree_ReadFromBuffer(this, flags, flags2, buffer, netLogStub);
	}

	netSyncTree* netSyncTree::GetForType(NetObjEntityType type)
	{
		return getSyncTreeForType(nullptr, (int)type);
	}
}

// CVehicleCreationDataNode::CanApply()
// Basically the above rage::netSyncTree::CanApplyToObject()
// will call each of its this->SyncTrees and call this virtual function that we are hooking
static bool (*g_origVehicleCreationDataNode__CanApply)(void*, rage::netObject*);
static bool CVehicleCreationDataNode__CanApply(void* thisptr, rage::netObject* netObj /*Hidden argument*/)
{
	// Crashfix for de-sync between helicopter netobj type and vehicle hash
	if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Heli)
	{
		uint32_t hash = *(uint32_t*)(uintptr_t(thisptr) + 200);
		if (!IS_THIS_MODEL_A_HELI_internal(hash))
		{
			// Force hash to a Buzzard2, or it will cause crashes down the line(ropemanager, damagestatus, etc.)
			// better than returning false (makes them invisible)
			*(uint32_t*)(uintptr_t(thisptr) + 200) = 0x2C75F0DD;
		}
	}
	return g_origVehicleCreationDataNode__CanApply(thisptr, netObj);
}

static HookFunction hookinit([]()
{
	MH_CreateHook(hook::get_pattern("48 89 5C 24 10 55 48 8B EC 48 83 EC 20 8B 45 10 8B 89 C8 00 00 00"), CVehicleCreationDataNode__CanApply, (void**)&g_origVehicleCreationDataNode__CanApply);
	MH_EnableHook(MH_ALL_HOOKS);
});
