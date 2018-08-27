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

extern NetLibrary* g_netLibrary;

class CNetGamePlayer;

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
static rage::netPlayerMgrBase* g_playerMgr = (rage::netPlayerMgrBase*)0x1427D2320;

void* g_tempRemotePlayer;

static CNetGamePlayer* g_players[256];
static std::unordered_map<uint16_t, CNetGamePlayer*> g_playersByNetId;
static std::unordered_map<CNetGamePlayer*, uint16_t> g_netIdsByPlayer;

static CNetGamePlayer* g_playerList[256];
static int g_playerListCount;

static CNetGamePlayer*(*g_origGetPlayerByIndex)(int);

static CNetGamePlayer* GetPlayerByIndex(int index)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
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

	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
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
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origGetPlayerByIndexNet(index);
	}

	// todo: check network game flag
	return GetPlayerByIndex(index);
}

static bool(*g_origIsNetworkPlayerActive)(int);

static bool IsNetworkPlayerActive(int index)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origIsNetworkPlayerActive(index);
	}

	return (GetPlayerByIndex(index) != nullptr);
}

static bool(*g_origIsNetworkPlayerConnected)(int);

static bool IsNetworkPlayerConnected(int index)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origIsNetworkPlayerConnected(index);
	}

	return (GetPlayerByIndex(index) != nullptr);
}

//int g_physIdx = 1;
int g_physIdx = 42;

struct ScInAddr
{
	uint64_t unkKey1;
	uint64_t unkKey2;
	uint32_t secKeyTime; // added in 393
	uint32_t ipLan;
	uint16_t portLan;
	uint32_t ipUnk;
	uint16_t portUnk;
	uint32_t ipOnline;
	uint16_t portOnline;
	uint16_t pad3;
	uint32_t newVal; // added in 372
	uint64_t rockstarAccountId; // 463/505
};

namespace sync
{
	void TempHackMakePhysicalPlayer(uint16_t clientId, int idx = -1)
	{
		void* fakeInAddr = calloc(256, 1);
		void* fakeFakeData = calloc(256, 1);

		ScInAddr* inAddr = (ScInAddr*)fakeInAddr;
		inAddr->ipLan = clientId ^ 0xFEED;
		inAddr->ipUnk = clientId ^ 0xFEED;
		inAddr->ipOnline = clientId ^ 0xFEED;

		// 1290
		// 1365
		void* nonphys = calloc(256, 1);
		((void(*)(void*))0x141076C58)(nonphys); // ctor

		void* phys = calloc(1024, 1);
		((void(*)(void*))0x1410750F8)(phys);

		auto player = g_playerMgr->AddPlayer(fakeInAddr, fakeFakeData, nullptr, phys, nonphys);
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

void HandleCliehtDrop(const NetLibraryClientInfo& info)
{
	if (info.netId != g_netLibrary->GetServerNetID())
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
		uint16_t objectId;
		auto ped = ((void*(*)(void*, uint16_t*, CNetGamePlayer*))0x140FF4D34)(nullptr, &objectId, player);

		trace("reassigning ped: %016llx %d\n", (uintptr_t)ped, objectId);

		if (ped)
		{
			TheClones->DeleteObjectId(objectId);

			trace("deleted object id\n");

			((void(*)(void*, uint16_t, CNetGamePlayer*))0x140FDB078)(ped, objectId, player);

			trace("success! reassigned the ped!\n");
		}

		// TEMP: properly handle order so that we don't have to fake out the game
		g_playersByNetId[info.netId] = reinterpret_cast<CNetGamePlayer*>(g_tempRemotePlayer);
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

		g_players[info.slotId] = nullptr;
	}
}

static CNetGamePlayer*(*g_origGetOwnerNetPlayer)(rage::netObject*);

static CNetGamePlayer* netObject__GetOwnerNetPlayer(rage::netObject* object)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origGetOwnerNetPlayer(object);
	}

	if (object->syncData.ownerId == 31)
	{
		auto player = g_playersByNetId[TheClones->GetClientId(object)];

		// FIXME: figure out why bad playerinfos occur
		if (player->playerInfo != nullptr)
		{
			return player;
		}
	}

	return g_playerMgr->localPlayer;
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
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origAllocateNetPlayer(mgr);
	}

	void* plr = malloc(672);

	// 1103
	// 1290
	// 1365
	return ((CNetGamePlayer*(*)(void*))0x141074F78)(plr);
}

