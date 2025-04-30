#include <StdInc.h>
#include <Hooking.h>

#include <netObject.h>
#include <netObjectMgr.h>

#include <MinHook.h>

#include <ICoreGameInit.h>
#include <NetLibrary.h>

#include <CloneManager.h>
#include <CrossBuildRuntime.h>

#include <netBlender.h>
#include <netSyncTree.h>
#include <EntitySystem.h>
#include <netPlayerManager.h>

static ICoreGameInit* icgi;

#ifdef IS_RDR3
// Used to properly support extended physical Indexes in RDR3
extern void ClearNetObject(rage::netObject* object);
extern void SetupNetObject(rage::netObject* object);
#endif

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
		uint32_t reason;
		if (object->CanSynchronise(true, &reason))
#endif
		{
			object->StartSynchronising();
		}
	}

#ifdef IS_RDR3
	SetupNetObject(object);
#endif

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

#ifdef IS_RDR3
		ClearNetObject(object);
#endif

		delete object;
	}
}

#ifdef GTA_FIVE
static int g_trainTrackNodeIndexOffset;

static hook::cdecl_stub<void(CVehicle*, int, int)> CTrain__SetTrainCoord([]()
{
	return hook::pattern("44 8B C2 48 83 C4 ? 5B").count(1).get(0).get<void>(8);
});

static hook::cdecl_stub<bool(CVehicle*)> CTrain__IsCarriageEngine([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 80 A3 ? ? ? ? ? 24 ? 02 C0 08 83 ? ? ? ? F6 83 ? ? ? ? ? 74 ? 8A 05"));
});
#endif

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
	// Handle scenarios where a train was previously owned by the server
#ifdef GTA_FIVE
	// Make sure that this is a train and that we are now the new owner of it
	if (object->objectType == (uint16_t)NetObjEntityType::Train && targetPlayer->physicalPlayerIndex() == rage::GetLocalPlayer()->physicalPlayerIndex())
	{
		if (CVehicle* train = (CVehicle*)object->GetGameObject())
		{
			// Ensure this is the engine and the client has no knowledge of the trains current track node.
			if (CTrain__IsCarriageEngine(train) && *(int*)((uintptr_t)train + g_trainTrackNodeIndexOffset) == 0)
			{
				//Find the trains track node based on its current location
				CTrain__SetTrainCoord(train, -1, -1);
				// Force blend to apply location
				object->GetBlender()->Update();
			}
		}
	}
#endif
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

#ifdef IS_RDR3
using ObjectCB = void(__fastcall*)(void*, void*);

static void (*g_orig_netObjectMgrBase__ForAllObjects)(rage::netObjectMgr*, ObjectCB, void*);
static void netObjectMgrBase__ForAllObjects(rage::netObjectMgr* manager, ObjectCB cb, void* unk)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_orig_netObjectMgrBase__ForAllObjects(manager, cb, unk);
	}

	auto playerList = g_playerListRemote;
	for (int i = 0; i < g_playerListCountRemote; i++)
	{
		auto player = playerList[i];
		auto& objects = TheClones->GetPlayerObjects(player->physicalPlayerIndex());
		for (auto& entry : objects)
		{
			cb(entry.second, unk);
		}
	}
}

using FilterObjectCB = bool(__fastcall*)(void*);
static uint32_t (*g_orig_netObjectMgrBase__GetObjectFilteredCount)(rage::netObjectMgr*, FilterObjectCB);
static uint32_t netObjectMgrBase__GetObjectFilteredCount(rage::netObjectMgr* manager, FilterObjectCB cb)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_orig_netObjectMgrBase__GetObjectFilteredCount(manager, cb);
	}

	auto& objects = TheClones->GetPlayerObjects(rage::GetLocalPlayer()->physicalPlayerIndex());
	int count = 0;
	for (auto& entry : objects)
	{
		bool result = cb(entry.second);

		if (result)
		{
			count++;
		}
	}

	return count;
}

static hook::cdecl_stub<bool(rage::netPlayer*)> _isNetPlayerLocal([]()
{
	return hook::get_call(hook::get_pattern("48 8B 81 ? ? ? ? 48 85 C0 74 ? BA"));
});

using LocalObjectCB = void(__fastcall*)(rage::netObject*, void*);
static bool(*g_orig_netObjectMgrBase__ForAllLocalObjects)(rage::netObjectMgr*, LocalObjectCB, void*);
static bool netObjectMgrBase__ForAllLocalObjects(rage::netObjectMgr* manager, LocalObjectCB cb, void* a3)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_orig_netObjectMgrBase__ForAllLocalObjects(manager, cb, a3);
	}

	auto& objects = TheClones->GetPlayerObjects(rage::GetLocalPlayer()->physicalPlayerIndex());

	for (auto& entry : objects)
	{
		cb(entry.second, a3);
	}
}
#endif

