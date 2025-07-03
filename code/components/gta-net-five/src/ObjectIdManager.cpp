#include <StdInc.h>
#include <Hooking.h>

#include <NetBuffer.h>
#include <NetLibrary.h>

#include <GameInit.h>
#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <CloneManager.h>

#include <MinHook.h>

#include "ObjectIds.h"
#include "ObjectIdsPacketHandler.h"

static std::list<int> g_objectIds;
static std::set<int> g_usedObjectIds;
static std::set<int> g_stolenObjectIds;

static uint32_t(*g_origAssignObjectId)(void*);

static uint32_t AssignObjectId(void* objectIds)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origAssignObjectId(objectIds);
	}

	if (g_objectIds.empty())
	{
		trace("No free object ID!\n");
		return 0;
	}

	auto it = g_objectIds.begin();
	auto objectId = *it;

	g_objectIds.erase(it);
	g_stolenObjectIds.erase(objectId);
	g_usedObjectIds.insert(objectId);

	TheClones->Log("%s: id %d\n", __func__, objectId);

	return objectId;
}

static bool(*g_origReturnObjectId)(void*, uint16_t);

static bool ReturnObjectId(void* objectIds, uint16_t objectId)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origReturnObjectId(objectIds, objectId);
	}

	if (g_usedObjectIds.find(objectId) != g_usedObjectIds.end())
	{
		TheClones->Log("%s: id %d\n", __func__, objectId);

		g_usedObjectIds.erase(objectId);

		// only return it to ourselves if it was ours to begin with
		// (and only use this for network protocol version 0x201903031957 or above - otherwise server bookkeeping will go out of sync)
		if (g_stolenObjectIds.find(objectId) == g_stolenObjectIds.end())
		{
			if (!TheClones->IsRemovingObjectId(objectId))
			{
				g_objectIds.push_back(objectId);
			}
		}

		g_stolenObjectIds.erase(objectId);

		return true;
	}

	return false;
}

void ObjectIds_ReturnObjectId(uint16_t objectId)
{
	ReturnObjectId(nullptr, objectId);
}

static bool(*g_origHasSpaceForObjectId)(void*, int, bool);

static bool HasSpaceForObjectId(void* objectIds, int num, bool unkScript)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origHasSpaceForObjectId(objectIds, num, unkScript);
	}

	return (g_objectIds.size() >= num);
}

void ObjectIds_AddObjectId(int objectId)
{
	// track 'stolen' migration object IDs so we can return them to the server once they get deleted
	bool wasOurs = false;

	// this is ours now
	g_usedObjectIds.insert(objectId);

	// remove this object ID from our free list
	for (auto it = g_objectIds.begin(); it != g_objectIds.end(); it++)
	{
		if (*it == objectId)
		{
			wasOurs = true;

			g_objectIds.erase(it);
			break;
		}
	}

	if (!wasOurs)
	{
		g_stolenObjectIds.insert(objectId);
	}

	TheClones->Log("%s: id %d (wasOurs: %s)\n", __func__, objectId, wasOurs ? "true" : "false");
}

void ObjectIds_ConfirmObjectId(int objectId)
{
	TheClones->Log("%s: id %d\n", __func__, objectId);

	g_objectIds.push_back(objectId);
}

void ObjectIds_StealObjectId(int objectId)
{
	TheClones->Log("%s: id %d\n", __func__, objectId);

	if (g_usedObjectIds.find(objectId) != g_usedObjectIds.end())
	{
		g_stolenObjectIds.insert(objectId);
	}
	else
	{
		// remove this object ID from our free list (it was already returned, but server doesn't want it to be ours anymore)
		for (auto it = g_objectIds.begin(); it != g_objectIds.end(); it++)
		{
			if (*it == objectId)
			{
				g_objectIds.erase(it);
				break;
			}
		}
	}
}

void ObjectIds_RemoveObjectId(int objectId)
{
	// this is no longer ours
	g_usedObjectIds.erase(objectId);

	// we don't have to care if it got stolen anymore
	g_stolenObjectIds.erase(objectId);

	TheClones->Log("%s: id %d\n", __func__, objectId);
}

static NetLibrary* g_netLibrary;

static bool g_requestedIds;

void ObjectIds_BindNetLibrary(NetLibrary* netLibrary)
{
	g_netLibrary = netLibrary;

	netLibrary->AddPacketHandler<fx::ObjectIdsPacketHandler>(false);
}

static HookFunction hookFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		if (g_netLibrary->GetConnectionState() != NetLibrary::CS_ACTIVE)
		{
			return;
		}

		if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
		{
			return;
		}

		int reqCount = 16;

		if (Instance<ICoreGameInit>::Get()->HasVariable("onesync_big"))
		{
			reqCount = 4;
		}

		if (g_objectIds.size() < reqCount)
		{
			if (!g_requestedIds)
			{
				net::packet::ClientRequestObjectIdsPacket packet;
				g_netLibrary->SendNetPacket(packet);

				g_requestedIds = true;
			}
		}
	});

	OnKillNetworkDone.Connect([]()
	{
		g_objectIds.clear();
		g_usedObjectIds.clear();
		g_stolenObjectIds.clear();

		g_requestedIds = false;
	});

	MH_Initialize();


#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("FF 81 ? ? ? ? 8B 81 ? ? ? ? FF 89 ? ? ? ? 33 D2"), AssignObjectId, (void**)&g_origAssignObjectId);
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 48 89 7C 24 ? 81 B9"), ReturnObjectId, (void**)&g_origReturnObjectId);
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B B1 ? ? ? ? 8B DA"), HasSpaceForObjectId, (void**)&g_origHasSpaceForObjectId);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("0F B7 08 66 FF C9 66 3B CA 76", -0x1C), AssignObjectId, (void**)&g_origAssignObjectId);
	MH_CreateHook(hook::get_pattern("45 8B D9 85 DB 7E ? 8B B9", -0x1A), ReturnObjectId, (void**)&g_origReturnObjectId);
	MH_CreateHook(hook::get_pattern("48 83 EC 20 8B B9 ? ? ? ? 8B DA", -0x6), HasSpaceForObjectId, (void**)&g_origHasSpaceForObjectId);
#endif

	MH_EnableHook(MH_ALL_HOOKS);
});