#include <minhook.h>

static void(*g_origPassObjectControl)(CNetGamePlayer* player, rage::netObject* netObject, int a3);

void ObjectIds_RemoveObjectId(int objectId);

static void PassObjectControlStub(CNetGamePlayer* player, rage::netObject* netObject, int a3)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origPassObjectControl(player, netObject, a3);
	}

	if (player->physicalPlayerIndex == netObject__GetOwnerNetPlayer(netObject)->physicalPlayerIndex)
	{
		return;
	}

	trace("passing object %016llx (%d) control from %d to %d\n", (uintptr_t)netObject, netObject->objectId, netObject->syncData.ownerId, player->physicalPlayerIndex);

	ObjectIds_RemoveObjectId(netObject->objectId);

	TheClones->GiveObjectToClient(netObject, g_netIdsByPlayer[player]);

	//auto lastIndex = player->physicalPlayerIndex;
	//player->physicalPlayerIndex = 31;

	g_origPassObjectControl(player, netObject, a3);

	//player->physicalPlayerIndex = lastIndex;
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
	if (Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return false;
	}

	return tree->m_D0(a2);
}

static int(*g_origGetNetworkPlayerListCount)();
static CNetGamePlayer**(*g_origGetNetworkPlayerList)();

static int GetNetworkPlayerListCount()
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origGetNetworkPlayerListCount();
	}

	return g_playerListCount;
}

static CNetGamePlayer** GetNetworkPlayerList()
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origGetNetworkPlayerList();
	}

	return g_playerList;
}

