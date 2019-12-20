#include <StdInc.h>
#include <CoreConsole.h>
#include <Hooking.h>
#include <ScriptEngine.h>

#include <netBlender.h>
#include <netInterface.h>
#include <netObject.h>
#include <netObjectMgr.h>
#include <netSyncTree.h>
#include <rlNetBuffer.h>

#include <CloneManager.h>

#include <ICoreGameInit.h>

#include <base64.h>

#include <NetLibrary.h>
#include <NetBuffer.h>

#include <Pool.h>

#include <EntitySystem.h>

extern NetLibrary* g_netLibrary;

class CNetGamePlayer;

static ICoreGameInit* icgi;

namespace rage
{
class netObject;

class netPlayerMgrBase
{
public:
	char pad[232 - 8];
	CNetGamePlayer* localPlayer;

public:
	virtual ~netPlayerMgrBase() = 0;

	virtual void Initialize() = 0;

	virtual void Shutdown() = 0;

	virtual void m_18() = 0;

	virtual CNetGamePlayer* AddPlayer(void* scInAddr, void* unkNetValue, void* addedIn1290, void* playerData, void* nonPhysicalPlayerData) = 0;

	virtual void RemovePlayer(CNetGamePlayer* player) = 0;
};
}

// 1103
// 1290
// 1365
// 1493
// 1604
static rage::netPlayerMgrBase* g_playerMgr;

void* g_tempRemotePlayer;

CNetGamePlayer* g_players[256];
std::unordered_map<uint16_t, CNetGamePlayer*> g_playersByNetId;
std::unordered_map<CNetGamePlayer*, uint16_t> g_netIdsByPlayer;

static CNetGamePlayer* g_playerList[256];
static int g_playerListCount;

static CNetGamePlayer* g_playerListRemote[256];
static int g_playerListCountRemote;

static CNetGamePlayer*(*g_origGetPlayerByIndex)(uint8_t);

static CNetGamePlayer* __fastcall GetPlayerByIndex(uint8_t index)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPlayerByIndex(index);
	}

	// temp hack
	/*if (index == 0)
	{
		return g_playerMgr->localPlayer;
	}*/

	if (index < 0 || index >= 256)
	{
		return nullptr;
	}

	return g_players[index];
}

static void(*g_origJoinBubble)(void* bubbleMgr, CNetGamePlayer* player);

static void JoinPhysicalPlayerOnHost(void* bubbleMgr, CNetGamePlayer* player)
{
	g_origJoinBubble(bubbleMgr, player);

	if (!icgi->OneSyncEnabled)
	{
		return;
	}

	if (player != g_playerMgr->localPlayer)
	{
		return;
	}

	trace("Assigning physical player index for the local player.\n");

	auto clientId = g_netLibrary->GetServerNetID();
	auto idx = g_netLibrary->GetServerSlotID();

	player->physicalPlayerIndex = idx;

	g_players[idx] = player;

	g_playersByNetId[clientId] = player;
	g_netIdsByPlayer[player] = clientId;

	// add to sequential list
	g_playerList[g_playerListCount] = player;
	g_playerListCount++;

	// don't add to g_playerListRemote(!)
}

CNetGamePlayer* GetPlayerByNetId(uint16_t netId)
{
	return g_playersByNetId[netId];
}

CNetGamePlayer* GetLocalPlayer()
{
	return g_playerMgr->localPlayer;
}

static CNetGamePlayer*(*g_origGetPlayerByIndexNet)(int);

static CNetGamePlayer* GetPlayerByIndexNet(int index)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPlayerByIndexNet(index);
	}

	// todo: check network game flag
	return GetPlayerByIndex(index);
}

static bool(*g_origIsNetworkPlayerActive)(int);

static bool IsNetworkPlayerActive(int index)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origIsNetworkPlayerActive(index);
	}

	return (GetPlayerByIndex(index) != nullptr);
}

static bool(*g_origIsNetworkPlayerConnected)(int);

static bool IsNetworkPlayerConnected(int index)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origIsNetworkPlayerConnected(index);
	}

	return (GetPlayerByIndex(index) != nullptr);
}

//int g_physIdx = 1;
int g_physIdx = 42;

namespace sync
{
	void TempHackMakePhysicalPlayer(uint16_t clientId, int idx = -1)
	{
		auto npPool = rage::GetPoolBase("CNonPhysicalPlayerData");

		// probably shutting down the network subsystem
		if (!npPool)
		{
			return;
		}

		void* fakeInAddr = calloc(256, 1);
		void* fakeFakeData = calloc(256, 1);

		rlGamerInfo* inAddr = (rlGamerInfo*)fakeInAddr;
		inAddr->peerAddress.localAddr.ip.addr = clientId ^ 0xFEED;
		inAddr->peerAddress.relayAddr.ip.addr = clientId ^ 0xFEED;
		inAddr->peerAddress.publicAddr.ip.addr = clientId ^ 0xFEED;
		inAddr->peerAddress.rockstarAccountId = clientId;
		inAddr->gamerId = clientId;

		// 1290
		// 1365
		// 1493
		// 1604
		//void* nonphys = calloc(256, 1);

		// this has to come from the pool directly as the game will expect to free it
		void* nonPhys = rage::PoolAllocate(npPool);
		((void(*)(void*))hook::get_adjusted(0x1410A4024))(nonPhys); // ctor

		// 1493
		// 1604
		void* phys = calloc(1024, 1);
		((void(*)(void*))hook::get_adjusted(0x1410A2480))(phys);

		auto player = g_playerMgr->AddPlayer(fakeInAddr, fakeFakeData, nullptr, phys, nonPhys);
		g_tempRemotePlayer = player;

		if (idx == -1)
		{
			idx = g_physIdx;
			g_physIdx++;
		}

		player->physicalPlayerIndex = idx;
		g_players[idx] = player;

		g_playersByNetId[clientId] = player;
		g_netIdsByPlayer[player] = clientId;

		g_playerList[g_playerListCount] = player;
		g_playerListCount++;

		g_playerListRemote[g_playerListCountRemote] = player;
		g_playerListCountRemote++;

		// resort player lists
		std::sort(g_playerList, g_playerList + g_playerListCount, [](CNetGamePlayer * left, CNetGamePlayer * right)
		{
			return (left->physicalPlayerIndex < right->physicalPlayerIndex);
		});

		std::sort(g_playerListRemote, g_playerListRemote + g_playerListCountRemote, [](CNetGamePlayer* left, CNetGamePlayer* right)
		{
			return (left->physicalPlayerIndex < right->physicalPlayerIndex);
		});
	}
}

void HandleClientInfo(const NetLibraryClientInfo& info)
{
	if (info.netId != g_netLibrary->GetServerNetID())
	{
		trace("Creating physical player %d (%s)\n", info.slotId, info.name);

		sync::TempHackMakePhysicalPlayer(info.netId, info.slotId);
	}
}

static hook::cdecl_stub<void* (CNetGamePlayer*)> getPlayerPedForNetPlayer([]()
{
	return hook::get_call(hook::get_pattern("84 C0 74 1C 48 8B CF E8 ? ? ? ? 48 8B D8", 7));
});

rage::netObject* GetLocalPlayerPedNetObject()
{
	auto ped = getPlayerPedForNetPlayer(g_playerMgr->localPlayer);

	if (ped)
	{
		auto netObj = *(rage::netObject * *)((char*)ped + 208);

		return netObj;
	}

	return nullptr;
}

void HandleClientDrop(const NetLibraryClientInfo& info)
{
	if (info.netId != g_netLibrary->GetServerNetID() && info.slotId != g_netLibrary->GetServerSlotID())
	{
		trace("Processing removal for player %d (%s)\n", info.slotId, info.name);

		if (info.slotId < 0 || info.slotId >= _countof(g_players))
		{
			trace("That's not a valid slot! Aborting.\n");
			return;
		}

		auto player = g_players[info.slotId];

		if (!player)
		{
			trace("That slot has no player. Aborting.\n");
			return;
		}

		// reassign the player's ped
		// TODO: only do this on a single client(!)

		// 1103
		// 1290
		// 1365
		// 1493
		// 1604
		uint16_t objectId = 0;
		//auto ped = ((void*(*)(void*, uint16_t*, CNetGamePlayer*))hook::get_adjusted(0x141022B20))(nullptr, &objectId, player);
		auto ped = getPlayerPedForNetPlayer(player);

		if (ped)
		{
			auto netObj = *(rage::netObject**)((char*)ped + 208);

			if (netObj)
			{
				objectId = netObj->objectId;
			}
		}

		trace("reassigning ped: %016llx %d\n", (uintptr_t)ped, objectId);

		if (ped)
		{
			TheClones->DeleteObjectId(objectId, true);

			trace("deleted object id\n");

			// 1604
			//((void(*)(void*, uint16_t, CNetGamePlayer*))hook::get_adjusted(0x141008D14))(ped, objectId, player);

			trace("success! reassigned the ped!\n");
		}

		// TEMP: properly handle order so that we don't have to fake out the game
		g_playersByNetId[info.netId] = nullptr;
		g_netIdsByPlayer[player] = -1;

		// TODO: actually leave the player including playerinfo
		//g_playerMgr->RemovePlayer(player);

		for (int i = 0; i < g_playerListCount; i++)
		{
			if (g_playerList[i] == player)
			{
				memmove(&g_playerList[i], &g_playerList[i + 1], sizeof(*g_playerList) * (g_playerListCount - i - 1));
				g_playerListCount--;

				break;
			}
		}

		for (int i = 0; i < g_playerListCountRemote; i++)
		{
			if (g_playerListRemote[i] == player)
			{
				memmove(&g_playerListRemote[i], &g_playerListRemote[i + 1], sizeof(*g_playerListRemote) * (g_playerListCountRemote - i - 1));
				g_playerListCountRemote--;

				break;
			}
		}

		// resort player lists
		std::sort(g_playerList, g_playerList + g_playerListCount, [](CNetGamePlayer * left, CNetGamePlayer * right)
		{
			return (left->physicalPlayerIndex < right->physicalPlayerIndex);
		});

		std::sort(g_playerListRemote, g_playerListRemote + g_playerListCountRemote, [](CNetGamePlayer* left, CNetGamePlayer* right)
		{
			return (left->physicalPlayerIndex < right->physicalPlayerIndex);
		});

		player->Reset();

		g_players[info.slotId] = nullptr;
	}
}

