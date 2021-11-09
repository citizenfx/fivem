#include <StdInc.h>
#include <Hooking.h>

#include <netObject.h>
#include <netObjectMgr.h>

#include <MinHook.h>

#include <ICoreGameInit.h>
#include <NetLibrary.h>

#include <CloneManager.h>
#include <CrossBuildRuntime.h>

static ICoreGameInit* icgi;

extern void CD_AllocateSyncData(uint16_t objectId);
extern void CD_FreeSyncData(uint16_t objectId);

static void(*g_orig_netObjectMgrBase__RegisterNetworkObject)(rage::netObjectMgr*, rage::netObject*);

static void netObjectMgrBase__RegisterNetworkObject(rage::netObjectMgr* manager, rage::netObject* object)
{
	CD_AllocateSyncData(object->GetObjectId());

	if (!icgi->OneSyncEnabled)
	{
		return g_orig_netObjectMgrBase__RegisterNetworkObject(manager, object);
	}

	if (!CloneObjectMgr->RegisterNetworkObject(object))
	{
		return;
	}

	// create a blender, if not existent
	if (!object->GetBlender())
	{
		object->CreateNetBlender();
	}

	if (!object->syncData.isRemote)
	{
#ifdef GTA_FIVE
		if (object->CanSynchronise(true))
#elif IS_RDR3
		int reason;
		if (object->CanSynchronise(true, &reason))
#endif
		{
			object->StartSynchronising();
		}
	}

	object->OnRegistered();
}

static void(*g_orig_netObjectMgrBase__DestroyNetworkObject)(rage::netObjectMgr*, rage::netObject*);

void ObjectIds_ReturnObjectId(uint16_t objectId);

static void netObjectMgrBase__DestroyNetworkObject(rage::netObjectMgr* manager, rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		CD_FreeSyncData(object->GetObjectId());
		return g_orig_netObjectMgrBase__DestroyNetworkObject(manager, object);
	}

	if (!object->syncData.shouldNotBeDeleted)
	{
		CD_FreeSyncData(object->GetObjectId());
		CloneObjectMgr->DestroyNetworkObject(object);

		if (!object->syncData.isRemote && object->syncData.nextOwnerId == 0xFF)
		{
			ObjectIds_ReturnObjectId(object->GetObjectId());
		}

		delete object;
	}
}

static void(*g_orig_netObjectMgrBase__ChangeOwner)(rage::netObjectMgr*, rage::netObject*, CNetGamePlayer*, int);

static void netObjectMgrBase__ChangeOwner(rage::netObjectMgr* manager, rage::netObject* object, CNetGamePlayer* targetPlayer, int migrationType)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_orig_netObjectMgrBase__ChangeOwner(manager, object, targetPlayer, migrationType);
	}

	auto oldOwnerId = object->syncData.ownerId;

	object->ChangeOwner(targetPlayer, migrationType);
	object->PostMigrate(migrationType);

	CloneObjectMgr->ChangeOwner(object, oldOwnerId, targetPlayer, migrationType);
}

static rage::netObject* (*g_orig_netObjectMgrBase__GetNetworkObject)(rage::netObjectMgr* manager, uint16_t id, bool evenIfDeleting);

static rage::netObject* netObjectMgrBase__GetNetworkObject(rage::netObjectMgr* manager, uint16_t id, bool evenIfDeleting)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_orig_netObjectMgrBase__GetNetworkObject(manager, id, evenIfDeleting);
	}

	auto object = CloneObjectMgr->GetNetworkObject(id);

	if (object && object->syncData.wantsToDelete && !evenIfDeleting)
	{
		object = nullptr;
	}

	return object;
}

CNetGamePlayer* netObject__GetPlayerOwner(rage::netObject* object);

static rage::netObject* (*g_orig_netObjectMgrBase__GetNetworkObjectForPlayer)(rage::netObjectMgr* manager, uint16_t id, rage::netPlayer* player, bool evenIfDeleting);

static rage::netObject* netObjectMgrBase__GetNetworkObjectForPlayer(rage::netObjectMgr* manager, uint16_t id, rage::netPlayer* player, bool evenIfDeleting)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_orig_netObjectMgrBase__GetNetworkObjectForPlayer(manager, id, player, evenIfDeleting);
	}

	auto object = CloneObjectMgr->GetNetworkObject(id);

	if (object && object->syncData.wantsToDelete && !evenIfDeleting)
	{
		object = nullptr;
	}

	if (object && netObject__GetPlayerOwner(object) != player)
	{
		object = nullptr;
	}

	return object;
}