static HookFunction hookFunction([]()
{
	// temp dbg
	//hook::put<uint16_t>(hook::get_pattern("0F 84 80 00 00 00 49 8B 07 49 8B CF FF 50 20"), 0xE990);

	//hook::put(0x141980A68, NetLogStub_DoLog);
	//hook::put(0x141980A50, NetLogStub_DoTrace);
	//hook::jump(0x140A19640, NetLogStub_DoLog);

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("4C 8B F1 41 BD 05", -0x22), PassObjectControlStub, (void**)&g_origPassObjectControl);

	// return to disable breaking hooks
	//return;

	{
		auto location = hook::get_pattern("44 89 BE B4 00 00 00 FF 90", 7);
		hook::nop(location, 6);
		hook::call(location, m158Stub);
	}

	{
		auto location = hook::get_pattern("48 8B CF FF 90 68 01 00 00 84 C0 74 12 48 8B CF", 3);
		hook::nop(location, 6);
		hook::call(location, m168Stub);
	}

	// 1290
	// 1365
	MH_CreateHook((void*)0x14107D370, AllocateNetPlayer, (void**)&g_origAllocateNetPlayer);

	MH_CreateHook(hook::get_pattern("8A 41 49 3C FF 74 17 3C 20 73 13 0F B6 C8"), netObject__GetOwnerNetPlayer, (void**)&g_origGetOwnerNetPlayer);

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
		MH_CreateHook(hook::get_call(location + 0), GetNetworkPlayerListCount, (void**)&g_origGetNetworkPlayerListCount);
		MH_CreateHook(hook::get_call(location + 8), GetNetworkPlayerList, (void**)&g_origGetNetworkPlayerList);
	}

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
			if (Instance<ICoreGameInit>::Get()->OneSyncEnabled)
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

		virtual bool IsApplicableToPlayer(CNetGamePlayer* player) = 0;

		virtual void m_18() = 0;

		virtual void m_20() = 0;

		virtual void WriteToBuffer(rage::netBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void ReadFromBuffer(rage::netBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual bool Execute(CNetGamePlayer* sourcePlayer, void* connUnk) = 0;

		virtual void WriteReply(rage::netBuffer* buffer, CNetGamePlayer* replyPlayer) = 0;

		virtual void ApplyReply(rage::netBuffer* buffer, CNetGamePlayer* sourcePlayer) = 0;

		virtual void WritePostData(rage::netBuffer* buffer, bool isReply, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void ApplyPostData(rage::netBuffer* buffer, bool isReply, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void m_60() = 0;

		virtual void m_68() = 0;

		virtual void m_70() = 0;

		virtual void m_78() = 0;

		virtual bool Equals(const netGameEvent* event) = 0;

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
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origAddEvent(eventMgr, ev);
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
	rage::netBuffer rlBuffer(packetStub, sizeof(packetStub));

	ev->WriteToBuffer(&rlBuffer, g_player31, nullptr);
	ev->WritePostData(&rlBuffer, false, g_player31, nullptr);

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
				player->physicalPlayerIndex = 31;

				if (ev->IsApplicableToPlayer(player))
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

static void EventManager_Update()
{
	for (auto& eventPair : g_events)
	{
		auto [ev, time] = eventPair.second;

		if (ev)
		{
			auto expiryDuration = (ev->requiresReply) ? 3s : 200ms;

			if ((msec() - time) > expiryDuration)
			{
				delete ev;

				eventPair.second = { };
			}
		}
	}
}

static void HandleNetGameEvent(const char* idata, size_t len)
{
	if (!Instance<ICoreGameInit>::Get()->HasVariable("networkInited"))
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

	rage::netBuffer rlBuffer(const_cast<uint8_t*>(data.data()), data.size());
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
			ev->ApplyReply(&rlBuffer, player);
			ev->ApplyPostData(&rlBuffer, true, player, g_playerMgr->localPlayer);

			delete ev;
			g_events[eventHeader] = {};
		}
	}
	else
	{
		using TEventHandlerFn = void(*)(rage::netBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t, uint32_t, uint32_t);

		// for all intents and purposes, the player will be 31
		auto lastIndex = player->physicalPlayerIndex;
		player->physicalPlayerIndex = 31;

		auto eventHandlerList = (TEventHandlerFn*)((*(char**)g_netEventMgr) + 0x3AB80);
		eventHandlerList[eventType](&rlBuffer, player, g_playerMgr->localPlayer, eventHeader, 0, 0);

		player->physicalPlayerIndex = lastIndex;
	}
}

static void(*g_origExecuteNetGameEvent)(void* eventMgr, rage::netGameEvent* ev, rage::netBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t evH, uint32_t, uint32_t);

static void ExecuteNetGameEvent(void* eventMgr, rage::netGameEvent* ev, rage::netBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t evH, uint32_t a, uint32_t b)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origExecuteNetGameEvent(eventMgr, ev, buffer, player, unkConn, evH, a, b);
	}

	//trace("executing a %s\n", ev->GetName());

	ev->ReadFromBuffer(buffer, player, unkConn);

	// missing: some checks
	if (ev->Execute(player, unkConn))
	{
		ev->ApplyPostData(buffer, false, player, unkConn);

		if (ev->requiresReply)
		{
			uint8_t packetStub[1024];
			rage::netBuffer rlBuffer(packetStub, sizeof(packetStub));

			ev->WriteReply(&rlBuffer, player);
			ev->WritePostData(&rlBuffer, true, player, nullptr);

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
		netLibrary->OnClientInfoReceived.Connect([](const NetLibraryClientInfo& info)
		{
			if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
			{
				return;
			}

			HandleClientInfo(info);
		});

		netLibrary->OnClientInfoDropped.Connect([](const NetLibraryClientInfo& info)
		{
			if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
			{
				return;
			}

			HandleCliehtDrop(info);
		});

		netLibrary->AddReliableHandler("msgNetGameEvent", [](const char* data, size_t len)
		{
			if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
			{
				return;
			}

			HandleNetGameEvent(data, len);
		});
	});
});

static bool(*g_origSendGameEvent)(void*, void*);

static bool SendGameEvent(void* eventMgr, void* ev)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origSendGameEvent(eventMgr, ev);
	}

	return 1;
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

	MH_EnableHook(MH_ALL_HOOKS);
});

#include <nutsnbolts.h>
#include <GameInit.h>

static char(*g_origWriteDataNode)(void* node, uint32_t flags, void* mA0, rage::netObject* object, rage::netBuffer* buffer, int time, void* playerObj, char playerId, void* unk);


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

static char(*g_origReadDataNode)(void* node, uint32_t flags, void* mA0, rage::netBuffer* buffer, rage::netObject* object);