static CNetGamePlayer*(*g_origGetOwnerNetPlayer)(rage::netObject*);

CNetGamePlayer* netObject__GetPlayerOwner(rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetOwnerNetPlayer(object);
	}

	if (object->syncData.ownerId == 31)
	{
		auto player = g_playersByNetId[TheClones->GetClientId(object)];

		// FIXME: figure out why bad playerinfos occur
		if (player != nullptr && player->playerInfo != nullptr)
		{
			return player;
		}
	}

	return g_playerMgr->localPlayer;
}

static uint8_t(*g_origGetOwnerPlayerId)(rage::netObject*);

static uint8_t netObject__GetPlayerOwnerId(rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetOwnerPlayerId(object);
	}

	return netObject__GetPlayerOwner(object)->physicalPlayerIndex;
}

static CNetGamePlayer*(*g_origGetPendingPlayerOwner)(rage::netObject*);

static CNetGamePlayer* netObject__GetPendingPlayerOwner(rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPendingPlayerOwner(object);
	}

	if (object->syncData.nextOwnerId != 0xFF)
	{
		auto player = g_playersByNetId[TheClones->GetPendingClientId(object)];

		if (player != nullptr && player->playerInfo != nullptr)
		{
			return player;
		}
	}

	return nullptr;
}

static hook::cdecl_stub<void*(int handle)> getScriptEntity([]()
{
	return hook::pattern("44 8B C1 49 8B 41 08 41 C1 F8 08 41 38 0C 00").count(1).get(0).get<void>(-12);
});

static hook::cdecl_stub<uint32_t(void*)> getScriptGuidForEntity([]()
{
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
});

struct ReturnedCallStub : public jitasm::Frontend
{
	ReturnedCallStub(int idx, uintptr_t targetFunc)
		: m_index(idx), m_targetFunc(targetFunc)
	{

	}

	static void InstrumentedTarget(void* targetFn, void* callFn, int index)
	{
		trace("called %016llx (m_%x) from %016llx\n", (uintptr_t)targetFn, index, (uintptr_t)callFn);
	}

	virtual void InternalMain() override
	{
		push(rcx);
		push(rdx);
		push(r8);
		push(r9);

		mov(rcx, m_targetFunc);
		mov(rdx, qword_ptr[rsp + 0x20]);
		mov(r8d, m_index);

		// scratch space (+ alignment for stack)
		sub(rsp, 0x28);

		mov(rax, (uintptr_t)InstrumentedTarget);
		call(rax);

		add(rsp, 0x28);

		pop(r9);
		pop(r8);
		pop(rdx);
		pop(rcx);

		mov(rax, m_targetFunc);
		jmp(rax);
	}

private:
	uintptr_t m_originFunc;
	uintptr_t m_targetFunc;

	int m_index;
};

static void NetLogStub_DoTrace(void*, const char* fmt, ...)
{
	char buffer[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	trace("[NET] %s\n", buffer);
}

static void NetLogStub_DoLog(void*, const char* type, const char* fmt, ...)
{
	char buffer[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	trace("[NET] %s %s\n", type, buffer);
}

static CNetGamePlayer*(*g_origAllocateNetPlayer)(void*);

static CNetGamePlayer* AllocateNetPlayer(void* mgr)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origAllocateNetPlayer(mgr);
	}

	void* plr = malloc(672);

	// 1103
	// 1290
	// 1365
	// 1493
	// 1604
	return ((CNetGamePlayer*(*)(void*))hook::get_adjusted(0x1410A2300))(plr);
}

#include <minhook.h>

static void(*g_origPassObjectControl)(CNetGamePlayer* player, rage::netObject* netObject, int a3);

void ObjectIds_RemoveObjectId(int objectId);

static void PassObjectControlStub(CNetGamePlayer* player, rage::netObject* netObject, int a3)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origPassObjectControl(player, netObject, a3);
	}

	if (player->physicalPlayerIndex == netObject__GetPlayerOwner(netObject)->physicalPlayerIndex)
	{
		return;
	}

	trace("passing object %016llx (%d) control from %d to %d\n", (uintptr_t)netObject, netObject->objectId, netObject->syncData.ownerId, player->physicalPlayerIndex);
	TheClones->Log("%s: passing object %016llx (%d) control from %d to %d\n", __func__, (uintptr_t)netObject, netObject->objectId, netObject->syncData.ownerId, player->physicalPlayerIndex);

	ObjectIds_RemoveObjectId(netObject->objectId);

	netObject->syncData.nextOwnerId = 31;
	TheClones->SetTargetOwner(netObject, g_netIdsByPlayer[player]);

	fwEntity* entity = (fwEntity*)netObject->GetGameObject();
	if (entity && entity->IsOfType(HashString("CVehicle")))
	{
		CVehicle* vehicle = static_cast<CVehicle*>(entity);
		VehicleSeatManager* seatManager = vehicle->GetSeatManager();

		for (int i = 0; i < seatManager->GetNumSeats(); i++)
		{
			auto occupant = seatManager->GetOccupant(i);

			if (occupant)
			{
				auto netOccupant = reinterpret_cast<rage::netObject*>(occupant->GetNetObject());

				if (netOccupant)
				{
					if (!netOccupant->syncData.isRemote && netOccupant->objectType != 11)
					{
						trace("passing occupant %d control as well\n", netOccupant->objectId);

						PassObjectControlStub(player, netOccupant, a3);
					}
				}
			}
		}
	}

	//auto lastIndex = player->physicalPlayerIndex;
	//player->physicalPlayerIndex = 31;

	g_origPassObjectControl(player, netObject, a3);

	//player->physicalPlayerIndex = lastIndex;
}

static void(*g_origSetOwner)(rage::netObject* object, CNetGamePlayer* newOwner);

static void SetOwnerStub(rage::netObject* netObject, CNetGamePlayer* newOwner)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSetOwner(netObject, newOwner);
	}

	if (newOwner->physicalPlayerIndex == g_playerMgr->localPlayer->physicalPlayerIndex)
	{
		TheClones->Log("%s: taking ownership of object id %d - stack trace:\n", __func__, netObject->objectId);

		uintptr_t* traceStart = (uintptr_t*)_AddressOfReturnAddress();

		for (int i = 0; i < 96; i++)
		{
			if ((i % 6) == 0)
			{
				TheClones->Log("\n");
			}

			TheClones->Log("%016llx ", traceStart[i]);
		}

		TheClones->Log("\n");
	}

	g_origSetOwner(netObject, newOwner);

	if (newOwner->physicalPlayerIndex == netObject__GetPlayerOwner(netObject)->physicalPlayerIndex)
	{
		return;
	}

	TheClones->Log("%s: passing object %016llx (%d) ownership from %d to %d\n", __func__, (uintptr_t)netObject, netObject->objectId, netObject->syncData.ownerId, newOwner->physicalPlayerIndex);

	ObjectIds_RemoveObjectId(netObject->objectId);

	netObject->syncData.nextOwnerId = 31;
	TheClones->SetTargetOwner(netObject, g_netIdsByPlayer[newOwner]);
}

static bool m158Stub(rage::netObject* object, CNetGamePlayer* player, int type, int* outReason)
{
	int reason;
	bool rv = object->m_158(player, type, &reason);

	if (!rv)
	{
		trace("couldn't pass control for reason %d\n", reason);
	}

	return rv;
}

static bool m168Stub(rage::netObject* object, int* outReason)
{
	int reason;
	bool rv = object->m_168(&reason);

	if (!rv)
	{
		//trace("couldn't blend object for reason %d\n", reason);
	}

	return rv;
}

static void(*g_origEjectPedFromVehicle)(char* vehicle, char* ped, uint8_t a3, uint8_t a4);

static void EjectPedFromVehicleStub(char* vehicle, char* ped, uint8_t a3, uint8_t a4)
{
	rage::netObject* vehObj = *(rage::netObject**)(vehicle + 208);
	rage::netObject* pedObj = *(rage::netObject**)(ped + 208);

	if (vehObj && pedObj)
	{
		trace("removing ped %d from vehicle %d from %016llx\n", pedObj->objectId, vehObj->objectId, (uintptr_t)_ReturnAddress());
	}

	g_origEjectPedFromVehicle(vehicle, ped, a3, a4);
}

static void(*g_origSetPedLastVehicle)(char*, char*);

static void SetPedLastVehicleStub(char* ped, char* vehicle)
{
	if (vehicle == nullptr)
	{
		rage::netObject* pedObj = *(rage::netObject**)(ped + 208);

		if (pedObj)
		{
			trace("removing ped %d from vehicle from %016llx\n", pedObj->objectId, (uintptr_t)_ReturnAddress());
		}
	}

	g_origSetPedLastVehicle(ped, vehicle);
}

int getPlayerId()
{
	return g_playerMgr->localPlayer->physicalPlayerIndex;
}

static bool mD0Stub(rage::netSyncTree* tree, int a2)
{
	if (icgi->OneSyncEnabled)
	{
		return false;
	}

	return tree->m_D0(a2);
}

static int(*g_origGetNetworkPlayerListCount)();
static CNetGamePlayer**(*g_origGetNetworkPlayerList)();

static int netInterface_GetNumRemotePhysicalPlayers()
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetNetworkPlayerListCount();
	}

	return g_playerListCountRemote;
}

static CNetGamePlayer** netInterface_GetRemotePhysicalPlayers()
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetNetworkPlayerList();
	}

	return g_playerListRemote;
}

