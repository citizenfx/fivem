#include <StdInc.h>
#include <Hooking.h>

#include <NetBuffer.h>
#include <NetLibrary.h>

#include <GameInit.h>
#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <CloneManager.h>

#include <MinHook.h>

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
		if (Instance<ICoreGameInit>::Get()->NetProtoVersion < 0x201903031957 || g_stolenObjectIds.find(objectId) == g_stolenObjectIds.end())
		{
			g_objectIds.push_back(objectId);
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

	TheClones->Log("%s: id %d\n", __func__, objectId);
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

	netLibrary->AddReliableHandler("msgObjectIds", [](const char* data, size_t len)
	{
		net::Buffer buffer(reinterpret_cast<const uint8_t*>(data), len);

		auto numIds = buffer.Read<uint16_t>();

		int last = 0;

		for (int i = 0; i < numIds; i++)
		{
			auto skip = buffer.Read<uint16_t>();
			auto take = buffer.Read<uint16_t>();

			last += skip + 1;

			for (int j = 0; j <= take; j++)
			{
				int objectId = last++;

				TheClones->Log("got object id %d\n", objectId);

				g_objectIds.push_back(objectId);
				g_stolenObjectIds.erase(objectId);
			}
		}

		g_requestedIds = false;
	});
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
				net::Buffer outBuffer;
				outBuffer.Write<uint16_t>(32);

				g_netLibrary->SendReliableCommand("msgRequestObjectIds", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());

				g_requestedIds = true;
			}
		}
	});

	OnKillNetworkDone.Connect([]()
	{
		g_objectIds.clear();
		g_usedObjectIds.clear();

		g_requestedIds = false;
	});

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("FF 89 C4 3E 00 00 33 D2", -12), AssignObjectId, (void**)&g_origAssignObjectId);
	MH_CreateHook(hook::get_pattern("44 8B 91 C4 3E 00 00", -0x14), ReturnObjectId, (void**)&g_origReturnObjectId);
	MH_CreateHook(hook::get_pattern("48 83 EC 20 8B B1 C4 3E 00 00", -0xB), HasSpaceForObjectId, (void**)&g_origHasSpaceForObjectId);
	MH_EnableHook(MH_ALL_HOOKS);
});