static HookFunction hookFunction([]()
{
	MH_Initialize();

#if GTA_FIVE
	//Taken from extra-natives-five/VehicleExtraNatives.cpp
	g_trainTrackNodeIndexOffset = *hook::get_pattern<uint32_t>("E8 ? ? ? ? 40 8A F8 84 C0 75 ? 48 8B CB E8", -4);
	
	MH_CreateHook(hook::get_pattern("48 8B F2 0F B7 52 0A 41 B0 01", -0x19), netObjectMgrBase__RegisterNetworkObject, (void**)&g_orig_netObjectMgrBase__RegisterNetworkObject); //
	MH_CreateHook(hook::get_pattern("8A 42 4C 45 33 FF 48 8B DA C0 E8 02", -0x21), netObjectMgrBase__DestroyNetworkObject, (void**)&g_orig_netObjectMgrBase__DestroyNetworkObject); //
	if (xbr::IsGameBuildOrGreater<3258>())
	{
		MH_CreateHook(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 ? ? 48 89 ? ? ? 89 ? ? 41 ? 41 56 41 57 48 81 EC ? ? ? ? 44 8A ? 4B"), netObjectMgrBase__ChangeOwner, (void**)&g_orig_netObjectMgrBase__ChangeOwner); //
	}
	else
	{
		MH_CreateHook(hook::get_pattern("44 8A 62 4B 33 DB 41 8B E9", -0x20), netObjectMgrBase__ChangeOwner, (void**)&g_orig_netObjectMgrBase__ChangeOwner); //
	}
	MH_CreateHook(hook::get_pattern("44 38 33 75 30 66 44", -0x40), netObjectMgrBase__GetNetworkObject, (void**)&g_orig_netObjectMgrBase__GetNetworkObject); //
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B F0 48 85 C0 0F 84 ? ? ? ? 48 8B C8 E8 ? ? ? ? BB")), netObjectMgrBase__GetNetworkObjectForPlayer, (void**)&g_orig_netObjectMgrBase__GetNetworkObjectForPlayer);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("48 8B F2 41 B0 01 0F B7 52", -0x1B), netObjectMgrBase__RegisterNetworkObject, (void**)&g_orig_netObjectMgrBase__RegisterNetworkObject);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 76 08 48 83 EB 01 75 E8")), netObjectMgrBase__DestroyNetworkObject, (void**)&g_orig_netObjectMgrBase__DestroyNetworkObject);
	MH_CreateHook(hook::get_pattern("0F B6 43 ? 48 03 C0 48 8B 4C C7 08 EB", -0x64), netObjectMgrBase__GetNetworkObjectForPlayer, (void**)&g_orig_netObjectMgrBase__GetNetworkObjectForPlayer);

	MH_CreateHook(hook::get_pattern("41 83 F9 04 75 ? 8D 4B 20 E8 ? ? ? ? 48", -0x39), netObjectMgrBase__ChangeOwner, (void**)&g_orig_netObjectMgrBase__ChangeOwner);
	MH_CreateHook(hook::get_pattern("45 8A F0 0F B7 F2 E8 ? ? ? ? 33 DB 38", -0x24), netObjectMgrBase__GetNetworkObject, (void**)&g_orig_netObjectMgrBase__GetNetworkObject);

	// Remove dependency on NetworkObjectMgrBase::m_playerObjects
	MH_CreateHook(hook::get_pattern("49 8B CE 4C 8B FA 33 F6", -37), netObjectMgrBase__GetObjectFilteredCount, (void**)g_orig_netObjectMgrBase__GetObjectFilteredCount);
	MH_CreateHook(hook::get_pattern("0F 84 ? ? ? ? 48 8B 80 ? ? ? ? 80 78 ? ? 72 ? BA", -50), netObjectMgrBase__ForAllLocalObjects, (void**)&g_orig_netObjectMgrBase__ForAllLocalObjects);
	// Replace ForAllObjects to support extended local player physical index
	MH_CreateHook(hook::get_pattern("E8 ? ? ? ? 44 8A 0D ? ? ? ? 48 8B 3D", -0x44), netObjectMgrBase__ForAllObjects, (void**)&g_orig_netObjectMgrBase__ForAllObjects);
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
 