static int(*g_origGetNetworkPlayerListCount2)();
static CNetGamePlayer**(*g_origGetNetworkPlayerList2)();

static int netInterface_GetNumPhysicalPlayers()
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetNetworkPlayerListCount2();
	}

	return g_playerListCount;
}

static CNetGamePlayer** netInterface_GetAllPhysicalPlayers()
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetNetworkPlayerList2();
	}

	return g_playerList;
}

static void netObject__ClearPendingPlayerIndex(rage::netObject* object)
{
	if (icgi->OneSyncEnabled)
	{
		if (object->syncData.nextOwnerId == 31 && TheClones->GetPendingClientId(object) != 0xFFFF)
		{
			return;
		}
	}

	object->syncData.nextOwnerId = -1;
}

static void*(*g_origGetScenarioTaskScenario)(void* scenarioTask);

static void* GetScenarioTaskScenario(char* scenarioTask)
{
	void* scenario = g_origGetScenarioTaskScenario(scenarioTask);

	if (!scenario)
	{
		static bool hasCrashedBefore = false;

		if (!hasCrashedBefore)
		{
			hasCrashedBefore = true;

			trace("WARNING: invalid scenario task triggered (scenario ID: %d)\n", *(uint32_t*)(scenarioTask + 168));
		}

		*(uint32_t*)(scenarioTask + 168) = 0;

		scenario = g_origGetScenarioTaskScenario(scenarioTask);

		assert(scenario);
	}

	return scenario;
}

static int(*g_origNetworkBandwidthMgr_CalculatePlayerUpdateLevels)(void* mgr, int* a2, int* a3, int* a4);

static int networkBandwidthMgr_CalculatePlayerUpdateLevelsStub(void* mgr, int* a2, int* a3, int* a4)
{
	if (icgi->OneSyncEnabled)
	{
		return 0;
	}

	return g_origNetworkBandwidthMgr_CalculatePlayerUpdateLevels(mgr, a2, a3, a4);
}

static int(*g_origGetNetObjPlayerGroup)(void* entity);

static int GetNetObjPlayerGroup(void* entity)
{
	// #TODO1S: groups
	// indexes a fixed-size array of 64, of which 32 are players, and 16+16 are script/code groups
	// we don't want to write past 32 ever, and _especially_ not past 64
	// this needs additional patching that currently isn't done.
	if (icgi->OneSyncEnabled)
	{
		return 0;
	}

	return g_origGetNetObjPlayerGroup(entity);
}

using TObjectPred = bool(*)(rage::netObject*);

static int(*g_origCountObjects)(rage::netObjectMgr* objectMgr, TObjectPred pred);

static int netObjectMgr__CountObjects(rage::netObjectMgr* objectMgr, TObjectPred pred)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origCountObjects(objectMgr, pred);
	}

	auto objectList = TheClones->GetObjectList();

	return std::count_if(objectList.begin(), objectList.end(), pred);
}

static void(*g_origObjectManager_End)(void*);

void ObjectManager_End(rage::netObjectMgr* objectMgr)
{
	if (icgi->OneSyncEnabled)
	{
		// don't run deletion if object manager is already shut down
		char* mgrPtr = (char*)objectMgr;

		if (*(bool*)(mgrPtr + 9992))
		{
			auto objectCb = [objectMgr](rage::netObject* object)
			{
				if (!object)
				{
					return;
				}

				// don't force-delete the local player
				if (object->objectType == (uint16_t)NetObjEntityType::Player && !object->syncData.isRemote)
				{
					objectMgr->UnregisterNetworkObject(object, 0, true, false);
					return;
				}

				objectMgr->UnregisterNetworkObject(object, 0, true, true);
			};

			for (int i = 0; i < 256; i++)
			{
				CloneObjectMgr->ForAllNetObjects(i, objectCb, true);
			}

			auto listCopy = TheClones->GetObjectList();

			for (auto netObj : listCopy)
			{
				objectCb(netObj);
			}
		}
	}

	g_origObjectManager_End(objectMgr);
}

static void(*g_origPlayerManager_End)(void*);

void PlayerManager_End(void* mgr)
{
	if (icgi->OneSyncEnabled)
	{
		g_netIdsByPlayer.clear();
		g_playersByNetId.clear();

		for (auto& p : g_players)
		{
			if (p)
			{
				if (p != g_playerMgr->localPlayer)
				{
					trace("player manager shutdown: resetting player %s\n", p->GetName());
					p->Reset();
				}
			}

			p = nullptr;
		}

		// reset the player list so we won't try removing _any_ players
		// #TODO1S: don't corrupt the physical linked list in the first place
		*(void**)((char*)g_playerMgr + 288) = nullptr;
		*(void**)((char*)g_playerMgr + 296) = nullptr;
		*(uint32_t*)((char*)g_playerMgr + 304) = 0;

		*(void**)((char*)g_playerMgr + 312) = nullptr;
		*(void**)((char*)g_playerMgr + 320) = nullptr;
		*(uint32_t*)((char*)g_playerMgr + 328) = 0;

		g_playerListCount = 0;
		g_playerListCountRemote = 0;
	}

	g_origPlayerManager_End(mgr);
}

namespace rage
{
	struct rlGamerId
	{
		uint64_t accountId;
	};
}

static rage::netPlayer*(*g_origGetPlayerFromGamerId)(rage::netPlayerMgrBase* mgr, const rage::rlGamerId& gamerId, bool flag);

static rage::netPlayer* GetPlayerFromGamerId(rage::netPlayerMgrBase* mgr, const rage::rlGamerId& gamerId, bool flag)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPlayerFromGamerId(mgr, gamerId, flag);
	}

	for (auto p : g_players)
	{
		if (p)
		{
			if (p->GetGamerInfo()->peerAddress.rockstarAccountId == gamerId.accountId)
			{
				return p;
			}
		}
	}

	return nullptr;
}

static float VectorDistance(const float* point1, const float* point2)
{
	float xd = point1[0] - point2[0];
	float yd = point1[1] - point2[1];
	float zd = point1[2] - point2[2];

	return sqrtf((xd * xd) + (yd * yd) + (zd * zd));
}

static hook::cdecl_stub<float*(float*, CNetGamePlayer*, void*, bool)> getNetPlayerRelevancePosition([]()
{
	return hook::get_pattern("45 33 FF 48 85 C0 0F 84 5B 01 00 00", -0x34);
});

static int(*g_origGetPlayersNearPoint)(const float* point, float range, CNetGamePlayer* outArray[32], bool sorted, bool unkVal);

static int GetPlayersNearPoint(const float* point, float range, CNetGamePlayer* outArray[32], bool sorted, bool unkVal)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPlayersNearPoint(point, range, outArray, sorted, unkVal);
	}

	CNetGamePlayer* tempArray[512];

	int idx = 0;

	auto playerList = netInterface_GetRemotePhysicalPlayers();
	for (int i = 0; i < netInterface_GetNumRemotePhysicalPlayers(); i++)
	{
		auto player = playerList[i];

		if (getPlayerPedForNetPlayer(player))
		{
			alignas(16) float vectorPos[4];

			if (range >= 100000000.0f || VectorDistance(point, getNetPlayerRelevancePosition(vectorPos, player, nullptr, unkVal)) < range)
			{
				tempArray[idx] = player;
				idx++;
			}
		}
	}

	if (sorted)
	{
		std::sort(tempArray, tempArray + idx, [point](CNetGamePlayer* a1, CNetGamePlayer* a2)
		{
			alignas(16) float vectorPos1[4];
			alignas(16) float vectorPos2[4];

			float d1 = VectorDistance(point, getNetPlayerRelevancePosition(vectorPos1, a1, nullptr, false));
			float d2 = VectorDistance(point, getNetPlayerRelevancePosition(vectorPos2, a2, nullptr, false));

			return (d1 < d2);
		});
	}

	idx = std::min(idx, 32);

	std::copy(tempArray, tempArray + idx, outArray);

	return idx;
}

static void(*g_origVoiceChatMgr_EstimateBandwidth)(void*);

static void VoiceChatMgr_EstimateBandwidth(void* mgr)
{
	if (icgi->OneSyncEnabled)
	{
		return;
	}

	g_origVoiceChatMgr_EstimateBandwidth(mgr);
}

static void(*g_origCPedGameStateDataNode__access)(void*, void*);

namespace sync
{
	extern net::Buffer g_cloneMsgPacket;
	extern std::vector<uint8_t> g_cloneMsgData;
}

static void CPedGameStateDataNode__access(char* dataNode, void* accessor)
{
	g_origCPedGameStateDataNode__access(dataNode, accessor);

	// not needed right now
	return;

	// 1604

	// if on mount/mount ID is set
	if (*(uint16_t*)(dataNode + 310) && icgi->OneSyncEnabled)
	{
		auto extraDumpPath = MakeRelativeCitPath(L"cache\\extra_dump_info.bin");

		auto f = _wfopen(extraDumpPath.c_str(), L"wb");

		if (f)
		{
			fwrite(sync::g_cloneMsgPacket.GetData().data(), 1, sync::g_cloneMsgPacket.GetData().size(), f);
			fclose(f);
		}

		extraDumpPath = MakeRelativeCitPath(L"cache\\extra_dump_info2.bin");

		f = _wfopen(extraDumpPath.c_str(), L"wb");

		if (f)
		{
			fwrite(sync::g_cloneMsgData.data(), 1, sync::g_cloneMsgData.size(), f);
			fclose(f);
		}

		FatalError("CPedGameStateDataNode: tried to read a mount ID, this is wrong, please click 'save information' below and post the file in https://forum.fivem.net/t/318260 to help us resolve this issue.");
	}
}

static void(*g_origManageHostMigration)(void*);

static void ManageHostMigrationStub(void* a1)
{
	if (!icgi->OneSyncEnabled)
	{
		g_origManageHostMigration(a1);
	}
}

static void(*g_origUnkBubbleWrap)();