std::map<int, std::map<void*, std::tuple<int, uint32_t>>> g_netObjectNodeMapping;

static bool ReadDataNodeStub(void* node, uint32_t flags, void* mA0, rage::netBuffer* buffer, rage::netObject* object)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return g_origReadDataNode(node, flags, mA0, buffer, object);
	}

	/*if (flags == 1 || flags == 2)
	{
		uint32_t in;
		buffer->ReadInteger(&in, 8);
		assert(in == 0x5A);
	}*/

	bool didRead = g_origReadDataNode(node, flags, mA0, buffer, object);

	if (didRead && g_curNetObject)
	{
		g_netObjectNodeMapping[g_curNetObject->objectId][node] = { 0, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() };
	}
}

static bool WriteDataNodeStub(void* node, uint32_t flags, void* mA0, rage::netObject* object, rage::netBuffer* buffer, int time, void* playerObj, char playerId, void* unk)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
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
		buffer->WriteInteger(0, 11);

		bool rv = g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk);

		// write the actual length on top of the position
		uint32_t endPosition = buffer->GetPosition();
		auto length = endPosition - position - 11 - 1;

		if (length > 1 || flags == 1)
		{
			buffer->Seek(position);

			buffer->WriteBit(true);
			buffer->WriteInteger(length, 11);
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
	//if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		g_origUpdateSyncDataOn108(node, object);
	}
}

static void(*g_origCallSkip)(void* a1, void* a2, void* a3, void* a4, void* a5);

static void SkipCopyIf1s(void* a1, void* a2, void* a3, void* a4, void* a5)
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
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

		MH_EnableHook(MH_ALL_HOOKS);
	}
});

class netTimeSync
{
public:
	void Update();

	void HandleTimeSync(net::Buffer& buffer);

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
	uint32_t m_appliedDelta; // 128
	uint8_t m_applyFlags; // 132
	uint8_t m_disabled; // 133
};

#include <mmsystem.h>

void netTimeSync::Update()
{
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		return;
	}

	if (m_connectionMgr && /*m_flags & 2 && */!m_disabled)
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

			if (!(m_applyFlags & 1))
			{
				m_appliedDelta = m_timeDelta + timeGetTime();
			}

			m_applyFlags |= 3;
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

static HookFunction hookFunctionTime([]()
{
	/*MH_Initialize();
	
	MH_EnableHook(MH_ALL_HOOKS);*/

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
	if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
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

static HookFunction hookFunctionWorldGrid([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("44 8A 40 2D 41", -0x1B), DoesLocalPlayerOwnWorldGrid, (void**)&g_origDoesLocalPlayerOwnWorldGrid);
	MH_EnableHook(MH_ALL_HOOKS);

	// if population breaks in non-1s, this is possibly why
	hook::nop(hook::get_pattern("38 05 ? ? ? ? 75 0A 48 8B CF E8", 6), 2);
});

int ObjectToEntity(int objectId)
{
	int playerIdx = (objectId >> 16) - 1;
	int objectIdx = (objectId & 0xFFFF);

	int entityIdx = -1;

	rage::netObjectMgr::GetInstance()->ForAllNetObjects(playerIdx, [&](rage::netObject* obj)
	{
		char* objectChar = (char*)obj;
		uint16_t thisObjectId = *(uint16_t*)(objectChar + 10);

		if (objectIdx == thisObjectId)
		{
			entityIdx = getScriptGuidForEntity(obj->GetGameObject());
		}
	});

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
		if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
		{
			return;
		}

		EventManager_Update();
		TheClones->Update();
	});

	OnKillNetworkDone.Connect([]()
	{
		trackedObjects.clear();
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
		rage::netObjectMgr::GetInstance()->RegisterObject(obj);

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
		rage::netObjectMgr::GetInstance()->RegisterObject(obj);

		netBlender_SetTimestamp(obj->GetBlender(), g_queryFunctions->GetTimestamp());

		obj->m_1D0();

		obj->GetBlender()->ApplyBlend();
		obj->GetBlender()->m_38();

		obj->m_1C0();

		trace("got game object %llx\n", (uintptr_t)obj->GetGameObject());
	});
#endif
});
