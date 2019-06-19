#include <StdInc.h>
#include <Hooking.h>

#include <netObject.h>
#include <netObjectMgr.h>

#include <MinHook.h>

#include <ICoreGameInit.h>
#include <NetLibrary.h>

#include <CloneManager.h>

static ICoreGameInit* icgi;

static void(*g_orig_netObjectMgrBase__RegisterNetworkObject)(rage::netObjectMgr*, rage::netObject*);

static void netObjectMgrBase__RegisterNetworkObject(rage::netObjectMgr* manager, rage::netObject* object)
{
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
		if (object->CanSynchronise(true))
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
		return g_orig_netObjectMgrBase__DestroyNetworkObject(manager, object);
	}

	if (!object->syncData.shouldNotBeDeleted)
	{
		CloneObjectMgr->DestroyNetworkObject(object);

		if (!object->syncData.isRemote && object->syncData.nextOwnerId == 0xFF)
		{
			ObjectIds_ReturnObjectId(object->objectId);
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

	CloneObjectMgr->ChangeOwner(object, targetPlayer, migrationType);

	object->ChangeOwner(targetPlayer, migrationType);
	object->PostMigrate(migrationType);
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
	// 1604
	MH_Initialize();
	MH_CreateHook((void*)0x141615FC0, netObjectMgrBase__RegisterNetworkObject, (void**)&g_orig_netObjectMgrBase__RegisterNetworkObject);
	MH_CreateHook((void*)0x141605054, netObjectMgrBase__DestroyNetworkObject, (void**)&g_orig_netObjectMgrBase__DestroyNetworkObject);
	MH_CreateHook((void*)0x1416033D0, netObjectMgrBase__ChangeOwner, (void**)&g_orig_netObjectMgrBase__ChangeOwner);
	MH_CreateHook((void*)0x141608454, netObjectMgrBase__GetNetworkObject, (void**)&g_orig_netObjectMgrBase__GetNetworkObject);
	MH_CreateHook((void*)0x141608524, netObjectMgrBase__GetNetworkObjectForPlayer, (void**)& g_orig_netObjectMgrBase__GetNetworkObjectForPlayer);
	MH_EnableHook(MH_ALL_HOOKS);
});

static InitFunction initFunctionEv([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		icgi = Instance<ICoreGameInit>::Get();
	});
});