static void UnkBubbleWrap()
{
	if (!icgi->OneSyncEnabled)
	{
		g_origUnkBubbleWrap();
	}
}

static void(*g_origUnkEventMgr)(void*, void*);

static void UnkEventMgr(void* mgr, void* ply)
{
	if (!icgi->OneSyncEnabled)
	{
		g_origUnkEventMgr(mgr, ply);
	}
}

static HookFunction hookFunction([]()
{
	// 1604
	g_playerMgr = (rage::netPlayerMgrBase*)hook::get_adjusted(0x142875710);

	// net damage array, size 32*4
	uint32_t* damageArrayReplacement = (uint32_t*)hook::AllocateStubMemory(256 * sizeof(uint32_t));
	memset(damageArrayReplacement, 0, 256 * sizeof(uint32_t));

	{
		std::initializer_list<std::tuple<std::string_view, int>> bits = {
			{ "74 30 3C 20 73 0D 48 8D 0D", 9 },
			{ "0F 85 9F 00 00 00 48 85 FF", 0x12 },
			{ "80 F9 FF 74 2F 48 8D 15", 8 },
			{ "80 BF 90 00 00 00 FF 74 21 48 8D", 12 }
		};

		for (const auto& bit : bits)
		{
			auto location = hook::get_pattern<int32_t>(std::get<0>(bit), std::get<1>(bit));

			*location = (intptr_t)damageArrayReplacement - (intptr_t)location - 4;
		}

		// 128
		hook::put<uint8_t>(hook::get_pattern("74 30 3C 20 73 0D 48 8D 0D", 3), 0x80);
	}

	// temp dbg
	//hook::put<uint16_t>(hook::get_pattern("0F 84 80 00 00 00 49 8B 07 49 8B CF FF 50 20"), 0xE990);

	//hook::put(0x141980A68, NetLogStub_DoLog);
	//hook::put(0x141980A50, NetLogStub_DoTrace);
	//hook::jump(0x140A19640, NetLogStub_DoLog);

	// netobjmgr count, temp dbg
	hook::put<uint8_t>(hook::get_pattern("48 8D 05 ? ? ? ? BE 1F 00 00 00 48 8B F9", 8), 128);

	// 1604, netobjmgr alloc size, temp dbg
	hook::put<uint32_t>(0x14101CF4F, 32712 + 8192);

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("4C 8B F1 41 BD 05", -0x22), PassObjectControlStub, (void**)&g_origPassObjectControl);
	MH_CreateHook(hook::get_pattern("8A 41 49 4C 8B F2 48 8B", -0x10), SetOwnerStub, (void**)&g_origSetOwner);

	// scriptHandlerMgr::ManageHostMigration, has fixed 32 player array and isn't needed* for 1s
	MH_CreateHook(hook::get_pattern("01 4F 60 81 7F 60 D0 07 00 00 0F 8E", -0x47), ManageHostMigrationStub, (void**)&g_origManageHostMigration);

	// 1604, some bubble stuff
	//hook::return_function(0x14104D148);
	MH_CreateHook(hook::get_pattern("33 F6 33 DB 33 ED 0F 28 80", -0x3A), UnkBubbleWrap, (void**)&g_origUnkBubbleWrap);

	MH_CreateHook(hook::get_pattern("0F 29 70 C8 0F 28 F1 33 DB 45", -0x1C), GetPlayersNearPoint, (void**)&g_origGetPlayersNearPoint);

	// func that reads neteventmgr by player idx, crashes page heap
	MH_CreateHook(hook::get_pattern("80 7A 2D FF 48 8B EA 48 8B F1 0F", -0x13), UnkEventMgr, (void**)&g_origUnkEventMgr);

	// return to disable breaking hooks
	//return;

	{
		auto location = hook::get_pattern("44 89 BE B4 00 00 00 FF 90", 7);
		hook::nop(location, 6);
		hook::call(location, m158Stub);
	}

	{
		auto location = hook::get_pattern("48 8B CF FF 90 70 01 00 00 84 C0 74 12 48 8B CF", 3);
		hook::nop(location, 6);

		// m170 in 1604 now
		hook::call(location, m168Stub);
	}

	// 1290
	// 1365
	// 1493
	// 1604
	MH_CreateHook((void*)hook::get_adjusted(0x1410AA870), AllocateNetPlayer, (void**)&g_origAllocateNetPlayer);

	MH_CreateHook(hook::get_pattern("8A 41 49 3C FF 74 17 3C 20 73 13 0F B6 C8"), netObject__GetPlayerOwner, (void**)&g_origGetOwnerNetPlayer);
	MH_CreateHook(hook::get_pattern("8A 41 4A 3C FF 74 17 3C 20 73 13 0F B6 C8"), netObject__GetPendingPlayerOwner, (void**)&g_origGetPendingPlayerOwner);

	// function is only 4 bytes, can't be hooked like this
	//MH_CreateHook(hook::get_call(hook::get_pattern("FF 50 68 49 8B CE E8 ? ? ? ? 48 8B 05", 6)), netObject__GetPlayerOwnerId, (void**)&g_origGetOwnerPlayerId);

	// NETWORK_GET_PLAYER_INDEX_FROM_PED
	{
		auto location = hook::get_pattern("48 85 C9 74 0A E8 ? ? ? ? 0F B6 C0", 5);
		hook::set_call(&g_origGetOwnerPlayerId, location);
		hook::call(location, netObject__GetPlayerOwnerId);
	}

	hook::jump(hook::get_pattern("C6 41 4A FF C3", 0), netObject__ClearPendingPlayerIndex);

	// replace joining local net player to bubble
	{
		auto location = hook::get_pattern("48 8B D0 E8 ? ? ? ? E8 ? ? ? ? 83 BB ? ? ? ? 04", 3);

		hook::set_call(&g_origJoinBubble, location);
		hook::call(location, JoinPhysicalPlayerOnHost);
	}

	{
		auto match = hook::pattern("80 F9 20 73 13 48 8B").count(2);
		MH_CreateHook(match.get(0).get<void>(0), GetPlayerByIndex, (void**)&g_origGetPlayerByIndex);
		MH_CreateHook(match.get(1).get<void>(0), GetPlayerByIndex, nullptr);
	}

	MH_CreateHook(hook::get_pattern("48 83 EC 28 33 C0 38 05 ? ? ? ? 74 0A"), GetPlayerByIndexNet, (void**)&g_origGetPlayerByIndexNet);

	MH_CreateHook(hook::get_pattern("75 07 85 C9 0F 94 C3 EB", -0x19), IsNetworkPlayerActive, (void**)&g_origIsNetworkPlayerActive);
	MH_CreateHook(hook::get_pattern("75 07 85 C9 0F 94 C0 EB", -0x13), IsNetworkPlayerConnected, (void**)&g_origIsNetworkPlayerConnected); // connected

	{
		auto location = hook::get_pattern<char>("44 0F 28 CF F3 41 0F 59 C0 F3 44 0F 59 CF F3 44 0F 58 C8 E8", 19);
		MH_CreateHook(hook::get_call(location + 0), netInterface_GetNumRemotePhysicalPlayers, (void**)&g_origGetNetworkPlayerListCount);
		MH_CreateHook(hook::get_call(location + 8), netInterface_GetRemotePhysicalPlayers, (void**)&g_origGetNetworkPlayerList);
	}

	{
		auto location = hook::get_pattern<char>("48 8B F0 85 DB 74 56 8B", -0x34);
		MH_CreateHook(hook::get_call(location + 0x28), netInterface_GetNumPhysicalPlayers, (void**)&g_origGetNetworkPlayerListCount2);
		MH_CreateHook(hook::get_call(location + 0x2F), netInterface_GetAllPhysicalPlayers, (void**)&g_origGetNetworkPlayerList2);
	}

	MH_CreateHook(hook::get_pattern("48 85 DB 74 20 48 8B 03 48 8B CB FF 50 30 48 8B", -0x34), GetPlayerFromGamerId, (void**)&g_origGetPlayerFromGamerId);

	MH_CreateHook(hook::get_pattern("4C 8B F9 74 7D", -0x2B), netObjectMgr__CountObjects, (void**)&g_origCountObjects);

	MH_CreateHook(hook::get_pattern("78 18 4C 8B 05", -10), GetScenarioTaskScenario, (void**)&g_origGetScenarioTaskScenario);

	MH_CreateHook(hook::get_pattern("0F 29 70 C8 4D 8B E1 4D 8B E8", -0x1C), networkBandwidthMgr_CalculatePlayerUpdateLevelsStub, (void**)&g_origNetworkBandwidthMgr_CalculatePlayerUpdateLevels);

	// #TODO1S: fix player/ped groups so we don't need this workaround anymore
	MH_CreateHook(hook::get_call(hook::get_pattern("48 83 C1 10 48 C1 E0 06 48 03 C8 E8", -21)), GetNetObjPlayerGroup, (void**)&g_origGetNetObjPlayerGroup);

	MH_CreateHook(hook::get_pattern("45 8D 65 20 C6 81 ? ? 00 00 01 48 8D 59 08", -0x2F), ObjectManager_End, (void**)&g_origObjectManager_End);
	MH_CreateHook(hook::get_call(hook::get_call(hook::get_pattern<char>("48 8D 05 ? ? ? ? 48 8B D9 48 89 01 E8 ? ? ? ? 84 C0 74 08 48 8B CB E8", -0x19) + 0x32)), PlayerManager_End, (void**)&g_origPlayerManager_End);

	// disable voice chat bandwidth estimation for 1s (it will overwrite some memory)
	MH_CreateHook(hook::get_pattern("40 8A 72 2D 40 80 FE FF 0F 84", -0x46), VoiceChatMgr_EstimateBandwidth, (void**)&g_origVoiceChatMgr_EstimateBandwidth);

	// crash logging for invalid mount indices
	MH_CreateHook(hook::get_pattern("48 8B FA 48 8D 91 44 01 00 00 48 8B F1", -0x16), CPedGameStateDataNode__access, (void**)&g_origCPedGameStateDataNode__access);

	// getnetplayerped 32 cap
	hook::nop(hook::get_pattern("83 F9 1F 77 26 E8", 3), 2);

	// always allow to migrate, even if not cloned on bit test
	hook::put<uint8_t>(hook::get_pattern("75 29 48 8B 02 48 8B CA FF 50 30"), 0xEB);

	// delete objects from clonemgr before deleting them
	static struct : public jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			mov(rcx, rbx);
			mov(rax, (uintptr_t)&DeletionMethod);
			jmp(rax);
		}

		static void DeletionMethod(rage::netObject* object)
		{
			if (icgi->OneSyncEnabled)
			{
				TheClones->OnObjectDeletion(object);
			}

			delete object;
		}
	} delStub;

	hook::call(hook::get_pattern("48 C1 EA 04 E8 ? ? ? ? 48 8B 03", 17), delStub.GetCode());

	// clobber nodes for all players, not just when connected to netplayermgr
	hook::nop(hook::get_pattern("0F A3 C7 73 18 44", 3), 2); // not working, maybe?

	// always write up-to-date data to nodes, not the cached data from syncdata
	{
		auto location = hook::get_pattern("FF 90 D0 00 00 00 84 C0 0F 84 80 00 00 00", 0);
		hook::nop(location, 6);
		hook::call(location, mD0Stub);
	}

	// hardcoded 32/128 array sizes in CNetObjEntity__TestProximityMigration
	{
		auto location = hook::get_pattern<char>("48 81 EC A0 01 00 00 33 DB 48 8B F9 38 1D", -0x15);

		// 0x20: scratch space
		// 256 * 8: 256 players, ptr size
		// 256 * 4: 256 players, int size
		auto stackSize = (0x20 + (256 * 8) + (256 * 4));
		auto ptrsBase = 0x20;
		auto intsBase = ptrsBase + (256 * 8);

		hook::put<uint32_t>(location + 0x18, stackSize);
		hook::put<uint32_t>(location + 0xF8, stackSize);

		hook::put<uint32_t>(location + 0xC9, intsBase);
	}

	// same for CNetObjPed
	{
		auto location = hook::get_pattern<char>("48 81 EC A0 01 00 00 33 FF 48 8B D9", -0x15);

		auto stackSize = (0x20 + (256 * 8) + (256 * 4));
		auto ptrsBase = 0x20;
		auto intsBase = ptrsBase + (256 * 8);

		hook::put<uint32_t>(location + 0x18, stackSize);
		hook::put<uint32_t>(location + 0x13C, stackSize);

		hook::put<uint32_t>(location + 0xD5, intsBase);
	}

	// 32 array size somewhere called by CTaskNMShot
	{
		auto location = hook::get_pattern<char>("48 8D A8 38 FF FF FF 48 81 ? ? ? ? 00 80 3D", -0x1C);

		hook::put<uint32_t>(location + 0x26, 0xA0 + (256 * 8));
		hook::put<uint32_t>(location + 0x269, 0xA0 + (256 * 8));
	}

	// CNetworkDamageTracker float[32] array
	// (would overflow into pool data and be.. quite bad)
	{
		// pool constructor call
		hook::put<uint32_t>(hook::get_pattern("C7 44 24 20 88 00 00 00 E8", 4), sizeof(void*) + (sizeof(float) * 256));

		// memset in CNetworkDamageTracker::ctor
		hook::put<uint32_t>(hook::get_pattern("48 89 11 48 8B D9 41 B8 80 00 00 00", 8), sizeof(float) * 256);
	}

	MH_EnableHook(MH_ALL_HOOKS);
});