static HookFunction hookFunction([]()
{
	MH_Initialize();

#if GTA_FIVE
	MH_CreateHook(hook::get_pattern("48 8B F2 0F B7 52 0A 41 B0 01", -0x19), netObjectMgrBase__RegisterNetworkObject, (void**)&g_orig_netObjectMgrBase__RegisterNetworkObject); //
	MH_CreateHook(hook::get_pattern("8A 42 4C 45 33 FF 48 8B DA C0 E8 02", -0x21), netObjectMgrBase__DestroyNetworkObject, (void**)&g_orig_netObjectMgrBase__DestroyNetworkObject); //
	MH_CreateHook(hook::get_pattern("44 8A 62 4B 33 DB 41 8B E9", -0x20), netObjectMgrBase__ChangeOwner, (void**)&g_orig_netObjectMgrBase__ChangeOwner); //
	MH_CreateHook(hook::get_pattern("44 38 33 75 30 66 44", -0x40), netObjectMgrBase__GetNetworkObject, (void**)&g_orig_netObjectMgrBase__GetNetworkObject); //
	MH_CreateHook(hook::get_pattern("41 80 78 ? FF 74 2D 41 0F B6 40"), netObjectMgrBase__GetNetworkObjectForPlayer, (void**)&g_orig_netObjectMgrBase__GetNetworkObjectForPlayer);
#elif IS_RDR3
	if (xbr::IsGameBuildOrGreater<1436>())
	{
		MH_CreateHook(hook::get_pattern("48 8B F2 41 B0 01 0F B7 52", -0x1B), netObjectMgrBase__RegisterNetworkObject, (void**)&g_orig_netObjectMgrBase__RegisterNetworkObject); //
		MH_CreateHook(hook::get_pattern("4C 8D 2D ? ? ? ? 48 8B F2 48 8B E9 BB", -0x1F), netObjectMgrBase__DestroyNetworkObject, (void**)&g_orig_netObjectMgrBase__DestroyNetworkObject); //
		MH_CreateHook(hook::get_pattern("0F B6 43 ? 48 03 C0 48 8B 4C C7 08 EB", -0x64), netObjectMgrBase__GetNetworkObjectForPlayer, (void**)&g_orig_netObjectMgrBase__GetNetworkObjectForPlayer);
	}
	else
	{
		MH_CreateHook(hook::get_pattern("41 0F B7 55 00 41 B0 01 48 8B E9 E8", xbr::IsGameBuildOrGreater<1355>() ? -0x20 : -0x27), netObjectMgrBase__RegisterNetworkObject, (void**)&g_orig_netObjectMgrBase__RegisterNetworkObject); //
		MH_CreateHook(hook::get_pattern("45 33 FF C1 E8 03 48 8B F2 48 8B E9 A8 01", -0x24), netObjectMgrBase__DestroyNetworkObject, (void**)&g_orig_netObjectMgrBase__DestroyNetworkObject); //
		MH_CreateHook(hook::get_pattern("0F B6 43 ? 48 03 C0 48 8B 4C C7 08 EB", -0x3B), netObjectMgrBase__GetNetworkObjectForPlayer, (void**)&g_orig_netObjectMgrBase__GetNetworkObjectForPlayer);
	}

	MH_CreateHook(hook::get_pattern("41 83 F9 04 75 ? 8D 4B 20 E8 ? ? ? ? 48", -0x31), netObjectMgrBase__ChangeOwner, (void**)&g_orig_netObjectMgrBase__ChangeOwner); //
	MH_CreateHook(hook::get_pattern("45 8A F0 0F B7 F2 E8 ? ? ? ? 33 DB 38", -0x24), netObjectMgrBase__GetNetworkObject, (void**)&g_orig_netObjectMgrBase__GetNetworkObject); //
#endif

	MH_EnableHook(MH_ALL_HOOKS);
});

static InitFunction initFunctionEv([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		icgi = Instance<ICoreGameInit>::Get();
	});
});