// event stuff
static void* g_netEventMgr;

namespace rage
{
	class netGameEvent
	{
	public:
		virtual ~netGameEvent() = 0;

		virtual const char* GetName() = 0;

		virtual bool IsInScope(CNetGamePlayer* player) = 0;

		virtual bool TimeToResend(uint32_t) = 0;

		virtual bool CanChangeScope() = 0;

		virtual void Prepare(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void Handle(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual bool Decide(CNetGamePlayer* sourcePlayer, void* connUnk) = 0;

		virtual void PrepareReply(rage::datBitBuffer* buffer, CNetGamePlayer* replyPlayer) = 0;

		virtual void HandleReply(rage::datBitBuffer* buffer, CNetGamePlayer* sourcePlayer) = 0;

		virtual void PrepareExtraData(rage::datBitBuffer* buffer, bool isReply, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void HandleExtraData(rage::datBitBuffer* buffer, bool isReply, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void m_60() = 0;

		virtual void m_68() = 0;

		virtual void m_70() = 0;

		virtual void m_78() = 0;

		virtual bool Equals(const netGameEvent* event) = 0;

		virtual bool NotEquals(const netGameEvent* event) = 0;

		virtual bool MustPersist() = 0;

		virtual bool MustPersistWhenOutOfScope() = 0;

		virtual bool HasTimedOut() = 0;

	public:
		uint16_t eventId;

		uint8_t requiresReply : 1;
	};
}

static CNetGamePlayer* g_player31;

#include <chrono>

using namespace std::chrono_literals;

inline std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

// TODO: event expiration?
static std::map<uint16_t, std::tuple<rage::netGameEvent*, std::chrono::milliseconds>> g_events;
static uint16_t eventHeader;

static void(*g_origAddEvent)(void*, rage::netGameEvent*);

static void EventMgr_AddEvent(void* eventMgr, rage::netGameEvent* ev)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origAddEvent(eventMgr, ev);
	}

	// don't give control using events!
	if (strcmp(ev->GetName(), "GIVE_CONTROL_EVENT") == 0)
	{
		return;
	}

	// is this a duplicate event?
	for (auto& eventPair : g_events)
	{
		auto [key, tup] = eventPair;
		auto [event, time] = tup;

		if (event && event->Equals(ev))
		{
			delete ev;
			return;
		}
	}

	// TODO: use a real player for some things
	if (!g_player31)
	{
		g_player31 = AllocateNetPlayer(nullptr);
		g_player31->physicalPlayerIndex = 31;
	}

	//trace("packing a %s\n", ev->GetName());

	// allocate a RAGE buffer
	uint8_t packetStub[1024];
	rage::datBitBuffer rlBuffer(packetStub, sizeof(packetStub));

	ev->Prepare(&rlBuffer, g_player31, nullptr);
	ev->PrepareExtraData(&rlBuffer, false, g_player31, nullptr);

	net::Buffer outBuffer;

	// TODO: replace with bit array?
	std::set<int> targetPlayers;

	for (auto& player : g_players)
	{
		if (player && player->nonPhysicalPlayerData)
		{
			// temporary pointer check
			if ((uintptr_t)player->nonPhysicalPlayerData > 256)
			{
				// make it 31 for a while (objectmgr dependencies mandate this)
				auto originalIndex = player->physicalPlayerIndex;
				player->physicalPlayerIndex = (player != g_playerMgr->localPlayer) ? 31 : 0;

				if (ev->IsInScope(player))
				{
					targetPlayers.insert(g_netIdsByPlayer[player]);
				}

				player->physicalPlayerIndex = originalIndex;
			}
			else
			{
				AddCrashometry("player_corruption", "true");
			}
		}
	}

	outBuffer.Write<uint8_t>(targetPlayers.size());

	for (int playerId : targetPlayers)
	{
		outBuffer.Write<uint16_t>(playerId);
	}

	outBuffer.Write<uint16_t>(eventHeader);
	outBuffer.Write<uint8_t>(0);
	outBuffer.Write<uint16_t>(ev->eventId);
	
	uint32_t len = rlBuffer.GetDataLength();
	outBuffer.Write<uint16_t>(len); // length (short)
	outBuffer.Write(rlBuffer.m_data, len); // data

	g_netLibrary->SendReliableCommand("msgNetGameEvent", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());

	// we don't need the event anymore
	g_events[eventHeader] = { ev, msec() };

	++eventHeader;
}

static atPoolBase** g_netGameEventPool;

static void EventManager_Update()
{
	if (!*g_netGameEventPool)
	{
		return;
	}

	for (auto& eventPair : g_events)
	{
		auto [ev, time] = eventPair.second;

		if (ev)
		{
			auto expiryDuration = 5s;

			if (ev->HasTimedOut() || (msec() - time) > expiryDuration)
			{
				delete ev;

				eventPair.second = { };
			}
		}
	}
}

static void HandleNetGameEvent(const char* idata, size_t len)
{
	if (!icgi->HasVariable("networkInited"))
	{
		return;
	}

	// TODO: use a real player for some things that _are_ 32-safe
	if (!g_player31)
	{
		g_player31 = AllocateNetPlayer(nullptr);
		g_player31->physicalPlayerIndex = 31;
	}

	net::Buffer buf(reinterpret_cast<const uint8_t*>(idata), len);
	auto sourcePlayerId = buf.Read<uint16_t>();
	auto eventHeader = buf.Read<uint16_t>();
	auto isReply = buf.Read<uint8_t>();
	auto eventType = buf.Read<uint16_t>();
	auto length = buf.Read<uint16_t>();

	auto player = g_playersByNetId[sourcePlayerId];

	if (!player)
	{
		player = g_player31;
	}

	std::vector<uint8_t> data(length);
	buf.Read(data.data(), data.size());

	rage::datBitBuffer rlBuffer(const_cast<uint8_t*>(data.data()), data.size());
	rlBuffer.m_f1C = 1;

	if (eventType > 0x5A)
	{
		return;
	}

	if (isReply)
	{
		auto [ ev, time ] = g_events[eventHeader];

		if (ev)
		{
			ev->HandleReply(&rlBuffer, player);
			ev->HandleExtraData(&rlBuffer, true, player, g_playerMgr->localPlayer);

			delete ev;
			g_events[eventHeader] = {};
		}
	}
	else
	{
		using TEventHandlerFn = void(*)(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t, uint32_t, uint32_t);

		// for all intents and purposes, the player will be 31
		auto lastIndex = player->physicalPlayerIndex;
		player->physicalPlayerIndex = 31;

		auto eventMgr = *(char**)g_netEventMgr;

		if (eventMgr)
		{
			auto eventHandlerList = (TEventHandlerFn*)(eventMgr + 0x3AB80);
			auto eh = eventHandlerList[eventType];
			
			if (eh && (uintptr_t)eh >= hook::get_adjusted(0x140000000) && (uintptr_t)eh < hook::get_adjusted(0x146000000))
			{
				eh(&rlBuffer, player, g_playerMgr->localPlayer, eventHeader, 0, 0);
			}
		}

		player->physicalPlayerIndex = lastIndex;
	}
}

static void(*g_origExecuteNetGameEvent)(void* eventMgr, rage::netGameEvent* ev, rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t evH, uint32_t, uint32_t);

static void ExecuteNetGameEvent(void* eventMgr, rage::netGameEvent* ev, rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t evH, uint32_t a, uint32_t b)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origExecuteNetGameEvent(eventMgr, ev, buffer, player, unkConn, evH, a, b);
	}

	//trace("executing a %s\n", ev->GetName());

	ev->Handle(buffer, player, unkConn);

	// missing: some checks
	if (ev->Decide(player, unkConn))
	{
		ev->HandleExtraData(buffer, false, player, unkConn);

		if (ev->requiresReply)
		{
			uint8_t packetStub[1024];
			rage::datBitBuffer rlBuffer(packetStub, sizeof(packetStub));

			ev->PrepareReply(&rlBuffer, player);
			ev->PrepareExtraData(&rlBuffer, true, player, nullptr);

			net::Buffer outBuffer;
			outBuffer.Write<uint8_t>(1);
			outBuffer.Write<uint16_t>(g_netIdsByPlayer[player]);

			outBuffer.Write<uint16_t>(evH);
			outBuffer.Write<uint8_t>(1);
			outBuffer.Write<uint16_t>(ev->eventId);

			uint32_t len = rlBuffer.GetDataLength();
			outBuffer.Write<uint16_t>(len); // length (short)
			outBuffer.Write(rlBuffer.m_data, len); // data

			g_netLibrary->SendReliableCommand("msgNetGameEvent", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());
		}
	}
}

static InitFunction initFunctionEv([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		icgi = Instance<ICoreGameInit>::Get();

		netLibrary->OnClientInfoReceived.Connect([](const NetLibraryClientInfo& info)
		{
			if (!icgi->OneSyncEnabled)
			{
				return;
			}

			if (g_players[info.slotId])
			{
				trace("Dropping duplicate player for slotID %d.\n", info.slotId);
				HandleClientDrop(info);
			}

			HandleClientInfo(info);
		});

		netLibrary->OnClientInfoDropped.Connect([](const NetLibraryClientInfo& info)
		{
			if (!icgi->OneSyncEnabled)
			{
				return;
			}

			HandleClientDrop(info);
		});

		netLibrary->AddReliableHandler("msgNetGameEvent", [](const char* data, size_t len)
		{
			if (!icgi->OneSyncEnabled)
			{
				return;
			}

			HandleNetGameEvent(data, len);
		}, true);
	});
});

static bool(*g_origSendGameEvent)(void*, void*);

static bool SendGameEvent(void* eventMgr, void* ev)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSendGameEvent(eventMgr, ev);
	}

	return 1;
}

static uint32_t(*g_origGetFireApplicability)(void* event, void* pos);

static uint32_t GetFireApplicability(void* event, void* pos)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetFireApplicability(event, pos);
	}

	// send all fires to all remote players
	return (1 << 31);
}

static HookFunction hookFunctionEv([]()
{
	MH_Initialize();

	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 84 C0 74 11 48 8B 0D", 0x14);
		g_netEventMgr = hook::get_address<void*>(location);
		MH_CreateHook(hook::get_call(location + 7), EventMgr_AddEvent, (void**)&g_origAddEvent);
	}

	MH_CreateHook(hook::get_pattern("48 8B DA 48 8B F1 41 81 FF 00", -0x2A), ExecuteNetGameEvent, (void**)&g_origExecuteNetGameEvent);

	// can cause crashes due to high player indices, default event sending
	MH_CreateHook(hook::get_pattern("48 83 EC 30 80 7A 2D FF 4C 8B D2", -0xC), SendGameEvent, (void**)&g_origSendGameEvent);

	// fire applicability
	MH_CreateHook(hook::get_pattern("85 DB 74 78 44 8B F3 48", -0x30), GetFireApplicability, (void**)&g_origGetFireApplicability);

	MH_EnableHook(MH_ALL_HOOKS);

	{
		auto location = hook::get_pattern("44 8B 40 20 8B 40 10 41 C1 E0 02 41 C1 F8 02 41 2B C0 0F 85", -7);
		g_netGameEventPool = hook::get_address<decltype(g_netGameEventPool)>(location);
	}
});

#include <nutsnbolts.h>
#include <GameInit.h>

static char(*g_origWriteDataNode)(void* node, uint32_t flags, void* mA0, rage::netObject* object, rage::datBitBuffer* buffer, int time, void* playerObj, char playerId, void* unk);


struct VirtualBase
{
	virtual ~VirtualBase() {}
};

struct VirtualDerivative : public VirtualBase
{
	virtual ~VirtualDerivative() override {}
};

std::string GetType(void* d)
{
	VirtualBase* self = (VirtualBase*)d;

	std::string typeName = fmt::sprintf("unknown (vtable %p)", *(void**)self);

	try
	{
		typeName = typeid(*self).name();
	}
	catch (std::__non_rtti_object&)
	{

	}

	return typeName;
}

extern rage::netObject* g_curNetObject;

static char(*g_origReadDataNode)(void* node, uint32_t flags, void* mA0, rage::datBitBuffer* buffer, rage::netObject* object);

std::map<int, std::map<void*, std::tuple<int, uint32_t>>> g_netObjectNodeMapping;

static bool ReadDataNodeStub(void* node, uint32_t flags, void* mA0, rage::datBitBuffer* buffer, rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origReadDataNode(node, flags, mA0, buffer, object);
	}

	// enable this for boundary checks
	/*if (flags == 1 || flags == 2)
	{
		uint32_t in = 0;
		buffer->ReadInteger(&in, 8);
		assert(in == 0x5A);
	}*/

	bool didRead = g_origReadDataNode(node, flags, mA0, buffer, object);

	if (didRead && g_curNetObject)
	{
		g_netObjectNodeMapping[g_curNetObject->objectId][node] = { 0, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() };
	}

	return didRead;
}

static bool WriteDataNodeStub(void* node, uint32_t flags, void* mA0, rage::netObject* object, rage::datBitBuffer* buffer, int time, void* playerObj, char playerId, void* unk)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk);
	}

	if (playerId != 31 || flags == 4)
	{
		return g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk);
	}
	else
	{
		// save position and write a placeholder length frame
		uint32_t position = buffer->GetPosition();
		buffer->WriteBit(false);
		buffer->WriteUns(0, 11);

		bool rv = g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk);

		// write the actual length on top of the position
		uint32_t endPosition = buffer->GetPosition();
		auto length = endPosition - position - 11 - 1;

		if (length > 1 || flags == 1)
		{
			buffer->Seek(position);

			buffer->WriteBit(true);
			buffer->WriteUns(length, 11);
			buffer->Seek(endPosition);

			if (g_curNetObject)
			{
				g_netObjectNodeMapping[g_curNetObject->objectId][node] = { 1, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() };
			}

			//trace("actually wrote %s\n", GetType(node));
		}
		else
		{
			buffer->Seek(position + 1);
		}

		return rv;
	}
}

static void(*g_origUpdateSyncDataOn108)(void* node, void* object);

static void UpdateSyncDataOn108Stub(void* node, void* object)
{
	//if (!icgi->OneSyncEnabled)
	{
		g_origUpdateSyncDataOn108(node, object);
	}
}

static void(*g_origManuallyDirtyNode)(void* node, void* object);

extern void DirtyNode(void* object, void* node);

static void ManuallyDirtyNodeStub(void* node, void* object)
{
	if (!icgi->OneSyncEnabled)
	{
		g_origManuallyDirtyNode(node, object);
		return;
	}

	DirtyNode(object, node);
}

static void(*g_orig_netSyncDataNode_ForceSend)(void* node, int actFlag1, int actFlag2, rage::netObject* object);

static void netSyncDataNode_ForceSendStub(void* node, int actFlag1, int actFlag2, rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		g_orig_netSyncDataNode_ForceSend(node, actFlag1, actFlag2, object);
		return;
	}

	// maybe needs to read act flags?
	DirtyNode(object, node);
}

static void(*g_orig_netSyncDataNode_ForceSendToPlayer)(void* node, int player, int actFlag1, int actFlag2, rage::netObject* object);

static void netSyncDataNode_ForceSendToPlayerStub(void* node, int player, int actFlag1, int actFlag2, rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		g_orig_netSyncDataNode_ForceSendToPlayer(node, player, actFlag1, actFlag2, object);
		return;
	}

	// maybe needs to read act flags?
	DirtyNode(object, node);
}

static void(*g_origCallSkip)(void* a1, void* a2, void* a3, void* a4, void* a5);

static void SkipCopyIf1s(void* a1, void* a2, void* a3, void* a4, void* a5)
{
	if (!icgi->OneSyncEnabled)
	{
		g_origCallSkip(a1, a2, a3, a4, a5);
	}
}

static HookFunction hookFunction2([]()
{
	// 2 matches, 1st is data, 2nd is parent
	{
		auto location = hook::get_pattern<char>("48 89 44 24 20 E8 ? ? ? ? 84 C0 0F 95 C0 48 83 C4 58", -0x3C);
		hook::set_call(&g_origWriteDataNode, location + 0x41);
		hook::jump(location, WriteDataNodeStub);
	}

	{
		auto location = hook::get_pattern("48 8B 03 48 8B D6 48 8B CB EB 06", -0x48);
		MH_Initialize();
		MH_CreateHook(location, ReadDataNodeStub, (void**)&g_origReadDataNode);

		{
			auto floc = hook::get_pattern<char>("84 C0 0F 84 39 01 00 00 48 83 7F", -0x29);
			hook::set_call(&g_origCallSkip, floc + 0xD9);
			hook::call(floc + 0xD9, SkipCopyIf1s);
			MH_CreateHook(floc, UpdateSyncDataOn108Stub, (void**)&g_origUpdateSyncDataOn108);
		}

		MH_CreateHook(hook::get_pattern("48 83 79 48 00 48 8B D9 74 19", -6), ManuallyDirtyNodeStub, (void**)&g_origManuallyDirtyNode);

		MH_CreateHook(hook::get_pattern("85 51 28 0F 84 E4 00 00 00 33 DB", -0x24), netSyncDataNode_ForceSendStub, (void**)&g_orig_netSyncDataNode_ForceSend);

		MH_CreateHook(hook::get_pattern("44 85 41 28 74 73 83 79 30 00", -0x1F), netSyncDataNode_ForceSendToPlayerStub, (void**)&g_orig_netSyncDataNode_ForceSendToPlayer);

		MH_EnableHook(MH_ALL_HOOKS);
	}
});

class netTimeSync
{
public:
	void Update();

	void HandleTimeSync(net::Buffer& buffer);

	bool IsInitialized();

	inline void SetConnectionManager(void* mgr)
	{
		m_connectionMgr = mgr;
	}

private:
	void* m_vtbl; // 0
	void* m_connectionMgr; // 8
	struct {
		uint32_t m_int1;
		uint16_t m_short1;
		uint32_t m_int2;
		uint16_t m_short2;
	} m_trustAddr; // 16
	uint32_t m_sessionKey; // 32
	int32_t m_timeDelta; // 36
	struct {
		void* self;
		void* cb;
	} messageDelegate; // 40
	char m_pad_38[32]; // 56
	uint32_t m_nextSync; // 88
	uint32_t m_configTimeBetweenSyncs; // 92
	uint32_t m_configMaxBackoff; // 96, usually 60000
	uint32_t m_effectiveTimeBetweenSyncs; // 100
	uint32_t m_lastRtt; // 104
	uint32_t m_retryCount; // 108
	uint32_t m_requestSequence; // 112
	uint32_t m_replySequence; // 116
	uint32_t m_flags; // 120
	uint32_t m_packetFlags; // 124
	uint32_t m_lastTime; // 128, used to prevent time from going backwards
	uint8_t m_applyFlags; // 132
	uint8_t m_disabled; // 133
};

#include <mmsystem.h>

static hook::cdecl_stub<void*()> _getConnectionManager([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? C7 44 24 40 60 EA 00 00"));
});

static bool(*g_origInitializeTime)(netTimeSync* timeSync, void* connectionMgr, int flags, void* trustHost,
	uint32_t sessionSeed, int* deltaStart, int packetFlags, int initialBackoff, int maxBackoff);

static bool g_initedTimeSync;

bool netTimeSync::IsInitialized()
{
	if (!g_initedTimeSync)
	{
		g_origInitializeTime(this, _getConnectionManager(), 1, nullptr, 0, nullptr, 7, 2000, 60000);

		// to make the game not try to get time from us
		m_connectionMgr = nullptr;

		g_initedTimeSync = true;

		return false;
	}

	return (m_applyFlags & 4) != 0;
}

void netTimeSync::Update()
{
	if (!icgi->OneSyncEnabled)
	{
		return;
	}

	if (/*m_connectionMgr /*&& m_flags & 2 && */!m_disabled)
	{
		uint32_t curTime = timeGetTime();

		if (!m_nextSync || int32_t(timeGetTime() - m_nextSync) >= 0)
		{
			m_requestSequence++;

			net::Buffer outBuffer;
			outBuffer.Write<uint32_t>(curTime); // request time
			outBuffer.Write<uint32_t>(m_requestSequence); // request sequence

			g_netLibrary->SendReliableCommand("msgTimeSyncReq", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());

			m_nextSync = (curTime + m_effectiveTimeBetweenSyncs) | 1;
		}
	}
}

void netTimeSync::HandleTimeSync(net::Buffer& buffer)
{
	auto reqTime = buffer.Read<uint32_t>();
	auto reqSequence = buffer.Read<uint32_t>();
	auto resDelta = buffer.Read<uint32_t>();

	if (m_disabled)
	{
		return;
	}

	/*if (!(m_flags & 2))
	{
		return;
	}*/

	// out of order?
	if (int32_t(reqSequence - m_replySequence) <= 0)
	{
		return;
	}

	auto rtt = timeGetTime() - reqTime;

	// bad timestamp, negative time passed
	if (int32_t(rtt) <= 0)
	{
		return;
	}

	int32_t timeDelta = resDelta + (rtt / 2) - timeGetTime();

	// is this a low RTT, or did we retry often enough?
	if (rtt <= 300 || m_retryCount >= 10)
	{
		if (!m_lastRtt)
		{
			m_lastRtt = rtt;
		}

		// is RTT within variance, low, or retried?
		if (rtt <= 100 ||
			(rtt / m_lastRtt) < 2 ||
			m_retryCount >= 10)
		{
			m_timeDelta = timeDelta;
			m_replySequence = reqSequence;

			// progressive backoff once we've established a valid time base
			if (m_effectiveTimeBetweenSyncs < m_configMaxBackoff)
			{
				m_effectiveTimeBetweenSyncs = std::min(m_configMaxBackoff, m_effectiveTimeBetweenSyncs * 2);
			}

			m_retryCount = 0;

			// use flag 4 to reset time at least once, even if game session code has done so to a higher value
			if (!(m_applyFlags & 4))
			{
				m_lastTime = m_timeDelta + timeGetTime();
			}

			m_applyFlags |= 7;
		}
		else
		{
			m_nextSync = 0;
			m_retryCount++;
		}

		// update average RTT
		m_lastRtt = (rtt + m_lastRtt) / 2;
	}
	else
	{
		m_nextSync = 0;
		m_retryCount++;
	}
}

static netTimeSync** g_netTimeSync;

bool IsWaitingForTimeSync()
{
	return !(*g_netTimeSync)->IsInitialized();
}

static InitFunction initFunctionTime([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		lib->AddReliableHandler("msgTimeSync", [](const char* data, size_t len)
		{
			net::Buffer buf(reinterpret_cast<const uint8_t*>(data), len);
			(*g_netTimeSync)->HandleTimeSync(buf);
		});
	});
});

bool netTimeSync__InitializeTimeStub(netTimeSync* timeSync, void* connectionMgr, int flags, void* trustHost,
	uint32_t sessionSeed, int* deltaStart, int packetFlags, int initialBackoff, int maxBackoff)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origInitializeTime(timeSync, connectionMgr, flags, trustHost, sessionSeed, deltaStart, packetFlags, initialBackoff, maxBackoff);
	}

	timeSync->SetConnectionManager(connectionMgr);

	return true;
}

static HookFunction hookFunctionTime([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 8B D9 48 39 79 08 0F 85 B5 00 00 00", -32), netTimeSync__InitializeTimeStub, (void**)&g_origInitializeTime);
	MH_EnableHook(MH_ALL_HOOKS);

	g_netTimeSync = hook::get_address<netTimeSync**>(hook::get_pattern("EB 16 48 8B 0D ? ? ? ? 45 33 C9 45 33 C0", 5));

	OnMainGameFrame.Connect([]()
	{
		(*g_netTimeSync)->Update();
	});
});

struct WorldGridEntry
{
	uint8_t sectorX;
	uint8_t sectorY;
	uint8_t slotID;
};

struct WorldGridState
{
	WorldGridEntry entries[12];
};

static WorldGridState g_worldGrid[256];

static InitFunction initFunctionWorldGrid([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		lib->AddReliableHandler("msgWorldGrid", [](const char* data, size_t len)
		{
			net::Buffer buf(reinterpret_cast<const uint8_t*>(data), len);
			auto base = buf.Read<uint16_t>();
			auto length = buf.Read<uint16_t>();

			if ((base + length) > sizeof(g_worldGrid))
			{
				return;
			}

			buf.Read(reinterpret_cast<char*>(g_worldGrid) + base, length);
		});
	});
});

bool(*g_origDoesLocalPlayerOwnWorldGrid)(float* pos);

bool DoesLocalPlayerOwnWorldGrid(float* pos)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origDoesLocalPlayerOwnWorldGrid(pos);
	}

	auto playerIdx = g_playerMgr->localPlayer->physicalPlayerIndex;

	int sectorX = std::max(pos[0] + 8192.0f, 0.0f) / 75;
	int sectorY = std::max(pos[1] + 8192.0f, 0.0f) / 75;

	bool does = false;

	for (const auto& entry : g_worldGrid[playerIdx].entries)
	{
		if (entry.sectorX == sectorX && entry.sectorY == sectorY && entry.slotID == playerIdx)
		{
			does = true;
			break;
		}
	}

	return does;
}

static int GetScriptParticipantIndexForPlayer(CNetGamePlayer* player)
{
	return player->physicalPlayerIndex;
}

static HookFunction hookFunctionWorldGrid([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("44 8A 40 2D 41", -0x1B), DoesLocalPlayerOwnWorldGrid, (void**)&g_origDoesLocalPlayerOwnWorldGrid);
	MH_EnableHook(MH_ALL_HOOKS);

	// if population breaks in non-1s, this is possibly why
	//hook::nop(hook::get_pattern("38 05 ? ? ? ? 75 0A 48 8B CF E8", 6), 2);

	// 1493+ of the above patch
	//hook::nop(hook::get_pattern("80 3D ? ? ? ? 00 75 0A 48 8B CF E8", 7), 2);

	// this patch above ^ shouldn't be needed with timeSync properly implemented, gamerIDs being set and RemotePlayer list fixes

	// this should apply to both 1s and non-1s (as participants are - hopefully? - not used by anyone in regular net)
	hook::jump(hook::get_pattern("84 C0 74 06 0F BF 43 38", -0x18), GetScriptParticipantIndexForPlayer);
});

int ObjectToEntity(int objectId)
{
	int entityIdx = -1;
	auto object = TheClones->GetNetObject(objectId & 0xFFFF);

	if (object)
	{
		entityIdx = getScriptGuidForEntity(object->GetGameObject());
	}

	return entityIdx;
}

#include <boost/range/adaptor/map.hpp>
#include <mmsystem.h>

struct ObjectData
{
	uint32_t lastSyncTime;
	uint32_t lastSyncAck;

	ObjectData()
	{
		lastSyncTime = 0;
		lastSyncAck = 0;
	}
};

static std::map<int, ObjectData> trackedObjects;

#include <lz4.h>

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		if (g_netLibrary == nullptr)
		{
			return;
		}

		if (g_netLibrary->GetConnectionState() != NetLibrary::CS_ACTIVE)
		{
			return;
		}

		// protocol 5 or higher are aware of this state
		if (g_netLibrary->GetServerProtocol() <= 4)
		{
			return;
		}

		// only work with 1s enabled
		if (!icgi->OneSyncEnabled)
		{
			return;
		}

		EventManager_Update();
		TheClones->Update();
	});

	OnKillNetwork.Connect([](const char*)
	{
		g_events.clear();
	});

	OnKillNetworkDone.Connect([]()
	{
		trackedObjects.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_ENTITY_OWNER", [](fx::ScriptContext& context)
	{
		fwEntity* entity = (fwEntity*)getScriptEntity(context.GetArgument<int>(0));
		
		if (!entity)
		{
			context.SetResult<int>(-1);
			return;
		}

		rage::netObject* netObj = (rage::netObject*)entity->GetNetObject();

		if (!netObj)
		{
			context.SetResult<int>(-1);
			return;
		}

		auto owner = netObject__GetPlayerOwner(netObj);
		context.SetResult<int>(owner->physicalPlayerIndex);
	});
#if 0
	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_SAVE_CLONE_CREATE", [](fx::ScriptContext& context)
	{
		char* entity = (char*)getScriptEntity(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("SAVE_CLONE_CREATE: invalid entity\n");

			context.SetResult<const char*>("");
			return;
		}

		auto netObj = *(rage::netObject**)(entity + 208);

		static char blah[90000];

		static char bluh[1000];
		memset(bluh, 0, sizeof(bluh));
		memset(blah, 0, sizeof(blah));

		rage::netBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		st = getSyncTreeForType(nullptr, (int)NetObjEntityType::Ped);
		st->WriteTree(1, 0, netObj, &buffer, g_queryFunctions->GetTimestamp(), nullptr, 31, nullptr);

		static char base64Buffer[2000];

		size_t outLength = sizeof(base64Buffer);
		char* txt = base64_encode((uint8_t*)buffer.m_data, (buffer.m_curBit / 8) + 1, &outLength);

		memcpy(base64Buffer, txt, outLength);
		free(txt);

		base64Buffer[outLength] = '\0';

		context.SetResult<const char*>(base64Buffer);
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_SAVE_CLONE_SYNC", [](fx::ScriptContext& context)
	{
		char* entity = (char*)getScriptEntity(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("SAVE_CLONE_SYNC: invalid entity\n");

			context.SetResult<const char*>("");
			return;
		}

		auto netObj = *(rage::netObject**)(entity + 208);

		static char blah[90000];

		static char bluh[1000];
		memset(bluh, 0, sizeof(bluh));
		memset(blah, 0, sizeof(blah));

		rage::netBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		st = getSyncTreeForType(nullptr, (int)NetObjEntityType::Ped);
		st->WriteTree(2, 0, netObj, &buffer, g_queryFunctions->GetTimestamp(), nullptr, 31, nullptr);

		static char base64Buffer[2000];

		size_t outLength = sizeof(base64Buffer);
		char* txt = base64_encode((uint8_t*)buffer.m_data, (buffer.m_curBit / 8) + 1, &outLength);

		memcpy(base64Buffer, txt, outLength);
		free(txt);

		trace("saving netobj %llx\n", (uintptr_t)netObj);

		base64Buffer[outLength] = '\0';

		context.SetResult<const char*>(base64Buffer);
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_LOAD_CLONE_CREATE", [](fx::ScriptContext& context)
	{
		auto data = context.GetArgument<const char*>(0);
		auto objectId = context.GetArgument<uint16_t>(1);
		auto objectType = context.GetArgument<const char*>(2);

		NetObjEntityType objType;

		if (strcmp(objectType, "automobile") == 0)
		{
			objType = NetObjEntityType::Automobile;
		}
		else if (strcmp(objectType, "player") == 0)
		{
			objType = NetObjEntityType::Ped; // until we make native players
		}
		else if (strcmp(objectType, "ped") == 0)
		{
			objType = NetObjEntityType::Ped;
		}
		else
		{
			context.SetResult(-1);
			return;
		}

		//trace("making a %d\n", (int)objType);

		size_t decLen;
		uint8_t* dec = base64_decode(data, strlen(data), &decLen);

		rage::netBuffer buf(dec, decLen);
		buf.m_f1C = 1;

		auto st = getSyncTreeForType(nullptr, (int)objType);

		netSyncTree_ReadFromBuffer(st, 1, 0, &buf, nullptr);

		free(dec);

		auto obj = createCloneFuncs[objType](objectId, 31, 0, 32);
		*((uint8_t*)obj + 75) = 1;

		if (!netSyncTree_CanApplyToObject(st, obj))
		{
			trace("Couldn't apply object.\n");

			delete obj;

			context.SetResult(-1);
			return;
		}

		st->ApplyToObject(obj, nullptr);
		rage::netObjectMgr::GetInstance()->RegisterNetworkObject(obj);

		netBlender_SetTimestamp(obj->GetBlender(), g_queryFunctions->GetTimestamp());

		obj->m_1D0();

		obj->GetBlender()->ApplyBlend();
		obj->GetBlender()->m_38();

		obj->m_1C0();

		context.SetResult(getScriptGuidForEntity(obj->GetGameObject()));
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_LOAD_CLONE_SYNC", [](fx::ScriptContext& context)
	{
		char* entity = (char*)getScriptEntity(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("LOAD_CLONE_SYNC: invalid entity\n");
			return;
		}

		auto obj = *(rage::netObject**)(entity + 208);

		auto data = context.GetArgument<const char*>(1);
		
		size_t decLen;
		uint8_t* dec = base64_decode(data, strlen(data), &decLen);

		rage::netBuffer buf(dec, decLen);
		buf.m_f1C = 1;

		auto st = obj->GetSyncTree();

		netSyncTree_ReadFromBuffer(st, 2, 0, &buf, nullptr);

		free(dec);

		netBlender_SetTimestamp(obj->GetBlender(), g_queryFunctions->GetTimestamp());
		obj->GetBlender()->m_28();

		st->ApplyToObject(obj, nullptr);

		obj->m_1D0();

		//obj->GetBlender()->m_30();
		obj->GetBlender()->m_58();

		//obj->GetBlender()->ApplyBlend();
		//obj->GetBlender()->m_38();

		//obj->m_1C0();
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_BLEND", [](fx::ScriptContext& context)
	{
		char* entity = (char*)getScriptEntity(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("LOAD_CLONE_SYNC: invalid entity\n");
			return;
		}

		auto obj = *(rage::netObject**)(entity + 208);
		//obj->GetBlender()->m_30();
		obj->GetBlender()->m_58();
	});

	static ConsoleCommand saveCloneCmd("save_clone", [](const std::string& address)
	{
		uintptr_t addressPtr = _strtoui64(address.c_str(), nullptr, 16);
		auto netObj = *(rage::netObject**)(addressPtr + 208);

		static char blah[90000];

		static char bluh[1000];

		rage::netBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		st->WriteTree(1, 0, netObj, &buffer, g_queryFunctions->GetTimestamp(), blah, 31, nullptr);

		FILE* f = _wfopen(MakeRelativeCitPath(L"tree.bin").c_str(), L"wb");
		fwrite(buffer.m_data, 1, (buffer.m_curBit / 8) + 1, f);
		fclose(f);
	});

	static ConsoleCommand loadCloneCmd("load_clone", []()
	{
		FILE* f = _wfopen(MakeRelativeCitPath(L"tree.bin").c_str(), L"rb");
		fseek(f, 0, SEEK_END);

		int len = ftell(f);

		fseek(f, 0, SEEK_SET);

		uint8_t data[1000];
		fread(data, 1, 1000, f);

		fclose(f);

		rage::netBuffer buf(data, len);
		buf.m_f1C = 1;

		auto st = getSyncTreeForType(nullptr, 0);

		netSyncTree_ReadFromBuffer(st, 1, 0, &buf, nullptr);

		auto obj = createCloneFuncs[NetObjEntityType::Automobile](rand(), 31, 0, 32);

		if (!netSyncTree_CanApplyToObject(st, obj))
		{
			trace("Couldn't apply object.\n");

			delete obj;
			return;
		}

		st->ApplyToObject(obj, nullptr);
		rage::netObjectMgr::GetInstance()->RegisterNetworkObject(obj);

		netBlender_SetTimestamp(obj->GetBlender(), g_queryFunctions->GetTimestamp());

		obj->m_1D0();

		obj->GetBlender()->ApplyBlend();
		obj->GetBlender()->m_38();

		obj->m_1C0();

		trace("got game object %llx\n", (uintptr_t)obj->GetGameObject());
	});
#endif
});
