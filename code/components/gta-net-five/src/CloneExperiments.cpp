#include <StdInc.h>
#include <CoreConsole.h>
#include <jitasm.h>
#include <Hooking.h>
#include <ScriptEngine.h>

#include <netBlender.h>
#include <netInterface.h>
#include <netGameEvent.h>
#include <netObject.h>
#include <netObjectMgr.h>
#include <netPlayerManager.h>
#include <netSyncTree.h>
#include <rlNetBuffer.h>

#include <CloneManager.h>

#include <CrossBuildRuntime.h>
#include <ICoreGameInit.h>

#include <base64.h>

#include <NetLibrary.h>
#include <NetBuffer.h>

#include <Pool.h>

#include <EntitySystem.h>
#include <sysAllocator.h>

#include <ResourceManager.h>
#include <StateBagComponent.h>
#include <atArray.h>

#include <Error.h>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "NetBitVersion.h"
#include "WorldGridPacketHandler.h"

extern NetLibrary* g_netLibrary;

class CNetGamePlayer;

ICoreGameInit* icgi;

namespace rage
{
	class netObject;
}

void* g_tempRemotePlayer;

static CNetGamePlayer* g_playerList[256];
static int g_playerListCount;

static CNetGamePlayer* g_playerListRemote[256];
static int g_playerListCountRemote;

static std::unordered_map<uint16_t, std::shared_ptr<fx::StateBag>> g_playerBags;

static CNetGamePlayer*(*g_origGetPlayerByIndex)(uint8_t);

static CNetGamePlayer* __fastcall GetPlayerByIndex(uint8_t index)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPlayerByIndex(index);
	}

	if (index < 0 || index >= 256)
	{
		return nullptr;
	}

	return g_players[index];
}

static void SetupLocalPlayer(CNetGamePlayer* player)
{
	if (player != rage::GetLocalPlayer())
	{
		return;
	}

	console::DPrintf("onesync", "Assigning physical player index for the local player.\n");

	auto clientId = g_netLibrary->GetServerNetID();
	auto idx = g_netLibrary->GetServerSlotID();

	player->physicalPlayerIndex() = idx;

	g_players[idx] = player;

	g_playersByNetId[clientId] = player;
	g_netIdsByPlayer[player] = clientId;

	// add to sequential list
	g_playerList[g_playerListCount] = player;
	g_playerListCount++;

	g_playerBags[clientId] = Instance<fx::ResourceManager>::Get()
							 ->GetComponent<fx::StateBagComponent>()
							 ->RegisterStateBag(fmt::sprintf("player:%d", clientId), true);

	// don't add to g_playerListRemote(!)
}

#ifdef GTA_FIVE
static void(*g_origJoinBubble)(void* bubbleMgr, CNetGamePlayer* player);

static void JoinPhysicalPlayerOnHost(void* bubbleMgr, CNetGamePlayer* player)
{
	g_origJoinBubble(bubbleMgr, player);

	if (icgi->OneSyncEnabled)
	{
		SetupLocalPlayer(player);
	}
}
#elif IS_RDR3
static void*(*g_origJoinBubble)(void* bubbleMgr, void* scSessionImpl, void* playerDataMsg, void* a4, uint8_t slotIndex);

static void* JoinPhysicalPlayerOnHost(void* bubbleMgr, void* scSessionImpl, void* playerDataMsg, void* a4, uint8_t slotIndex)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origJoinBubble(bubbleMgr, scSessionImpl, playerDataMsg, a4, slotIndex);
	}

	auto result = g_origJoinBubble(bubbleMgr, scSessionImpl, playerDataMsg, a4, g_netLibrary->GetServerSlotID());
	SetupLocalPlayer(rage::GetLocalPlayer());
	return result;
}
#endif

CNetGamePlayer* GetPlayerByNetId(uint16_t netId)
{
	return g_playersByNetId[netId];
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

static bool (*g_origNetPlayer_IsActive)(CNetGamePlayer*);

static bool netPlayer_IsActiveStub(CNetGamePlayer* player)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origNetPlayer_IsActive(player);
	}

	return true;
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

#ifdef GTA_FIVE
int g_physIdx = 42;
#elif IS_RDR3
int g_physIdx = 1;
#endif

#ifdef GTA_FIVE
static hook::cdecl_stub<void(void*)> _npCtor([]()
{
	return hook::get_pattern("C7 41 08 0A 00 00 00 C7 41 0C 20 00 00 00", -0xA);
});
#endif

static hook::cdecl_stub<void(void*)> _pCtor([]()
{
#ifdef GTA_FIVE
	return hook::get_address<void*>(hook::get_pattern("41 8B E9 4D 8B F0 48 8B DA E8", 9), 1, 5);
#elif IS_RDR3
	return hook::get_pattern("83 4B 1C FF 48 8D 05 ? ? ? ? 33 C9 48", -0xE);
#endif
});

#ifdef IS_RDR3
static hook::cdecl_stub<void(uint8_t)> _addCachedPlayerArrayEntry([]()
{
	return hook::get_call(hook::get_pattern("48 8B CB E8 ? ? ? ? 8A 4B 19 48 83 C4", 0x10));
});

static hook::cdecl_stub<void(uint8_t)> _removeCachedPlayerArrayEntry([]()
{
	return hook::get_call(hook::get_pattern("40 8A CD E8 ? ? ? ? 48 8B 6C 24 38", 3));
});
#endif

namespace sync
{
	static void SetupRemotePlayer(uint16_t clientId, CNetGamePlayer* player, int idx)
	{
		player->physicalPlayerIndex() = idx;
		g_players[idx] = player;

		g_playersByNetId[clientId] = player;
		g_netIdsByPlayer[player] = clientId;

		g_playerList[g_playerListCount] = player;
		g_playerListCount++;

		g_playerListRemote[g_playerListCountRemote] = player;
		g_playerListCountRemote++;

		// resort player lists
		std::sort(g_playerList, g_playerList + g_playerListCount, [](CNetGamePlayer* left, CNetGamePlayer* right)
		{
			return (left->physicalPlayerIndex() < right->physicalPlayerIndex());
		});

		std::sort(g_playerListRemote, g_playerListRemote + g_playerListCountRemote, [](CNetGamePlayer* left, CNetGamePlayer* right)
		{
			return (left->physicalPlayerIndex() < right->physicalPlayerIndex());
		});

		g_playerBags[clientId] = Instance<fx::ResourceManager>::Get()
								 ->GetComponent<fx::StateBagComponent>()
								 ->RegisterStateBag(fmt::sprintf("player:%d", clientId), true);
	}

#ifdef GTA_FIVE
	template<int Build>
	void TempHackMakePhysicalPlayerImpl(uint16_t clientId, int idx = -1)
	{
		auto npPool = rage::GetPoolBase("CNonPhysicalPlayerData");

		// probably shutting down the network subsystem
		if (!npPool)
		{
			return;
		}

		void* fakeInAddr = calloc(256, 1);
		void* fakeFakeData = calloc(256, 1);

		rlGamerInfo<Build>* inAddr = (rlGamerInfo<Build>*)fakeInAddr;
		inAddr->peerAddress.localAddr().ip.addr = clientId ^ 0xFEED;
		inAddr->peerAddress.relayAddr().ip.addr = clientId ^ 0xFEED;
		inAddr->peerAddress.publicAddr().ip.addr = clientId ^ 0xFEED;
		inAddr->peerAddress.rockstarAccountId() = clientId;
		inAddr->gamerId = clientId;

		// this has to come from the pool directly as the game will expect to free it
		void* nonPhys = rage::PoolAllocate(npPool);
		_npCtor(nonPhys); // ctor

		void* phys = calloc(1024, 1);
		_pCtor(phys);

		auto player = g_playerMgr->AddPlayer(fakeInAddr, fakeFakeData, nullptr, phys, nonPhys);
		g_tempRemotePlayer = player;

		// so we can safely remove (do this *before* assigning physical player index, or the game will add
		// to a lot of lists which aren't >32-safe)
		//g_playerMgr->UpdatePlayerListsForPlayer(player);
		// NOTE: THIS IS NOT SAFE unless array manager/etc. are patched

		if (idx == -1)
		{
			idx = g_physIdx;
			g_physIdx++;
		}

		SetupRemotePlayer(clientId, player, idx);
	}

	void TempHackMakePhysicalPlayer(uint16_t clientId, int idx = -1)
	{
		if (xbr::IsGameBuildOrGreater<2824>())
		{
			TempHackMakePhysicalPlayerImpl<2824>(clientId, idx);
		}
		else if (xbr::IsGameBuildOrGreater<2372>())
		{
			TempHackMakePhysicalPlayerImpl<2372>(clientId, idx);
		}
		else if (xbr::IsGameBuildOrGreater<2060>())
		{
			TempHackMakePhysicalPlayerImpl<2060>(clientId, idx);
		}
		else
		{
			TempHackMakePhysicalPlayerImpl<1604>(clientId, idx);
		}
	}
#elif IS_RDR3
	void TempHackMakePhysicalPlayer(uint16_t clientId, int idx = -1)
	{
		void* fakeInAddr = calloc(256, 1);
		void* fakeFakeData = calloc(256, 1);

		rlGamerInfo* inAddr = (rlGamerInfo*)fakeInAddr;
		inAddr->peerAddress.localAddr.ip.addr = (clientId ^ 0xFEED) | 0xc0a80000;
		inAddr->peerAddress.relayAddr.ip.addr = clientId ^ 0xFEED;
		inAddr->peerAddress.publicAddr.ip.addr = clientId ^ 0xFEED;
		inAddr->peerAddress.rockstarAccountId = clientId;

		void* phys = calloc(1024, 1);
		_pCtor(phys);

		auto player = g_playerMgr->AddPlayer(fakeInAddr, 0, phys, &fakeFakeData);
		g_tempRemotePlayer = player;

		if (idx == -1)
		{
			idx = g_physIdx;
			g_physIdx++;
		}

		_addCachedPlayerArrayEntry(idx);
		SetupRemotePlayer(clientId, player, idx);
	}
#endif
}

void HandleClientInfo(const NetLibraryClientInfo& info)
{
	if (info.netId != g_netLibrary->GetServerNetID())
	{
		console::DPrintf("onesync", "Creating physical player %d (%s)\n", info.slotId, info.name);

		sync::TempHackMakePhysicalPlayer(info.netId, info.slotId);
	}
}

static hook::cdecl_stub<void* (CNetGamePlayer*)> getPlayerPedForNetPlayer([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("84 C0 74 1C 48 8B CF E8 ? ? ? ? 48 8B D8", 7));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("48 8B CD 0F 11 06 48 8B D8 E8", -8));
#endif
});

#ifdef GTA_FIVE
static constexpr uint32_t g_entityNetObjOffset = 208;
#elif IS_RDR3
static constexpr uint32_t g_entityNetObjOffset = 224;
#endif

rage::netObject* GetNetObjectFromEntity(void* entity)
{
	// technically must be at least CDynamicEntity
	return *(rage::netObject**)((char*)entity + g_entityNetObjOffset);
}

rage::netObject* GetLocalPlayerPedNetObject()
{
	auto ped = getPlayerPedForNetPlayer(rage::GetLocalPlayer());

	if (ped)
	{
		return GetNetObjectFromEntity(ped);
	}

	return nullptr;
}

void HandleClientDrop(const NetLibraryClientInfo& info)
{
	if (info.netId != g_netLibrary->GetServerNetID() && info.slotId != g_netLibrary->GetServerSlotID())
	{
		console::DPrintf("onesync", "Processing removal for player %d (%s)\n", info.slotId, info.name);

		if (info.slotId < 0 || info.slotId >= _countof(g_players))
		{
			console::DPrintf("onesync", "That's not a valid slot! Aborting.\n");
			return;
		}

		auto player = g_players[info.slotId];

		if (!player)
		{
			console::DPrintf("onesync", "That slot has no player. Aborting.\n");
			return;
		}

		// reassign the player's ped
		// TODO: only do this on a single client(!)

		// 1604 unused
		uint16_t objectId = 0;
		//auto ped = ((void*(*)(void*, uint16_t*, CNetGamePlayer*))hook::get_adjusted(0x141022B20))(nullptr, &objectId, player);
		auto ped = getPlayerPedForNetPlayer(player);

		// reset player (but not until we have gotten the ped)
		player->Reset();

		if (ped)
		{
			auto netObj = GetNetObjectFromEntity(ped);

			if (netObj)
			{
				objectId = netObj->GetObjectId();
			}
		}

		console::DPrintf("onesync", "reassigning ped: %016llx %d\n", (uintptr_t)ped, objectId);

		if (ped)
		{
			// prevent stack overflow
			if (!info.name.empty())
			{
				TheClones->DeleteObjectId(objectId, 0, true);
			}

			console::DPrintf("onesync", "deleted object id\n");

			// 1604 unused
			//((void(*)(void*, uint16_t, CNetGamePlayer*))hook::get_adjusted(0x141008D14))(ped, objectId, player);

			console::DPrintf("onesync", "success! reassigned the ped!\n");
		}

		// make non-physical so we will remove from the non-physical list only
		/*auto physIdx = player->physicalPlayerIndex();
		player->physicalPlayerIndex() = -1;

		// remove object manager pointer temporarily
		static auto objectMgr = hook::get_address<void**>(hook::get_pattern("48 8B FA 0F B7 51 30 48 8B 0D ? ? ? ? 45 33 C0", 10));
		auto ogMgr = *objectMgr;
		*objectMgr = nullptr;

		// remove
		g_playerMgr->RemovePlayer(player);

		// restore
		*objectMgr = ogMgr;
		player->physicalPlayerIndex() = physIdx;*/
		// ^ is NOT safe, array handler manager etc. have 32 limits

		// TEMP: properly handle order so that we don't have to fake out the game
		g_playersByNetId[info.netId] = nullptr;
		g_netIdsByPlayer[player] = -1;

#ifdef IS_RDR3
		_removeCachedPlayerArrayEntry(info.slotId);
#endif

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
			return (left->physicalPlayerIndex() < right->physicalPlayerIndex());
		});

		std::sort(g_playerListRemote, g_playerListRemote + g_playerListCountRemote, [](CNetGamePlayer* left, CNetGamePlayer* right)
		{
			return (left->physicalPlayerIndex() < right->physicalPlayerIndex());
		});

		g_players[info.slotId] = nullptr;
		g_playerBags.erase(info.netId);
	}
}

static CNetGamePlayer*(*g_origGetOwnerNetPlayer)(rage::netObject*);

CNetGamePlayer* netObject__GetPlayerOwner(rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetOwnerNetPlayer(object);
	}

	if (object && object->syncData.ownerId == 31)
	{
		auto player = g_playersByNetId[TheClones->GetClientId(object)];

		// FIXME: figure out why bad playerinfos occur
		if (player != nullptr && player->GetPlayerInfo() != nullptr)
		{
			return player;
		}

#ifdef IS_RDR3
		return nullptr;
#endif

		rage::EnsurePlayer31();
		return rage::GetPlayer31();
	}

	return rage::GetLocalPlayer();
}

static uint8_t(*g_origGetOwnerPlayerId)(rage::netObject*);

static uint8_t netObject__GetPlayerOwnerId(rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetOwnerPlayerId(object);
	}

	auto owner = netObject__GetPlayerOwner(object);

	return owner ? owner->physicalPlayerIndex() : 0xFF;
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

		if (player != nullptr && player->GetPlayerInfo() != nullptr)
		{
			return player;
		}
	}

	return nullptr;
}

static hook::cdecl_stub<uint32_t(void*)> getScriptGuidForEntity([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
#elif IS_RDR3
	return hook::get_pattern("32 DB E8 ? ? ? ? 48 85 C0 75 ? 8A 05", -35);
#endif
});

#ifdef GTA_FIVE
struct ReturnedCallStub : public jitasm::Frontend
{
	ReturnedCallStub(int idx, uintptr_t targetFunc)
		: m_index(idx), m_targetFunc(targetFunc)
	{

	}

	static void InstrumentedTarget(void* targetFn, void* callFn, int index)
	{
		console::DPrintf("onesync", "called %016llx (m_%x) from %016llx\n", (uintptr_t)targetFn, index, (uintptr_t)callFn);
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

	console::DPrintf("onesync", "[NET] %s\n", buffer);
}

static void NetLogStub_DoLog(void*, const char* type, const char* fmt, ...)
{
	char buffer[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	console::DPrintf("onesync", "[NET] %s %s\n", type, buffer);
}
#endif

#include <minhook.h>

static void(*g_origPassObjectControl)(CNetGamePlayer* player, rage::netObject* netObject, int a3);

void ObjectIds_RemoveObjectId(int objectId);

static void PassObjectControlStub(CNetGamePlayer* player, rage::netObject* netObject, int a3)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origPassObjectControl(player, netObject, a3);
	}

	auto owner = netObject__GetPlayerOwner(netObject);

	if (!owner || player->physicalPlayerIndex() == owner->physicalPlayerIndex())
	{
		return;
	}

	console::DPrintf("onesync", "passing object %016llx (%d) control from %d to %d\n", (uintptr_t)netObject, netObject->GetObjectId(), netObject->syncData.ownerId, player->physicalPlayerIndex());
	TheClones->Log("%s: passing object %016llx (%d) control from %d to %d\n", __func__, (uintptr_t)netObject, netObject->GetObjectId(), netObject->syncData.ownerId, player->physicalPlayerIndex());

	ObjectIds_RemoveObjectId(netObject->GetObjectId());

	netObject->syncData.nextOwnerId = 31;
	TheClones->SetTargetOwner(netObject, g_netIdsByPlayer[player]);

	// REDM1S: implement for vehicles and mounts
#ifdef GTA_FIVE
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
					if (!netOccupant->syncData.isRemote && netOccupant->GetObjectType() != 11)
					{
						console::DPrintf("onesync", "passing occupant %d control as well\n", netOccupant->GetObjectId());

						PassObjectControlStub(player, netOccupant, a3);
					}
				}
			}
		}
	}
#endif

	//auto lastIndex = player->physicalPlayerIndex();
	//player->physicalPlayerIndex() = 31;

	g_origPassObjectControl(player, netObject, a3);

	//player->physicalPlayerIndex() = lastIndex;
}

static void(*g_origSetOwner)(rage::netObject* object, CNetGamePlayer* newOwner);

static void SetOwnerStub(rage::netObject* netObject, CNetGamePlayer* newOwner)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSetOwner(netObject, newOwner);
	}

	if (newOwner->physicalPlayerIndex() == rage::GetLocalPlayer()->physicalPlayerIndex())
	{
		TheClones->Log("%s: taking ownership of object id %d - stack trace:\n", __func__, netObject->GetObjectId());

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

	auto oldOwner = netObject__GetPlayerOwner(netObject);
	auto oldIndex = (oldOwner) ? oldOwner->physicalPlayerIndex() : 31;

	if (newOwner->physicalPlayerIndex() == oldIndex)
	{
		return;
	}

	TheClones->Log("%s: passing object %016llx (%d) ownership from %d to %d\n", __func__, (uintptr_t)netObject, netObject->GetObjectId(), netObject->syncData.ownerId, newOwner->physicalPlayerIndex());

	ObjectIds_RemoveObjectId(netObject->GetObjectId());

	netObject->syncData.nextOwnerId = 31;
	TheClones->SetTargetOwner(netObject, g_netIdsByPlayer[newOwner]);
}

static bool netObject__CanPassControl(rage::netObject* object, CNetGamePlayer* player, int type, int* outReason)
{
	int reason;
	bool rv = object->CanPassControl(player, type, &reason);

	if (!rv)
	{
		console::DPrintf("onesync", "couldn't pass control for reason %d\n", reason);
	}

	return rv;
}

#ifdef GTA_FIVE
static bool netObject__CanBlend(rage::netObject* object, int* outReason)
{
	int reason;
	bool rv = object->CanBlend(&reason);

	if (!rv)
	{
		//trace("couldn't blend object for reason %d\n", reason);
	}

	return rv;
}
#endif

int getPlayerId()
{
	return rage::GetLocalPlayer()->physicalPlayerIndex();
}

static bool mD0Stub(rage::netSyncTree* tree, int a2)
{
	if (icgi->OneSyncEnabled)
	{
		return false;
	}

#ifdef GTA_FIVE
	return tree->m_D0(a2);
#elif IS_RDR3
	return tree->m_C0(a2);
#endif
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

#ifdef GTA_FIVE
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
#endif

static int(*g_origNetworkBandwidthMgr_CalculatePlayerUpdateLevels)(void* mgr, int* a2, int* a3, int* a4);

static int networkBandwidthMgr_CalculatePlayerUpdateLevelsStub(void* mgr, int* a2, int* a3, int* a4)
{
	if (icgi->OneSyncEnabled)
	{
		return 0;
	}

	return g_origNetworkBandwidthMgr_CalculatePlayerUpdateLevels(mgr, a2, a3, a4);
}

#ifdef GTA_FIVE
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
#endif

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

// REDM1S: haven't check if code valid for RDR3
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
				if (object->GetObjectType() == (uint16_t)NetObjEntityType::Player && !object->syncData.isRemote)
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

// REDM1S: haven't tested in RDR3
static void(*g_origPlayerManager_End)(void*);

void PlayerManager_End(void* mgr)
{
	if (icgi->OneSyncEnabled)
	{
		g_netIdsByPlayer.clear();
		g_playersByNetId.clear();
		g_playerBags.clear();

		for (auto& p : g_players)
		{
			if (p)
			{
				if (p != rage::GetLocalPlayer())
				{
					console::DPrintf("onesync", "player manager shutdown: resetting player %s\n", p->GetName());
					p->Reset();
				}
			}

			p = nullptr;
		}

		// reset the player list so we won't try removing _any_ players
		// #TODO1S: don't corrupt the physical linked list in the first place
#ifdef GTA_FIVE
		const int offset = xbr::IsGameBuildOrGreater<2944>() ? 8 : 0;
#else
		const int offset = 0;
#endif
		*(void**)((char*)g_playerMgr + 288 + offset) = nullptr;
		*(void**)((char*)g_playerMgr + 296 + offset) = nullptr;
		*(uint32_t*)((char*)g_playerMgr + 304 + offset) = 0;

		*(void**)((char*)g_playerMgr + 312 + offset) = nullptr;
		*(void**)((char*)g_playerMgr + 320 + offset) = nullptr;
		*(uint32_t*)((char*)g_playerMgr + 328 + offset) = 0;

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

#ifdef GTA_FIVE
template<int Build>
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
			if (p->GetGamerInfo<Build>()->peerAddress.rockstarAccountId() == gamerId.accountId)
			{
				return p;
			}
		}
	}

	return nullptr;
}
#elif IS_RDR3
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
#endif

static float VectorDistance(const float* point1, const float* point2)
{
	float xd = point1[0] - point2[0];
	float yd = point1[1] - point2[1];
	float zd = point1[2] - point2[2];

	return sqrtf((xd * xd) + (yd * yd) + (zd * zd));
}

#if GTA_FIVE
static hook::cdecl_stub<float*(float*, CNetGamePlayer*, void*, bool)> getNetPlayerRelevancePosition([]()
{
	// 1737: Arxan.
	if (xbr::IsGameBuildOrGreater<2060>())
	{
		return hook::get_call(hook::get_pattern("48 8D 4C 24 40 45 33 C9 45 33 C0 48 8B D0 E8", 0xE));
	}
	else
	{
		return hook::get_pattern("45 33 FF 48 85 C0 0F 84 5B 01 00 00", -0x34);
	}
});
#elif IS_RDR3
static float*(*g_origGetNetPlayerRelevancePosition)(float* position, CNetGamePlayer* player, void* unk);

static float* getNetPlayerRelevancePosition(float* position, CNetGamePlayer* player, void* unk)
{
	if (!icgi->OneSyncEnabled || !player || player->physicalPlayerIndex() != 31)
	{
		return g_origGetNetPlayerRelevancePosition(position, player, unk);
	}

	for (int i = 0; i < 256; i++)
	{
		if (g_players[i] == player)
		{
			player->physicalPlayerIndex() = i;
			break;
		}
	}

	auto result = g_origGetNetPlayerRelevancePosition(position, player, unk);

	player->physicalPlayerIndex() = 31;

	return result;
}
#endif

#ifdef GTA_FIVE
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
#elif IS_RDR3
static int(*g_origGetPlayersNearPoint)(const float* point, uint32_t unkIndex, void* outIndex, CNetGamePlayer* outArray[32], bool unkVal, float range, bool sorted);

static int GetPlayersNearPoint(const float* point, uint32_t unkIndex, void* outIndex, CNetGamePlayer* outArray[32], bool unkVal, float range, bool sorted)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPlayersNearPoint(point, unkIndex, outIndex, outArray, range, range, sorted);
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

			if (range >= 100000000.0f || VectorDistance(point, getNetPlayerRelevancePosition(vectorPos, player, nullptr)) < range)
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

			float d1 = VectorDistance(point, getNetPlayerRelevancePosition(vectorPos1, a1, nullptr));
			float d2 = VectorDistance(point, getNetPlayerRelevancePosition(vectorPos2, a2, nullptr));

			return (d1 < d2);
		});
	}

	idx = std::min(idx, 32);
	unkIndex = idx;

	std::copy(tempArray, tempArray + idx, outArray);

	return idx;
}
#endif

static void(*g_origVoiceChatMgr_EstimateBandwidth)(void*);

static void VoiceChatMgr_EstimateBandwidth(void* mgr)
{
	if (icgi->OneSyncEnabled)
	{
		return;
	}

	g_origVoiceChatMgr_EstimateBandwidth(mgr);
}

namespace sync
{
	extern net::Buffer g_cloneMsgPacket;
	extern std::vector<uint8_t> g_cloneMsgData;
}

#ifdef GTA_FIVE

static void (*g_origManageTextVoiceChatStub)(void*);

static void ManageTextVoiceChatStub(void* a1)
{
	if (!icgi->OneSyncEnabled)
	{
		g_origManageTextVoiceChatStub(a1);
	}
}
#endif

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

#ifdef GTA_FIVE
static void* (*g_origNetworkObjectMgrCtor)(void*, void*);

static void* NetworkObjectMgrCtorStub(void* mgr, void* bw)
{
	auto alloc = rage::GetAllocator();
	alloc->Free(mgr);

	int initialSize = (xbr::IsGameBuildOrGreater<3258>()) ? 32712 + 8192 : 32712;

	mgr = alloc->Allocate(initialSize + 4096, 16, 0);

	g_origNetworkObjectMgrCtor(mgr, bw);

	return mgr;
}
#elif IS_RDR3
static void* (*g_origNetworkObjectMgrCtor)(void*, void*, void*);

static void* NetworkObjectMgrCtorStub(void* mgr, void* bw, void* unk)
{
	auto alloc = rage::GetAllocator();
	alloc->Free(mgr);

	int initialSize = (xbr::IsGameBuildOrGreater<1355>()) ? 268672 : 163056;

	mgr = alloc->Allocate(initialSize + 4096, 16, 0);

	g_origNetworkObjectMgrCtor(mgr, bw, unk);

	return mgr;
}
#endif

#ifdef IS_RDR3
struct CScenarioInfo
{
	void* vtable;
	BYTE unk_8;
	char pad_9[3];
	uint32_t m_name;

	// etc...
};

struct CScenarioInfoManager
{
	void* vtable;
	atArray<CScenarioInfo*> m_scenarioInfos;
	atArray<CScenarioInfo*> m_scenarioUnks;

	// etc...
};

static CScenarioInfoManager** g_scenarioInfoManager;
static int g_pointIdGetterVtableOffset;
static void* g_taskUseScenarioVtable;

static CScenarioInfo* (*g_origGetTaskScenarioInfo)(void* task);

static CScenarioInfo* GetTaskScenarioInfo(void* task)
{
	if (!task)
	{
		return nullptr;
	}

	auto scenario = g_origGetTaskScenarioInfo(task);

	if (!scenario && *(uint64_t*)task == (uint64_t)g_taskUseScenarioVtable)
	{
		auto scenarioInfoMgr = *g_scenarioInfoManager;

#if _DEBUG
		auto pointId = (*(uint32_t(__fastcall**)(void*))(*(uint64_t*)task + g_pointIdGetterVtableOffset))(task);

		trace("Failed to get scenario info by id %d for task (address %p, vtable %p). Loaded %d and %d scenario infos.\n",
			pointId, (void*)hook::get_unadjusted(task), (void*)hook::get_unadjusted(*(void**)task),
			scenarioInfoMgr->m_scenarioInfos.GetCount(), scenarioInfoMgr->m_scenarioUnks.GetCount());
#endif

		// Try to avoid crash by returning first scenario from the array.
		return scenarioInfoMgr->m_scenarioInfos[0];
	}

	return scenario;
}

static constexpr int kPlayerNetIdLength = 16;

static hook::cdecl_stub<void(rage::CSyncDataBase*, uint8_t*, char*)> _syncDataBase_SerializePlayerIndex([]()
{
	return hook::get_call(hook::get_pattern("0F B6 12 48 8B 05 ? ? ? ? 44", 0x1D));
});

static void (*g_origSyncDataReader_SerializePlayerIndex)(rage::CSyncDataReader*, uint8_t*, char*);
static void SyncDataReader_SerializePlayerIndex(rage::CSyncDataReader* syncData, uint8_t* index, char* prefix) 
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataReader_SerializePlayerIndex(syncData, index, prefix);
	}

	uint32_t playerNetId = -1;
	uint8_t playerIndex = -1;

	if (syncData->m_buffer->ReadInteger(&playerNetId, kPlayerNetIdLength))
	{
		if (auto player = GetPlayerByNetId(playerNetId))
		{
			playerIndex = player->physicalPlayerIndex();
		}
	}

	*index = playerIndex;

	_syncDataBase_SerializePlayerIndex(syncData, index, prefix);
}

static void (*g_origSyncDataWriter_SerializePlayerIndex)(rage::CSyncDataWriter*, uint8_t*, char*);
static void SyncDataWriter_SerializePlayerIndex(rage::CSyncDataWriter * syncData, uint8_t * index, char* prefix)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataWriter_SerializePlayerIndex(syncData, index, prefix);
	}

	uint16_t playerNetId = -1;

	if (auto player = GetPlayerByIndex(*index))
	{
		playerNetId = g_netIdsByPlayer[player];
	}

	_syncDataBase_SerializePlayerIndex(syncData, index, prefix);

	syncData->m_buffer->WriteUns(playerNetId, kPlayerNetIdLength);
}

static void (*g_origSyncDataSizeCalculator_SerializePlayerIndex)(rage::CSyncDataSizeCalculator*);
static void SyncDataSizeCalculator_SerializePlayerIndex(rage::CSyncDataSizeCalculator* syncData)
{
	static_assert(offsetof(rage::CSyncDataSizeCalculator, m_size) == 0x18);

	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataSizeCalculator_SerializePlayerIndex(syncData);
	}

	syncData->m_size += kPlayerNetIdLength;
}

static void (*g_origSyncDataSizeCalculator_SerializeObjectId)(rage::CSyncDataSizeCalculator*);
static void SyncDataSizeCalculator_SerializeObjectId(rage::CSyncDataSizeCalculator* syncData)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataSizeCalculator_SerializeObjectId(syncData);
	}

	syncData->m_size += (icgi->OneSyncBigIdEnabled ? 16 : 13);
}

static hook::cdecl_stub<void(rage::CSyncDataBase*, uint16_t*, char*)> _syncDataBaseSerializeObjectId([]()
{
	return hook::get_call(hook::get_pattern("0F B7 03 B9 3F 1F 00 00 66 FF C8 66 3B C1 76 04", -5));
});

static void (*g_origSyncDataWriter_SerializeObjectIdPrefix)(rage::CSyncDataWriter*, uint16_t*, char*);
static void SyncDataWriter_SerializeObjectIdPrefix(rage::CSyncDataWriter* syncData, uint16_t* objectId, char* prefix)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataWriter_SerializeObjectIdPrefix(syncData, objectId, prefix);
	}

	_syncDataBaseSerializeObjectId(syncData, objectId, prefix);

	syncData->m_buffer->WriteUns(*objectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
}

static void (*g_origSyncDataReader_SerializeObjectIdPrefix)(rage::CSyncDataReader*, uint16_t*, char*);
static void SyncDataReader_SerializeObjectIdPrefix(rage::CSyncDataReader* syncData, uint16_t* objectId, char* prefix)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataReader_SerializeObjectIdPrefix(syncData, objectId, prefix);
	}

	uint32_t netObjectId = 0;

	if (syncData->m_buffer->ReadInteger(&netObjectId, icgi->OneSyncBigIdEnabled ? 16 : 13))
	{
		*objectId = netObjectId;
	}

	_syncDataBaseSerializeObjectId(syncData, objectId, prefix);
}

static void (*g_origSyncDataWriter_SerializeObjectIdStatic)(rage::CSyncDataWriter*, uint16_t*);
static void SyncDataWriter_SerializeObjectIdStatic(rage::CSyncDataWriter* syncData, uint16_t* objectId)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataWriter_SerializeObjectIdStatic(syncData, objectId);
	}

	syncData->m_buffer->WriteUns(*objectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
}

static void (*g_origSyncDataReader_SerializeObjectIdStatic)(rage::CSyncDataReader*, uint16_t*);
static void SyncDataReader_SerializeObjectIdStatic(rage::CSyncDataReader* syncData, uint16_t* objectId)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataReader_SerializeObjectIdStatic(syncData, objectId);
	}

	uint32_t netObjectId = 0;

	if (syncData->m_buffer->ReadInteger(&netObjectId, icgi->OneSyncBigIdEnabled ? 16 : 13))
	{
		*objectId = netObjectId;
	}
}

static void (*g_origRespawnPlayerPedEvent_Serialize)(void*, rage::CSyncDataBase*);

static uint32_t g_respawnPlayerPedEvent_objIdRefOffset;
static uint8_t g_respawnPlayerPedEvent_objIdOffset;

static void RespawnPlayerPedEvent_Serialize(void* self, rage::CSyncDataBase* syncData)
{
	g_origRespawnPlayerPedEvent_Serialize(self, syncData);

	if (!icgi->OneSyncEnabled)
	{
		return;
	}

	if (syncData->m_type == rage::SYNC_DATA_READER || syncData->m_type == rage::SYNC_DATA_WRITER)
	{
		auto objectIdRef = (uint16_t*)((char*)self + g_respawnPlayerPedEvent_objIdRefOffset);
		auto objectId = (uint16_t*)((char*)self + g_respawnPlayerPedEvent_objIdOffset);

		// Object ID Mapping isn't needed here.
		*objectId = *objectIdRef;
	}
}

struct CNetworkEventComponentControlBase
{
	void* vtable;
	uint16_t m_objectId1;
	uint16_t m_objectId2;
	uint8_t unk_C;
	uint8_t unk_D;
	uint8_t unk_E;
	uint8_t unk_F;
	uint8_t unk_10;
	char pad_11[7];
};

struct CNetworkEventVehComponentControl : CNetworkEventComponentControlBase
{
	uint16_t m_occupantObjectId;
	BYTE unk_1A;
	BYTE unk_1B;
	uint16_t unk_1D;
	BYTE unk_1E;
	char pad_1F[1];
};

namespace rage
{
	struct scriptIdBase
	{
		void* vtable;
	};

	struct scriptId : scriptIdBase
	{
		uint32_t m_scriptHash;
	};
}

struct CGameScriptId : rage::scriptId
{
	uint32_t unk_10;
	uint32_t m_positionHash;
	union
	{
		struct
		{
			uint16_t m_value;
			uint16_t unk_1A;
		} m_objectId;
		uint32_t m_identifier;
	};
	uint32_t m_uniquifier;
	BYTE m_isObjectId;
	char pad_21[3];
};

static char (*g_origGameScriptId_Read)(CGameScriptId*, rage::datBitBuffer*);
static char GameScriptId_Read(CGameScriptId* self, rage::datBitBuffer* buffer)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGameScriptId_Read(self, buffer);
	}

	buffer->ReadInteger(&self->m_scriptHash, 32);
	buffer->ReadInteger(&self->unk_10, 32);

	bool hasPositionHash = false;
	if (buffer->ReadBit(&hasPositionHash) && hasPositionHash)
	{
		buffer->ReadInteger(&self->m_positionHash, 32);
	}
	else
	{
		self->m_positionHash = 0;
	}

	uint32_t identifier = -1;

	bool hasIdentifier = false;
	if (buffer->ReadBit(&hasIdentifier) && hasIdentifier)
	{
		bool isObjectId = false;

		if (buffer->ReadBit(&isObjectId) && isObjectId)
		{
			buffer->ReadInteger((uint32_t*)&self->m_objectId.m_value, icgi->OneSyncBigIdEnabled ? 16 : 13);
			identifier = self->m_objectId.m_value;
		}
		else
		{
			bool isBigIdentifier = false;
			buffer->ReadBit(&isBigIdentifier);

			buffer->ReadInteger(&self->m_identifier, isBigIdentifier ? 32 : 9);
			identifier = self->m_identifier;
		}
	}
	else
	{
		self->m_identifier = -1;
	}

	self->m_uniquifier = (identifier == -1) ? (self->m_positionHash + self->m_scriptHash) : (identifier + self->m_scriptHash + 1);

	return true;
}

static char (*g_origGameScriptId_Write)(CGameScriptId*, rage::datBitBuffer*);
static char GameScriptId_Write(CGameScriptId* self, rage::datBitBuffer* buffer)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGameScriptId_Write(self, buffer);
	}

	buffer->WriteUns(self->m_scriptHash, 32); // rage::scriptId::Write
	buffer->WriteUns(self->unk_10, 32);

	bool hasPositionHash = (self->m_positionHash != 0);

	buffer->WriteBit(hasPositionHash);
	if (hasPositionHash)
	{
		buffer->WriteUns(self->m_positionHash, 32);
	}

	bool hasIdentifier = (self->m_identifier != -1);

	buffer->WriteBit(hasIdentifier);
	if (hasIdentifier)
	{
		buffer->WriteBit(self->m_isObjectId);
		if (self->m_isObjectId)
		{
			buffer->WriteUns(self->m_objectId.m_value, icgi->OneSyncBigIdEnabled ? 16 : 13);
		}
		else
		{
			bool isBigIdentifier = (self->m_identifier >= 512);

			buffer->WriteBit(isBigIdentifier);
			buffer->WriteUns(self->m_identifier, isBigIdentifier ? 32 : 9);
		}
	}

	return true;
}

static void (*g_origNetworkEventComponentControlBase_Write)(CNetworkEventComponentControlBase*, rage::datBitBuffer*);
static void NetworkEventComponentControlBase_Write(CNetworkEventComponentControlBase* self, rage::datBitBuffer* buffer)
{
	static_assert(sizeof(CNetworkEventComponentControlBase) == 0x18);
	static_assert(sizeof(CNetworkEventVehComponentControl) == 0x20);

	if (!icgi->OneSyncEnabled)
	{
		return g_origNetworkEventComponentControlBase_Write(self, buffer);
	}

	buffer->WriteUns(self->m_objectId1, icgi->OneSyncBigIdEnabled ? 16 : 13);
	buffer->WriteUns(self->m_objectId2, icgi->OneSyncBigIdEnabled ? 16 : 13);
	buffer->WriteUns(self->unk_C, 6);
	buffer->WriteUns(self->unk_D, 1);
}

static void (*g_origNetworkEventVehComponentControl_Write)(CNetworkEventVehComponentControl*, rage::datBitBuffer*);
static void NetworkEventVehComponentControl_Write(CNetworkEventVehComponentControl* self, rage::datBitBuffer* buffer)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origNetworkEventVehComponentControl_Write(self, buffer);
	}

	buffer->WriteBit(self->unk_1A);

	if (self->unk_D || !self->unk_1A)
	{
		return;
	}

	buffer->WriteUns(self->m_occupantObjectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
}

static void (*g_origNetworkEventVehComponentControl_WriteInner)(CNetworkEventVehComponentControl*, rage::datBitBuffer*);
static void NetworkEventVehComponentControl_WriteInner(CNetworkEventVehComponentControl* self, rage::datBitBuffer* buffer)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origNetworkEventVehComponentControl_WriteInner(self, buffer);
	}

	if (!self->unk_E)
	{
		return;
	}

	buffer->WriteBit(self->unk_1A);

	if (!self->unk_1A)
	{
		return;
	}

	buffer->WriteUns(self->m_occupantObjectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
}

static uint32_t(*g_origGetPhysicalMappedObjectId)(CPhysical*);
static uint32_t GetPhysicalMappedObjectId(CPhysical* physical)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPhysicalMappedObjectId(physical);
	}

	const auto netObject = (rage::netObject*)physical->GetNetObject();

	if (!netObject)
	{
		return physical->GetUnkId();
	}

	return netObject->GetObjectId();
}

class CScriptEntityStateChangeEvent
{
public:
	class IScriptEntityStateParametersBase
	{
	public:
		virtual ~IScriptEntityStateParametersBase() = 0;
	};

	class CSetVehicleExclusiveDriver : IScriptEntityStateParametersBase
	{
	public:
		uint16_t m_objectId;
		char m_pad[2];
		uint32_t m_index;
	};

	class CSettingOfLookAtEntity : IScriptEntityStateParametersBase
	{
	public:
		uint16_t m_objectId;
		char m_pad[2];
		uint32_t m_flags;
		int unk_10;
		uint32_t unk_14;
	};
};

static void (*g_origSetVehicleExclusiveDriver_Write)(CScriptEntityStateChangeEvent::CSetVehicleExclusiveDriver*, rage::datBitBuffer*);
static void SetVehicleExclusiveDriver_Write(CScriptEntityStateChangeEvent::CSetVehicleExclusiveDriver* self, rage::datBitBuffer* buffer)
{
	static_assert(sizeof(CScriptEntityStateChangeEvent::CSetVehicleExclusiveDriver) == 0x10);

	if (!icgi->OneSyncEnabled)
	{
		return g_origSetVehicleExclusiveDriver_Write(self, buffer);
	}

	buffer->WriteUns(self->m_objectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
	buffer->WriteUns(self->m_index, 2);
}

static void (*g_origSettingOfLookAtEntity_Write)(CScriptEntityStateChangeEvent::CSettingOfLookAtEntity*, rage::datBitBuffer*);
static void SettingOfLookAtEntity_Write(CScriptEntityStateChangeEvent::CSettingOfLookAtEntity* self, rage::datBitBuffer* buffer)
{
	static_assert(sizeof(CScriptEntityStateChangeEvent::CSettingOfLookAtEntity) == 0x18);

	if (!icgi->OneSyncEnabled)
	{
		return g_origSettingOfLookAtEntity_Write(self, buffer);
	}

	buffer->WriteUns(self->m_objectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
	buffer->WriteUns(self->m_flags, 18);
	buffer->WriteUns(self->unk_14, 10);
}

static rlGamerInfo* (*g_origNetGamePlayerGetGamerInfo)(CNetGamePlayer*);

static rlGamerInfo* NetGamePlayerGetGamerInfo(CNetGamePlayer* self)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origNetGamePlayerGetGamerInfo(self);
	}

	if (auto playerInfo = self->GetPlayerInfo())
	{
		return (rlGamerInfo*)((char*)playerInfo + 0x20);
	}

	FatalError("CNetGamePlayer::GetGamerInfo returned nullptr, this is a fatal error.\n"
		"Physical player index: %d\nActive player index: %d\nPlayer address: %x\n\n"
		"Please report this issue, together with the information from 'Save information' down below on\n"
		"https://forum.cfx.re/t/cnetgameplayer-getgamerinfo-returns-nullptr-causing-crashes",
		self->physicalPlayerIndex(), self->activePlayerIndex(), (uint64_t)self);
}

static bool (*g_origNetGamePlayerIsVisibleToPlayer)(CNetGamePlayer*, CNetGamePlayer*, char);

static bool NetGamePlayerIsVisibleToPlayer(CNetGamePlayer* player, CNetGamePlayer* target, char flags)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origNetGamePlayerIsVisibleToPlayer(player, target, flags);
	}

	// The game only update "visible players bitset" once per second, if remote player scope-out between
	// updates and local player would be "lucky" enough to make the game call this function during gameplay,
	// they will most likely get crash because of bitset still containing invalid player marked as "visible".
	// So we make sure that target player is still have physical entity, before returning true.

	bool result = g_origNetGamePlayerIsVisibleToPlayer(player, target, flags);

	// don't continue when original function returned false or target is player 31
	if (!result || !target || target->physicalPlayerIndex() == 31)
	{
		return false;
	}

	// do not allow to return true if remote player has no physical entity
	if (!getPlayerPedForNetPlayer(target))
	{
		return false;
	}

	return true;
}
#endif

static HookFunction hookFunction([]()
{
#ifdef GTA_FIVE
	// use cached sector if no gameobject (weird check in IProximityMigrateableNodeDataAccessor impl)
	hook::nop(hook::get_pattern("FF 90 ? 00 00 00 33 C9 48 85 C0 74 4C", 11), 2);
#endif

	// dummy/ambient object player list ordering logic

	// CNetObjObject/creation node
	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				push(rax); // we want to save al ('Player Wants Control' bool)
				sub(rsp, 0x20);

#ifdef GTA_FIVE
				mov(rcx, r13); // netobj is in r13
#elif IS_RDR3
				mov(rcx, r15); // netobj is in r15
#endif

				mov(rax, (uint64_t)Fn);
				call(rax);

				mov(dl, al);

				add(rsp, 0x20);
				pop(rax);

				ret();
			}

			static bool Fn(rage::netObject* object)
			{
				auto localId = g_netLibrary->GetServerNetID();
				auto ownerId = TheClones->GetClientId(object);

				if (localId < ownerId)
				{
					ownerId = localId;
				}

				return (localId == ownerId);
			}
		} playerListOrder;

#ifdef GTA_FIVE
		hook::call_rcx(hook::get_pattern("0F 42 D3 3A DA 0F 94 C2", 3), playerListOrder.GetCode());
#elif IS_RDR3
		auto location = hook::get_pattern<char>("48 0F 42 C3 48 3B D8 0F 94 C2", 4);
		hook::nop(location - 21, 30);
		hook::call_rcx(location, playerListOrder.GetCode());
		hook::put<uint16_t>(location + 7, 0xC084); // test al, al
#endif
	}

	// TODO: same as above for door
	// TODO: similar for pickup/placement

	// CNetObjObject ownership-conflict deletion
	{
		static struct : jitasm::Frontend
		{
			// input:  rbx -> object a1
			//         rdi -> object from list
			// output: rdi -> object from list (again)
			//         al  -> treat as return
			void InternalMain() override
			{
				sub(rsp, 0x28);

				mov(rcx, rbx);
				mov(rdx, rdi);

				mov(rax, (uint64_t)Fn);
				call(rax);

				add(rsp, 0x28);
				ret();
			}

			static bool Fn(rage::netObject* selfObject, rage::netObject* listObject)
			{
				auto selfId = TheClones->GetClientId(selfObject);
				auto listId = TheClones->GetClientId(listObject);

				if (selfId < listId)
				{
					selfId = listId;
				}

				return (selfId != listId);
			}
		} ownerLoop;

#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("48 8B CB E8 ? ? ? ? 48 85 C0 74 4F");
		hook::nop(location, 0x3F);
		hook::call_rcx(location, ownerLoop.GetCode());
		hook::put<uint16_t>(location + 0x3F, 0xC084); // test al, al
#elif IS_RDR3
		if (xbr::IsGameBuildOrGreater<1436>())
		{
			auto location = hook::get_pattern<char>("48 8B CB E8 ? ? ? ? 48 85 C0 75 27 8D 50 01");
			hook::nop(location, 0x6E);
			hook::call_rcx(location, ownerLoop.GetCode());
			hook::put<uint16_t>(location + 0x6E, 0xC084); // test al, al
		}
		else
		{
			auto location = hook::get_pattern<char>("48 8B CB E8 ? ? ? ? 48 85 C0 74 57 48 8B CB");
			hook::nop(location, 0x47);
			hook::call_rcx(location, ownerLoop.GetCode());
			hook::put<uint16_t>(location + 0x47, 0xC084); // test al, al
		}
#endif
	}

#ifdef GTA_FIVE
	// net damage array, size 32*4
	uint32_t* damageArrayReplacement = (uint32_t*)hook::AllocateStubMemory(256 * sizeof(uint32_t));
	memset(damageArrayReplacement, 0, 256 * sizeof(uint32_t));

	{
		std::initializer_list<std::tuple<std::string_view, int>> bits = {
			{ "74 30 3C 20 73 0D 48 8D 0D", 9 },
			{ "0F 85 9F 00 00 00 48 85 FF", 0x12 },
			{ "80 F9 FF 74 2F 48 8D 15", 8 },
			{ "80 BF ? 00 00 00 FF 74 21 48 8D", 12 }
		};

		for (const auto& bit : bits)
		{
			auto location = hook::get_pattern<int32_t>(std::get<0>(bit), std::get<1>(bit));

			hook::put<int32_t>(location, (intptr_t)damageArrayReplacement - (intptr_t)location - 4);
		}

		// 128
		hook::put<uint8_t>(hook::get_pattern("74 30 3C 20 73 0D 48 8D 0D", 3), 0x80);
	}

	// temp dbg
	//hook::put<uint16_t>(hook::get_pattern("0F 84 80 00 00 00 49 8B 07 49 8B CF FF 50 20"), 0xE990);

	//hook::put(0x141980A68, NetLogStub_DoLog);
	//hook::put(0x141980A50, NetLogStub_DoTrace);
	//hook::jump(0x140A19640, NetLogStub_DoLog);
#endif

	// netobjmgr count, temp dbg
#ifdef GTA_FIVE
	hook::put<uint8_t>(hook::get_pattern("48 8D 05 ? ? ? ? BE 1F 00 00 00 48 8B F9", 8), 128);
#elif IS_RDR3
	//hook::put<uint8_t>(hook::get_pattern("48 8D 05 ? ? ? ? BF 20 00 00 00 48 89 01", 8), 128);
#endif

	// 1604 unused, netobjmgr alloc size, temp dbg
	// 1737 made this arxan
	//hook::put<uint32_t>(0x14101CF4F, 32712 + 4096);

	MH_Initialize();

#ifdef GTA_FIVE
	if (xbr::IsGameBuildOrGreater<2545>() && !xbr::IsGameBuildOrGreater<3095>())
	{
		MH_CreateHook(hook::get_pattern("B1 0A 48 89 03 33 C0 48 89", -0x1F), NetworkObjectMgrCtorStub, (void**)&g_origNetworkObjectMgrCtor);
	}
	else
	{
		MH_CreateHook(hook::get_pattern("48 89 03 33 C0 B1 0A 48 89", -0x15), NetworkObjectMgrCtorStub, (void**)&g_origNetworkObjectMgrCtor);
	}

	MH_CreateHook(hook::get_pattern("4C 8B F1 41 BD 05", -0x22), PassObjectControlStub, (void**)&g_origPassObjectControl);
	MH_CreateHook(hook::get_pattern("8A 41 49 4C 8B F2 48 8B", -0x10), SetOwnerStub, (void**)&g_origSetOwner);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("48 8B D9 E8 ? ? ? ? 33 ? 66 C7 83", (xbr::IsGameBuildOrGreater<1355>()) ? -0xA : -0x6), NetworkObjectMgrCtorStub, (void**)&g_origNetworkObjectMgrCtor);
	MH_CreateHook(hook::get_pattern("83 FE 01 41 0F 9F C4 48 85 DB 74", (xbr::IsGameBuildOrGreater<1436>()) ? -0x99 : -0x71), PassObjectControlStub, (void**)&g_origPassObjectControl);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 80 7B 47 00 75 ? 48 8B 03")), SetOwnerStub, (void**)&g_origSetOwner);
#endif

	// scriptHandlerMgr::ManageHostMigration, has fixed 32 player array and isn't needed* for 1s
#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("01 4F 60 81 7F 60 D0 07 00 00 0F 8E", -0x47), ManageHostMigrationStub, (void**)&g_origManageHostMigration);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("83 F8 01 0F 86 ? ? ? ? 8B 0D ? ? ? ? 01 4F", -0x38), ManageHostMigrationStub, (void**)&g_origManageHostMigration);
#endif

	// 1604 unused, some bubble stuff
	//hook::return_function(0x14104D148);
#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? 48 8D 0D"), UnkBubbleWrap, (void**)&g_origUnkBubbleWrap);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("48 83 EC ? 8A 05 ? ? ? ? 33 DB 0F 29 74 24 60", -6), UnkBubbleWrap, (void**)&g_origUnkBubbleWrap);
#endif

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("0F 29 70 C8 0F 28 F1 33 DB 45", -0x1C), GetPlayersNearPoint, (void**)&g_origGetPlayersNearPoint);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("33 DB 0F 29 70 D8 49 8B F9 4D 8B F0", -0x1B), GetPlayersNearPoint, (void**)&g_origGetPlayersNearPoint);
#endif

	// return to disable breaking hooks
	//return;

	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern("44 89 BE B4 00 00 00 FF 90", 7);
#elif IS_RDR3
		auto location = hook::get_pattern("45 8D 41 03 FF 90 ? ? ? ? 84 C0 0F", 4);
#endif
		hook::nop(location, 6);

		// 0x160 in 1604 - 0x170 in 2545
		hook::call(location, netObject__CanPassControl);
	}

#ifdef GTA_FIVE
	{
		// General area: 8B 05 ? ? ? ? FF C8 44 3B E8 75
		auto location = hook::get_pattern("48 8B CF FF 90 ? ? 00 00 84 C0 74 12 48 8B CF", 3);
		hook::nop(location, 6);

		// 0x170 in 1604 - 0x180 in 2545
		hook::call(location, netObject__CanBlend);
	}
#endif

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("8A 41 49 3C FF 74 17 3C 20 73 13 0F B6 C8"), netObject__GetPlayerOwner, (void**)&g_origGetOwnerNetPlayer);
	MH_CreateHook(hook::get_pattern("8A 41 4A 3C FF 74 17 3C 20 73 13 0F B6 C8"), netObject__GetPendingPlayerOwner, (void**)&g_origGetPendingPlayerOwner);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern((xbr::IsGameBuildOrGreater<1436>()) ? "8A 49 45 80 F9 20 72 03 33 C0 C3" : "80 79 45 20 72 ? 33 C0 C3"), netObject__GetPlayerOwner, (void**)&g_origGetOwnerNetPlayer);
	MH_CreateHook(hook::get_pattern((xbr::IsGameBuildOrGreater<1436>()) ? "8A 49 46 80 F9 FF 75 03" : "8A 41 46 3C FF 74"), netObject__GetPendingPlayerOwner, (void**)&g_origGetPendingPlayerOwner);
#endif

	// function is only 4 bytes, can't be hooked like this
	//MH_CreateHook(hook::get_call(hook::get_pattern("FF 50 68 49 8B CE E8 ? ? ? ? 48 8B 05", 6)), netObject__GetPlayerOwnerId, (void**)&g_origGetOwnerPlayerId);

	// NETWORK_GET_PLAYER_INDEX_FROM_PED
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern("74 ? 48 8B 88 ? ? ? ? 48 85 C9 74 ? E8 ? ? ? ? 0F B6", 14);
#elif IS_RDR3
		auto location = hook::get_pattern("48 85 C9 74 ? E8 ? ? ? ? 0F B6 C0 EB 05", 5);
#endif
		hook::set_call(&g_origGetOwnerPlayerId, location);
		hook::call(location, netObject__GetPlayerOwnerId);
	}

#ifdef IS_RDR3
	// ped texture overriding natives
	{
		auto location = hook::get_pattern("48 85 C9 74 09 E8 ? ? ? ? 8A D8 EB 02", 5);
		hook::call(location, netObject__GetPlayerOwnerId);
	}

	{
		auto location = hook::get_pattern("4C 8D ? 08 48 69 C8 ? ? ? ? 4C 03", 52);
		hook::call(location, netObject__GetPlayerOwnerId);
	}
#endif

#ifdef GTA_FIVE
	hook::jump(hook::get_pattern("C6 41 4A FF C3", 0), netObject__ClearPendingPlayerIndex);
#elif IS_RDR3
	hook::jump(hook::get_pattern("C6 41 46 FF C3", 0), netObject__ClearPendingPlayerIndex);
#endif

	// replace joining local net player to bubble
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern("48 8B D0 E8 ? ? ? ? E8 ? ? ? ? 83 BB ? ? ? ? 04", 3);
#elif IS_RDR3
		auto location = (xbr::IsGameBuildOrGreater<1436>()) ? hook::get_pattern("40 0F B6 CF 48 89 44 CB 40 48", -5) : hook::get_pattern("48 85 C9 74 ? 4C 8D 44 24 40 40 88 7C", 18);
#endif

		hook::set_call(&g_origJoinBubble, location);
		hook::call(location, JoinPhysicalPlayerOnHost);
	}

	{
#ifdef GTA_FIVE
		if (!xbr::IsGameBuildOrGreater<2060>())
		{
			auto match = hook::pattern("80 F9 20 73 13 48 8B").count(2);
			MH_CreateHook(match.get(0).get<void>(0), GetPlayerByIndex, (void**)&g_origGetPlayerByIndex);
			MH_CreateHook(match.get(1).get<void>(0), GetPlayerByIndex, nullptr);
		}
		else
		{
			auto match = hook::pattern("80 F9 20 73 13 48 8B").count(1);
			MH_CreateHook(match.get(0).get<void>(0), GetPlayerByIndex, (void**)&g_origGetPlayerByIndex);

			// #2 is arxan in 1868
			MH_CreateHook(hook::get_call(hook::get_pattern("40 0F 92 C7 40 84 FF 0F 85 ? ? ? ? 40 8A CE E8", 16)), GetPlayerByIndex, nullptr);
		}
#elif IS_RDR3
		auto pattern = (xbr::IsGameBuildOrGreater<1436>()) ? "80 F9 20 72 2B BA" : "80 F9 20 73 13 48 8B";
		auto match = hook::pattern(pattern).count(2);
		MH_CreateHook(match.get(0).get<void>((xbr::IsGameBuildOrGreater<1436>()) ? -19 : 0), GetPlayerByIndex, (void**)&g_origGetPlayerByIndex);
		MH_CreateHook(match.get(1).get<void>((xbr::IsGameBuildOrGreater<1436>()) ? -19 : 0), GetPlayerByIndex, nullptr);
#endif
	}

	MH_CreateHook(hook::get_pattern("48 83 EC 28 33 C0 38 05 ? ? ? ? 74 0A"), GetPlayerByIndexNet, (void**)&g_origGetPlayerByIndexNet);

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("75 07 ? C9 0F 94 C3 EB", xbr::IsGameBuildOrGreater<3095>() ? -0x27 : -0x19), IsNetworkPlayerActive, (void**)&g_origIsNetworkPlayerActive);
	MH_CreateHook(hook::get_pattern("75 07 85 C9 0F 94 C0 EB", -0x13), IsNetworkPlayerConnected, (void**)&g_origIsNetworkPlayerConnected); // connected
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("75 0A 85 C9 0F 94 C3 E9", -0x22), IsNetworkPlayerActive, (void**)&g_origIsNetworkPlayerActive);
	MH_CreateHook(hook::get_pattern("80 3D ? ? ? ? 00 75 ? 85 C9 0F 94 C0 EB", -0x9), IsNetworkPlayerConnected, (void**)&g_origIsNetworkPlayerConnected);
#endif

	//MH_CreateHook(hook::get_pattern("84 C0 74 0B 8A 9F ? ? 00 00", -0x14), netPlayer_IsActiveStub, (void**)&g_origNetPlayer_IsActive);

	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("44 0F 28 CF F3 41 0F 59 C0 F3 44 0F 59 CF F3 44 0F 58 C8 E8", 19);
		MH_CreateHook(hook::get_call(location + 0), netInterface_GetNumRemotePhysicalPlayers, (void**)&g_origGetNetworkPlayerListCount);
		MH_CreateHook(hook::get_call(location + 8), netInterface_GetRemotePhysicalPlayers, (void**)&g_origGetNetworkPlayerList);
#elif IS_RDR3
		auto location = hook::get_pattern<char>("48 8B 07 48 8B CF FF 90 ? ? ? ? 44 8B ? E8");
		MH_CreateHook(hook::get_call(location + 15), netInterface_GetNumRemotePhysicalPlayers, (void**)&g_origGetNetworkPlayerListCount);
		MH_CreateHook(hook::get_call(location + 22), netInterface_GetRemotePhysicalPlayers, (void**)&g_origGetNetworkPlayerList);
#endif
	}

	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("48 8B F0 85 DB 74 56 8B", -0x34);
		MH_CreateHook(hook::get_call(location + 0x28), netInterface_GetNumPhysicalPlayers, (void**)&g_origGetNetworkPlayerListCount2);
		MH_CreateHook(hook::get_call(location + 0x2F), netInterface_GetAllPhysicalPlayers, (void**)&g_origGetNetworkPlayerList2);
#elif IS_RDR3
		auto location = hook::get_pattern<char>("8A 43 30 3C 04 75 ? E8");
		MH_CreateHook(hook::get_call(location + 0x7), netInterface_GetNumPhysicalPlayers, (void**)&g_origGetNetworkPlayerListCount2);
		MH_CreateHook(hook::get_call(location + 0xF), netInterface_GetAllPhysicalPlayers, (void**)&g_origGetNetworkPlayerList2);
#endif
	}

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("48 85 DB 74 20 48 8B 03 48 8B CB FF 50 ? 48 8B", xbr::IsGameBuildOrGreater<3095>() ? -0x3D : -0x34),
		(xbr::IsGameBuildOrGreater<2824>()) ? GetPlayerFromGamerId<2824> :
		(xbr::IsGameBuildOrGreater<2372>()) ? GetPlayerFromGamerId<2372> :
		(xbr::IsGameBuildOrGreater<2060>()) ? GetPlayerFromGamerId<2060> : GetPlayerFromGamerId<1604>,
	(void**)&g_origGetPlayerFromGamerId);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("48 85 DB 74 20 48 8B 03 48 8B CB FF 50 ? 48 8B", -0x27), GetPlayerFromGamerId, (void**)&g_origGetPlayerFromGamerId);
#endif

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("4C 8B F9 74 7D", -0x2B), netObjectMgr__CountObjects, (void**)&g_origCountObjects);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("4C 8B F9 49 8B CC 4C 8B F2 33 FF E8", -0x24), netObjectMgr__CountObjects, (void**)&g_origCountObjects);
#endif

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("0F 29 70 C8 4D 8B E1 4D 8B E8", -0x1C), networkBandwidthMgr_CalculatePlayerUpdateLevelsStub, (void**)&g_origNetworkBandwidthMgr_CalculatePlayerUpdateLevels);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("0F 29 78 B8 4D 8B E0 48 8B F2 48 8B F9 33 DB E8", -0x23), networkBandwidthMgr_CalculatePlayerUpdateLevelsStub, (void**)&g_origNetworkBandwidthMgr_CalculatePlayerUpdateLevels);
#endif

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("78 18 4C 8B 05", -10), GetScenarioTaskScenario, (void**)&g_origGetScenarioTaskScenario);

	// #TODO1S: fix player/ped groups so we don't need this workaround anymore
	MH_CreateHook(hook::get_call(hook::get_pattern("48 8B 04 28 4D 8B 3C 2C 49 89 04 2C", 24)), GetNetObjPlayerGroup, (void**)&g_origGetNetObjPlayerGroup);
#endif

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("45 8D 65 20 C6 81 ? ? 00 00 01 48 8D 59 08", -0x2F), ObjectManager_End, (void**)&g_origObjectManager_End);
	
	if (xbr::IsGameBuildOrGreater<3258>())
	{
		MH_CreateHook(hook::get_call(hook::get_call(hook::get_pattern<char>("48 8B CB E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 ? 5B", 3))), PlayerManager_End, (void**)&g_origPlayerManager_End);
	}
	else if (xbr::IsGameBuildOrGreater<2545>())
	{
		MH_CreateHook(hook::get_call(hook::get_call(hook::get_pattern<char>("84 C0 74 14 48 8B CB E8 ? ? ? ? 48 8D 8B", 7))), PlayerManager_End, (void**)&g_origPlayerManager_End);
	}
	else
	{
		MH_CreateHook(hook::get_call(hook::get_call(hook::get_pattern<char>("48 8D 05 ? ? ? ? 48 8B D9 48 89 01 E8 ? ? ? ? 84 C0 74 08 48 8B CB E8", -0x19) + 0x32)), PlayerManager_End, (void**)&g_origPlayerManager_End);
	}
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("48 83 EC 30 45 33 FF 48 8B F1 44 38 B9", -0x18), ObjectManager_End, (void**)&g_origObjectManager_End);
	MH_CreateHook(hook::get_call(hook::get_call(hook::get_pattern<char>("48 8D 05 ? ? ? ? 48 8B F9 48 89 01 E8 ? ? ? ? 84 C0 74 08 48 8B ? E8", -0xF) + 0x28)), PlayerManager_End, (void**)&g_origPlayerManager_End);
#endif

	// disable voice chat bandwidth estimation for 1s (it will overwrite some memory)
#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("40 8A 72 ? 40 80 FE FF 0F 84", -0x46), VoiceChatMgr_EstimateBandwidth, (void**)&g_origVoiceChatMgr_EstimateBandwidth);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("0F B6 72 ? F6 84 B3 ? ? ? ? 01 75", -0x57), VoiceChatMgr_EstimateBandwidth, (void**)&g_origVoiceChatMgr_EstimateBandwidth);
#endif

#ifdef GTA_FIVE
	// getnetplayerped 32 cap
	hook::nop(hook::get_pattern("83 F9 1F 77 26 E8", 3), 2);
#endif

#ifdef IS_RDR3
	// in RDR3 net player relevance position is cached in array indexed with physical player index, we need to patch it
	MH_CreateHook((xbr::IsGameBuildOrGreater<1436>()) ? hook::get_pattern("0F A3 D0 0F 92 C0 88 06", -0x76) : hook::get_pattern("44 0F A3 C0 0F 92 C0 41 88 02", -0x32), getNetPlayerRelevancePosition, (void**)&g_origGetNetPlayerRelevancePosition);
#endif

	// always allow to migrate, even if not cloned on bit test
#ifdef GTA_FIVE
	hook::put<uint8_t>(hook::get_pattern("75 29 48 8B 02 48 8B CA FF 50"), 0xEB);
#elif IS_RDR3
	hook::put<uint8_t>(hook::get_pattern("75 29 48 8B 06 48 8B CE FF 50 60"), 0xEB);
#endif

	// delete objects from clonemgr before deleting them
	static struct : public jitasm::Frontend
	{
		virtual void InternalMain() override
		{
#ifdef GTA_FIVE
			mov(rcx, rbx);
			mov(rax, (uintptr_t)&DeletionMethod);
			jmp(rax);
#elif IS_RDR3
			mov(rcx, rsi);
			mov(rax, (uintptr_t)&DeletionMethod);
			jmp(rax);
#endif
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

#ifdef GTA_FIVE
	hook::call(hook::get_pattern("48 C1 EA 04 E8 ? ? ? ? 48 8B 03", 17), delStub.GetCode());
#elif IS_RDR3
	if (xbr::IsGameBuildOrGreater<1436>())
	{
		hook::call(hook::get_pattern("48 8B 06 41 8B D4 48 8B CE FF 10 48 8B 5C", 6), delStub.GetCode());
	} 
	else
	{
		hook::call(hook::get_pattern("48 8B 06 BA 01 00 00 00 48 8B CE FF 10 48 8B 5C 24 50", 8), delStub.GetCode());
	}
#endif

#ifdef IS_RDR3
	// unsafe CNetGamePlayer player ped getter call patch
	static struct : public jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			movzx(ecx, dl);
			mov(rax, (uintptr_t)&GetNetPlayerPedSafe);
			jmp(rax);
		}

		static void* GetNetPlayerPedSafe(uint8_t index)
		{
			if (auto player = GetPlayerByIndex(index))
			{
				return getPlayerPedForNetPlayer(player);
			}

			return nullptr;
		}
	} playerPedGetterStub;

	{
		auto location = hook::get_pattern("48 8B 05 ? ? ? ? 0F B6 CA 48 8B 8C C8 ? ? ? ? E8");
		hook::nop(location, 23);
		hook::call(location, playerPedGetterStub.GetCode());
	}

	// Patch `CGameScriptId::Write` and `CGameScriptId::Read` methods.
	{
		static_assert(sizeof(rage::scriptIdBase) == 0x8);
		static_assert(sizeof(rage::scriptId) == 0x10);
		static_assert(sizeof(CGameScriptId) == 0x28);

		static constexpr int kGameScriptIdReadIndex = 5;
		static constexpr int kGameScriptIdWriteIndex = 6;

		const auto gameScriptIdVtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8B F9 41 8A D8 48 83 C1 10 48 89 01 89 69 08", -15));

		g_origGameScriptId_Read = (decltype(g_origGameScriptId_Read))gameScriptIdVtable[kGameScriptIdReadIndex];
		hook::put(&gameScriptIdVtable[kGameScriptIdReadIndex], (uintptr_t)GameScriptId_Read);

		g_origGameScriptId_Write = (decltype(g_origGameScriptId_Write))gameScriptIdVtable[kGameScriptIdWriteIndex];
		hook::put(&gameScriptIdVtable[kGameScriptIdWriteIndex], (uintptr_t)GameScriptId_Write);
	}

	// CSyncedDataWriter/CSyncedDataReader::SerializeObjectID (static).
	MH_CreateHook(hook::get_pattern("66 3B C1 76 ? 33 C0 EB ? 0F B7 0A", -0xE), SyncDataWriter_SerializeObjectIdStatic, (void**)&g_origSyncDataWriter_SerializeObjectIdStatic);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 53 0A")), SyncDataReader_SerializeObjectIdStatic, (void**)&g_origSyncDataReader_SerializeObjectIdStatic);

	// CSyncedDataWriter/CSyncedDataReader::SerializeObjectID (prefixed).
	MH_CreateHook(hook::get_pattern("E8 ? ? ? ? 0F B7 03 B9", -0x10), SyncDataWriter_SerializeObjectIdPrefix, (void**)&g_origSyncDataWriter_SerializeObjectIdPrefix);
	MH_CreateHook(hook::get_pattern("B9 ? ? ? ? 49 8B 04 ? 8A 14 01", -0x36), SyncDataReader_SerializeObjectIdPrefix, (void**)&g_origSyncDataReader_SerializeObjectIdPrefix);

	// Patch methods of `NetworkEventComponentControlBase` and derivative classes. We don't need to patch reading methods as these are using generic SerializeObjectID method that is already patched.
	MH_CreateHook(hook::get_pattern("57 48 83 EC ? 0F B7 41 ? BD ? ? ? ? 66 FF C8", -0xF), NetworkEventComponentControlBase_Write, (void**)&g_origNetworkEventComponentControlBase_Write);
	MH_CreateHook(hook::get_pattern("38 53 1A 74 2E", -0x22), NetworkEventVehComponentControl_Write, (void**)&g_origNetworkEventVehComponentControl_Write);
	MH_CreateHook(hook::get_pattern("38 59 ? 74 ? 8A 51", -0x17), NetworkEventVehComponentControl_WriteInner, (void**)&g_origNetworkEventVehComponentControl_WriteInner);
	// `CScriptEntityStateChangeEvent` related, we don't need to patch reading methods as these are using generic SerializeObjectID method that is already patched.
	MH_CreateHook(hook::get_pattern("8B 53 0C 41 B8 02 00 00 00 48 8B CF 48 8B", -0x45), SetVehicleExclusiveDriver_Write, (void**)&g_origSetVehicleExclusiveDriver_Write);
	MH_CreateHook(hook::get_pattern("8B 53 14 41 B8 0A 00 00 00 48 8B CF 48 8B", -0x56), SettingOfLookAtEntity_Write, (void**)&g_origSettingOfLookAtEntity_Write);
	// Remove Object ID being checked against Object ID Mapping
	MH_CreateHook(hook::get_pattern("74 25 0F B7 48 42", -0xC), GetPhysicalMappedObjectId, (void**)&g_origGetPhysicalMappedObjectId);

	// RespawnPlayerPedEvent offset
	{
		g_respawnPlayerPedEvent_objIdRefOffset = *hook::get_pattern<uint8_t>("66 89 7B ? 44 89 73", 3);
		g_respawnPlayerPedEvent_objIdOffset = *hook::get_pattern<uint8_t>("66 44 89 73 ? E8 ? ? ? ? 48 85 C0", 4);
	}
	
	MH_CreateHook(hook::get_call(hook::get_pattern("48 89 5C 24 ? 57 48 83 EC ? 48 8B DA 48 8B F9 E8 ? ? ? ? 48 83 64 24 ? ? 48 8D 05", 57)), RespawnPlayerPedEvent_Serialize, (void**)&g_origRespawnPlayerPedEvent_Serialize);
	// removing object id mapping in CTaskClimbLadder
	{
		auto location = hook::get_pattern("0F B7 50 42 41 B9 3F 1F 00 00", 4);
		static struct : public jitasm::Frontend
		{
			intptr_t RetSuccess;
			intptr_t RetFail;

			void Init(const intptr_t retSuccess, const intptr_t retFail)
			{
				this->RetSuccess = retSuccess;
				this->RetFail = retFail;
			}

			virtual void InternalMain() override
			{
				sub(rsp, 0x20);
				mov(rcx, rax);
				mov(rdx, r8);

				mov(rax, reinterpret_cast<uintptr_t>(&CompareObjectIds));
				call(rax);

				add(rsp, 0x20);

				test(eax, eax);
				jz("fail");
						
				mov(rax, RetSuccess);
				jmp(rax);

				L("fail");
				mov(rax, RetFail);
				jmp(rax);  
			}

			static bool CompareObjectIds(rage::netObject* leftObj, rage::netObject* rightObj)
			{
				if (!leftObj || !rightObj)
				{
					return false;
				}

				auto lId = leftObj->GetObjectId();
				auto rId = rightObj->GetObjectId();

				if (lId == 0 || rId == 0)
				{
					return false;
				}

				return rId <= lId;				
			}
		} climbLadderStub;

		auto loc = hook::get_pattern("48 8B 0D ? ? ? ? 8D 42");
		auto retFail = (intptr_t)loc + 48;
		auto retSuccess = (intptr_t)loc + 52;
		climbLadderStub.Init(retSuccess, retFail);

		hook::nop(location, 0x1D);
		hook::jump(location, climbLadderStub.GetCode());
	}

	// patch sync data size calculator allowing more bits
	{
		static constexpr int kSizeCalculatorSerializePlayerIndex = 25;
		static constexpr int kSizeCalculatorSerializeObjectId1 = 26;
		static constexpr int kSizeCalculatorSerializeObjectId2 = 27;
		static constexpr int kSizeCalculatorSerializeObjectId3 = 28;
		
		const auto sizeCalculatorVtable = hook::get_address<uintptr_t*>(hook::get_pattern("B8 BF FF 00 00 48 8B CF 66 21 87", 27));
		
		g_origSyncDataSizeCalculator_SerializePlayerIndex = (decltype(g_origSyncDataSizeCalculator_SerializePlayerIndex))sizeCalculatorVtable[kSizeCalculatorSerializePlayerIndex];
		hook::put(&sizeCalculatorVtable[kSizeCalculatorSerializePlayerIndex], (uintptr_t)SyncDataSizeCalculator_SerializePlayerIndex);
		
		g_origSyncDataSizeCalculator_SerializeObjectId = (decltype(g_origSyncDataSizeCalculator_SerializeObjectId))sizeCalculatorVtable[kSizeCalculatorSerializeObjectId1];
		hook::put(&sizeCalculatorVtable[kSizeCalculatorSerializeObjectId1], (uintptr_t)SyncDataSizeCalculator_SerializeObjectId);
		hook::put(&sizeCalculatorVtable[kSizeCalculatorSerializeObjectId2], (uintptr_t)SyncDataSizeCalculator_SerializeObjectId);
		hook::put(&sizeCalculatorVtable[kSizeCalculatorSerializeObjectId3], (uintptr_t)SyncDataSizeCalculator_SerializeObjectId);
	}

	// patch SerializePlayerIndex methods of sync data reader/writer
	MH_CreateHook(hook::get_pattern("80 3B 20 73 ? 65 4C 8B 0C", -0x2F), SyncDataReader_SerializePlayerIndex, (void**)&g_origSyncDataReader_SerializePlayerIndex);
	MH_CreateHook(xbr::IsGameBuildOrGreater<1436>() ? hook::get_pattern("41 B2 3F 48 8D 54 24 30 44 88", -30) : hook::get_pattern("80 3A 20 48 8B D9 C6 44", -6), SyncDataWriter_SerializePlayerIndex, (void**)&g_origSyncDataWriter_SerializePlayerIndex);
	
	// attempt to get some information about CNetGamePlayer::GetGamerInfo related crashes
	{
		static constexpr int kNetGamePlayerGetGamerInfoIndex = 12;

		auto netGamePlayerVtable = hook::get_address<uintptr_t*>(hook::get_pattern("E8 ? ? ? ? 33 F6 48 8D 05 ? ? ? ? 48 8D 8B", 10));
		g_origNetGamePlayerGetGamerInfo = (decltype(g_origNetGamePlayerGetGamerInfo))netGamePlayerVtable[kNetGamePlayerGetGamerInfoIndex];
		hook::put(&netGamePlayerVtable[kNetGamePlayerGetGamerInfoIndex], (uintptr_t)NetGamePlayerGetGamerInfo);
	}

	// patch CAIConditionIsLocalPlayerVisibleToAnyPlayer behavior to properly handle scoping players
	{
		auto location = hook::get_pattern<char>("40 0F B6 D5 8B C2 44 8B C2 48", -13);
		MH_CreateHook(hook::get_call(location), NetGamePlayerIsVisibleToPlayer, (void**)&g_origNetGamePlayerIsVisibleToPlayer);
	}

	// patch crash caused by _getTaskScenarioInfo returning nullptr for CTaskUseScenario sometimes for an unknown reason
	{
		auto location = (char*)hook::get_call(hook::get_pattern("E8 ? ? ? ? 4D 8B 75 10"));

		g_pointIdGetterVtableOffset = *(int*)(location + 18);
		g_scenarioInfoManager = hook::get_address<decltype(g_scenarioInfoManager)>(location + 12);
		g_taskUseScenarioVtable = hook::get_address<void*>(hook::get_pattern<char>("0F 29 70 C8 44 8B FA 0F 29 78 B8 48 8B F9 0F", 31));

		MH_CreateHook(location, GetTaskScenarioInfo, (void**)&g_origGetTaskScenarioInfo);
	}
#endif

	// clobber nodes for all players, not just when connected to netplayermgr
	// not working, maybe?
#ifdef GTA_FIVE
	hook::nop(hook::get_pattern("0F A3 C7 73 18 44", 3), 2);
#elif IS_RDR3
	hook::nop(hook::get_pattern("8B 44 84 58 0F A3 D0 73", 7), 2);
#endif

	// always write up-to-date data to nodes, not the cached data from syncdata
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern("FF 90 ? ? ? ? 84 C0 0F 84 80 00 00 00 49", 0);
#elif IS_RDR3
		auto location = hook::get_pattern("44 8A 84 24 ? ? ? ? ? 8B D5 48 8B CF FF 90", 14);
#endif

		hook::nop(location, 6);
		hook::call(location, mD0Stub);
	}

#ifdef GTA_FIVE
	// some built-in text/voice chat logic using physical player index as a fixed 32-sized array index
	if (xbr::IsGameBuildOrGreater<2699>())
	{
		auto location = hook::get_call(hook::get_pattern<char>("48 69 C9 ? ? ? ? 48 81 C1 ? ? ? ? 48 03 CE E8", 17));
		MH_CreateHook(location, ManageTextVoiceChatStub, (void**)&g_origManageTextVoiceChatStub);	
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

		if (xbr::IsGameBuildOrGreater<2802>())
		{
			hook::put<uint32_t>(location + 0xFB, stackSize);
			hook::put<uint32_t>(location + 0xCC, intsBase);
		}
		else
		{
			hook::put<uint32_t>(location + 0xF8, stackSize);
			hook::put<uint32_t>(location + 0xC9, intsBase);
		}
	}

	// same for CNetObjPed
	{
		auto location = hook::get_pattern<char>("48 81 EC A0 01 00 00 33 FF 48 8B D9", -0x15);

		auto stackSize = (0x20 + (256 * 8) + (256 * 4));
		auto ptrsBase = 0x20;
		auto intsBase = ptrsBase + (256 * 8);

		hook::put<uint32_t>(location + 0x18, stackSize);

		if (xbr::IsGameBuildOrGreater<2802>())
		{
			hook::put<uint32_t>(location + 0x13F, stackSize);
			hook::put<uint32_t>(location + 0xD8, intsBase);
		}
		else
		{
			hook::put<uint32_t>(location + 0x13C, stackSize);
			hook::put<uint32_t>(location + 0xD5, intsBase);
		}
	}

	// 32 array size somewhere called by CTaskNMShot
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		auto location = hook::get_pattern<char>("48 8D A8 28 FF FF FF 48 81 ? ? ? ? 00 80 3D", -0x1C);

		hook::put<uint32_t>(location + 0x26, 0xB0 + (256 * 8));
		hook::put<uint32_t>(location + 0x26C, 0xB0 + (256 * 8));
	}
	else
	{
		auto location = hook::get_pattern<char>("48 8D A8 38 FF FF FF 48 81 ? ? ? ? 00 80 3D", -0x1C);

		hook::put<uint32_t>(location + 0x26, 0xA0 + (256 * 8));
		hook::put<uint32_t>(location + 0x269, 0xA0 + (256 * 8));
	}

	// 32 array size for network object limiting
	// #TODO: unwind info for these??
	if (!xbr::IsGameBuildOrGreater<2060>()) // only validated for 1604 so far
	{
		auto location = hook::get_pattern<char>("48 85 C0 0F 84 C3 06 00 00 E8", -0x4A);

		// stack frame ENTER
		hook::put<uint32_t>(location + 0x19, 0xE58);

		// stack frame LEAVE
		hook::put<uint32_t>(location + 0x71A, 0xE58);

		// var: rsp+1A0
		hook::put<uint32_t>(location + 0x3A8, 0xDA0);
		hook::put<uint32_t>(location + 0x3C4, 0xDA0);
		hook::put<uint32_t>(location + 0x40E, 0xDA0);
		hook::put<uint32_t>(location + 0x41D, 0xDA0);

		// var: rsp+1A8
		hook::put<uint32_t>(location + 0x3B6, 0xDA8);
		hook::put<uint32_t>(location + 0x3CB, 0xDA8);
		hook::put<uint32_t>(location + 0x427, 0xDA8);
		hook::put<uint32_t>(location + 0x431, 0xDA8);

		// var: rsp+1B0
		hook::put<uint32_t>(location + 0x3AF, 0xDB0);
		hook::put<uint32_t>(location + 0x3D2, 0xDB0);
		hook::put<uint32_t>(location + 0x440, 0xDB0);
		hook::put<uint32_t>(location + 0x453, 0xDB0);

		// var: rsp+1B8
		hook::put<uint32_t>(location + 0xA9, 0xDB8);
		hook::put<uint32_t>(location + 0x366, 0xDB8);
		hook::put<uint32_t>(location + 0x555, 0xDB8);

		// player indices
		hook::put<uint8_t>(location + 0x467, 0x7F); // 127.
		hook::put<uint8_t>(location + 0x39E, 0x7F);
	}

	// CNetworkDamageTracker float[32] array
	// (would overflow into pool data and be.. quite bad)
	{
		// pool constructor call
		hook::put<uint32_t>(hook::get_pattern("C7 44 24 20 88 00 00 00 E8", 4), sizeof(void*) + (sizeof(float) * 256));

		// memset in CNetworkDamageTracker::ctor
		hook::put<uint32_t>(hook::get_pattern("48 89 11 48 8B D9 41 B8 80 00 00 00", 8), sizeof(float) * 256);
	}
#endif

	MH_EnableHook(MH_ALL_HOOKS);

});

#include <nutsnbolts.h>
#include <GameInit.h>

extern rage::netObject* g_curNetObject;

static char(*g_origReadDataNode)(void* node, uint32_t serializationMode, uint32_t flags, rage::datBitBuffer* buffer, void* logger);

std::map<int, std::map<void*, std::tuple<int, uint32_t>>> g_netObjectNodeMapping;

static bool ReadDataNodeStub(void* node, uint32_t serializationMode, uint32_t flags, rage::datBitBuffer* buffer, void* logger)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origReadDataNode(node, serializationMode, flags, buffer, logger);
	}

	// enable this for boundary checks
	/*if (flags == 1 || flags == 2)
	{
		uint32_t in = 0;
		buffer->ReadInteger(&in, 8);
		assert(in == 0x5A);
	}*/

	bool didRead = g_origReadDataNode(node, serializationMode, flags, buffer, logger);

	if (didRead && g_curNetObject)
	{
		g_netObjectNodeMapping[g_curNetObject->GetObjectId()][node] = { 0, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() };
	}

	return didRead;
}

#ifdef GTA_FIVE
static char(*g_origWriteDataNode)(void* node, uint32_t flags, void* mA0, rage::netObject* object, rage::datBitBuffer* buffer, int time, void* playerObj, char playerId, void* unk, void* unk2545);

static bool WriteDataNodeStub(void* node, uint32_t flags, void* mA0, rage::netObject* object, rage::datBitBuffer* buffer, int time, void* playerObj, char playerId, void* unk, void* unk2545)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk, unk2545);
	}

	if (playerId != 31 || flags == 4)
	{
		return g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk, unk2545);
	}
	else
	{
		// save position and write a placeholder length frame
		uint32_t position = buffer->GetPosition();
		buffer->WriteBit(false);
		buffer->WriteUns(0, 11);

		bool rv = g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk, unk2545);

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
				g_netObjectNodeMapping[g_curNetObject->GetObjectId()][node] = { 1, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() };
			}
		}
		else
		{
			buffer->Seek(position + 1);
		}

		return rv;
	}
}
#elif IS_RDR3
static char(*g_origWriteDataNode)(void* node, uint32_t flags, uint32_t objectFlags, rage::netObject* object, rage::datBitBuffer* buffer, void* playerObj, char playerId, void* a8, void* unk);

static bool WriteDataNodeStub(void* node, uint32_t flags, uint32_t objectFlags, rage::netObject* object, rage::datBitBuffer* buffer, void* playerObj, char playerId, void* a8, void* unk)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origWriteDataNode(node, flags, objectFlags, object, buffer, playerObj, playerId, a8, unk);
	}

	if (playerId != 31 || flags == 4)
	{
		return g_origWriteDataNode(node, flags, objectFlags, object, buffer, playerObj, playerId, a8, unk);
	}
	else
	{
		// save position and write a placeholder length frame
		uint32_t position = buffer->GetPosition();
		buffer->WriteBit(false);
		buffer->WriteUns(0, 11);

		bool rv = g_origWriteDataNode(node, flags, objectFlags, object, buffer, playerObj, playerId, a8, unk);

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
				g_netObjectNodeMapping[g_curNetObject->GetObjectId()][node] = { 1, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() };
			}
		}
		else
		{
			buffer->Seek(position + 1);
		}

		return rv;
	}
}
#endif

//
//static void(*g_origUpdateSyncDataOn108)(void* node, void* object);
//static void UpdateSyncDataOn108Stub(void* node, void* object)
//{
//	//if (!icgi->OneSyncEnabled)
//	{
//		g_origUpdateSyncDataOn108(node, object);
//	}
//}

static void(*g_origManuallyDirtyNode)(void* node, void* object);

namespace rage
{
class netSyncDataNodeBase;
}

extern void DirtyNode(rage::netObject* object, rage::netSyncDataNodeBase* node);

static void ManuallyDirtyNodeStub(rage::netSyncDataNodeBase* node, rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		g_origManuallyDirtyNode(node, object);
		return;
	}

	DirtyNode(object, node);
}

#ifdef GTA_FIVE
static void(*g_orig_netSyncDataNode_ForceSend)(void* node, int actFlag1, int actFlag2, rage::netObject* object);

static void netSyncDataNode_ForceSendStub(rage::netSyncDataNodeBase* node, int actFlag1, int actFlag2, rage::netObject* object)
{
	if (!icgi->OneSyncEnabled)
	{
		g_orig_netSyncDataNode_ForceSend(node, actFlag1, actFlag2, object);
		return;
	}

	// maybe needs to read act flags?
	DirtyNode(object, node);
}
#elif IS_RDR3
static void (*g_orig_netSyncDataNode_ForceSend)(void* node, int actFlag1, int actFlag2, rage::netObject* object, bool unk);

static void netSyncDataNode_ForceSendStub(rage::netSyncDataNodeBase* node, int actFlag1, int actFlag2, rage::netObject* object, bool unk)
{
	if (!icgi->OneSyncEnabled)
	{
		g_orig_netSyncDataNode_ForceSend(node, actFlag1, actFlag2, object, unk);
		return;
	}

	// maybe needs to read act flags?
	DirtyNode(object, node);
}
#endif

static void(*g_orig_netSyncDataNode_ForceSendToPlayer)(void* node, int player, int actFlag1, int actFlag2, rage::netObject* object);

static void netSyncDataNode_ForceSendToPlayerStub(rage::netSyncDataNodeBase* node, int player, int actFlag1, int actFlag2, rage::netObject* object)
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
#ifndef ONESYNC_CLONING_NATIVES
#ifdef GTA_FIVE
		char* location;

		if (xbr::IsGameBuildOrGreater<2824>())
		{
			location = hook::get_pattern<char>("4C 8D 4C 24 30 4C 8D 86 B0 02 00 00 48 8B D0 48", -0x70);
			hook::set_call(&g_origWriteDataNode, location + 0x89);
		}
		else if (xbr::IsGameBuildOrGreater<2802>())
		{
			location = hook::get_pattern<char>("48 89 44 24 20 E8 ? ? ? ? 84 C0 0F 95 C0 48 83 C4 58", -0x63);
			hook::set_call(&g_origWriteDataNode, location + 0x68);
		}
		else
		{
			location = hook::get_pattern<char>("48 89 44 24 20 E8 ? ? ? ? 84 C0 0F 95 C0 48 83 C4 58", -0x3C);
			hook::set_call(&g_origWriteDataNode, location + 0x41);
		}
#elif IS_RDR3
		auto location = hook::get_pattern<char>("49 89 43 C8 E8 ? ? ? ? 84 C0 0F 95 C0 48 83 C4 58", -0x3E);
		hook::set_call(&g_origWriteDataNode, location + 0x42);
#endif

		hook::jump(location, WriteDataNodeStub);
#endif
	}

	{
		MH_Initialize();

#ifdef GTA_FIVE
		MH_CreateHook(hook::get_pattern("48 8B 03 48 8B D6 48 8B CB EB 06", -0x48), ReadDataNodeStub, (void**)&g_origReadDataNode);
#elif IS_RDR3
		MH_CreateHook(xbr::IsGameBuildOrGreater<1436>() ? hook::get_pattern("42 8A BC 6B", -0x33) : hook::get_pattern("40 8A BC 43", -0x3D), ReadDataNodeStub, (void**)&g_origReadDataNode);
#endif

#ifdef GTA_FIVE
		{
			auto floc = xbr::IsGameBuildOrGreater<2545>() ? hook::get_pattern<char>("84 C0 0F 84 39 01 00 00 48 39 77", -0x2B) : hook::get_pattern<char>("84 C0 0F 84 39 01 00 00 48 83 7F", -0x29);
			hook::set_call(&g_origCallSkip, floc + 0xD9);
			hook::call(floc + 0xD9, SkipCopyIf1s);
			// MH_CreateHook(floc, UpdateSyncDataOn108Stub, (void**)&g_origUpdateSyncDataOn108);
		}
#endif

#ifdef GTA_FIVE
		MH_CreateHook(xbr::IsGameBuildOrGreater<2545>() ? hook::get_pattern("48 83 79 48 00 48 8B FA 48 8B D9 74 40", -10) : hook::get_pattern("48 83 79 48 00 48 8B D9 74 19", -6), ManuallyDirtyNodeStub, (void**)&g_origManuallyDirtyNode);
		MH_CreateHook(hook::get_pattern("85 51 28 0F 84 E4 00 00 00 33 DB", -0x24), netSyncDataNode_ForceSendStub, (void**)&g_orig_netSyncDataNode_ForceSend);
		MH_CreateHook(hook::get_pattern("44 85 41 28 74 73 83 79 30 00", -0x1F), netSyncDataNode_ForceSendToPlayerStub, (void**)&g_orig_netSyncDataNode_ForceSendToPlayer);
#elif IS_RDR3
		MH_CreateHook(hook::get_pattern("FF 50 20 8B 57 3C 48 8B C8 E8", -0x29), ManuallyDirtyNodeStub, (void**)&g_origManuallyDirtyNode);
		MH_CreateHook(hook::get_pattern("85 51 28 0F 84 ? ? ? ? 33 DB 39", -0x24), netSyncDataNode_ForceSendStub, (void**)&g_orig_netSyncDataNode_ForceSend);
		MH_CreateHook(hook::get_pattern("44 85 41 28 0F 84 ? ? ? ? 83", -0x18), netSyncDataNode_ForceSendToPlayerStub, (void**)&g_orig_netSyncDataNode_ForceSendToPlayer);
#endif

		MH_EnableHook(MH_ALL_HOOKS);
	}
});

static InitFunction initFunctionWorldGrid([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		lib->AddPacketHandler<fx::WorldGridPacketHandler>(false);
	});
});

bool(*g_origDoesLocalPlayerOwnWorldGrid)(float* pos);

namespace sync
{
extern void* origin;
extern float* (*getCoordsFromOrigin)(void*, float*);
}

bool DoesLocalPlayerOwnWorldGrid(float* pos)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origDoesLocalPlayerOwnWorldGrid(pos);
	}

	alignas(16) float centerOfWorld[4];
	sync::getCoordsFromOrigin(sync::origin, centerOfWorld);

	float dX = pos[0] - centerOfWorld[0];
	float dY = pos[1] - centerOfWorld[1];

	// #TODO1S: make server able to send current range for player (and a world grid granularity?)
	constexpr float maxRange = (424.0f * 424.0f);
	
	int sectorX = std::max(pos[0] + 8192.0f, 0.0f) / 150;
	int sectorY = std::max(pos[1] + 8192.0f, 0.0f) / 150;

	auto playerIdx = g_netIdsByPlayer[rage::GetLocalPlayer()];

	bool does = false;

	for (const auto& entry : fx::g_worldGrid.entries)
	{
		if (entry.sectorX == sectorX && entry.sectorY == sectorY && entry.slotID == playerIdx)
		{
			does = true;
			break;
		}
	}

	// if the entity would be created outside of culling range, suppress it
	if (((dX * dX) + (dY * dY)) > maxRange)
	{
		if (does)
		{
			does = false;
		}
	}

	return does;
}

static int GetScriptParticipantIndexForPlayer(CNetGamePlayer* player)
{
	return player->physicalPlayerIndex();
}

#ifdef GTA_FIVE
static int DoesLocalPlayerOwnWorldGridWrapForInline(float* pos)
{
	return (DoesLocalPlayerOwnWorldGrid(pos)) ? 0 : 1;
}

static int* dword_1427EA288;
static int* dword_141D56E08;

static hook::cdecl_stub<float(void*, void*, void*, void*, void*, void*, void*)> _countPopVehiclesForSpace([]()
{
	return hook::get_call(hook::get_pattern("40 38 3D ? ? ? ? F3 0F 2C C0 89", -5));
});

static hook::cdecl_stub<bool(int peds, int cars, int n5s, int n8s, int n3s, bool)> _checkForObjectSpace([]()
{
	return hook::get_call(hook::get_pattern("45 33 C9 45 33 C0 33 C9 40 88", 17));
});

static bool PopulationPedCheck(int num)
{
	int count = (int)_countPopVehiclesForSpace(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) - *dword_1427EA288;

	if (count > 0)
	{
		return _checkForObjectSpace(num, 0, 0, 0, 0, false);
	}

	return true;
}
#elif IS_RDR3
bool (*g_origShouldSkipWaterPopulationSpawn)(float* position, float a2, char* a3);

bool ShouldSkipWaterPopulationSpawn(float* position, float a2, char* a3)
{
	auto result = g_origShouldSkipWaterPopulationSpawn(position, a2, a3);

	// if the original function allowed to spawn water population, check for world grid ownership.
	if (icgi->OneSyncEnabled && !result)
	{
		result = !DoesLocalPlayerOwnWorldGrid(position);
	}

	return result;
}
#endif

static HookFunction hookFunctionWorldGrid([]()
{
#ifdef GTA_FIVE
	// inline turntaking for ped creation
	{
		auto location = (xbr::IsGameBuildOrGreater<2944>()) ? hook::get_pattern<char>("45 33 C9 44 88 7C 24 20 E8 ? ? ? ? 33 FF 44 8B F8", 8) : hook::get_pattern<char>("45 33 C9 44 88 74 24 20 E8 ? ? ? ? 44 8B E0", 8);
		hook::call(location, DoesLocalPlayerOwnWorldGridWrapForInline);

		// if '1', instantly fail, don't try to iterate that one player
		const int offsetStart = (xbr::IsGameBuildOrGreater<2944>()) ? 14 : 12;
		const int offsetEnd = (xbr::IsGameBuildOrGreater<2944>()) ? 0x156 : 0x16D;

		hook::jump(location + offsetStart + 4, location - offsetEnd);

		// precede it with moving the iterator back in `edx`, or this will loop for a *long* time sometimes
		hook::put<uint32_t>(location + offsetStart, 0x6424548B);
	}

	// second variant of the same
	{
		auto location = (xbr::IsGameBuildOrGreater<2944>()) ? hook::get_pattern<char>("48 8B CB 44 88 64 24 20 E8 ? ? ? ? 41 8B F4", 8) : hook::get_pattern<char>("48 8B CE 44 88 6C 24 20 E8 ? ? ? ? 45 8B FD", 8);
		hook::call(location, DoesLocalPlayerOwnWorldGridWrapForInline);

		const int offsetEnd = (xbr::IsGameBuildOrGreater<2944>()) ? 0xD6 : 0xD5;

		// if '1', instantly fail, don't try to iterate that one player
		hook::jump(location + 15, location - offsetEnd);
	}

	// turntaking
	hook::jump(hook::get_pattern("48 8D 4C 24 30 45 33 C9 C6", xbr::IsGameBuildOrGreater<2944>() ? -0x28 : -0x30), DoesLocalPlayerOwnWorldGrid);

	if (xbr::IsGameBuildOrGreater<2944>())
	{
		hook::jump(hook::get_pattern("BB 01 00 00 00 8B F8 85 C0 0F 84", -0x35), DoesLocalPlayerOwnWorldGrid);
	}
	else
	{
		hook::jump(hook::get_pattern(((xbr::IsGameBuildOrGreater<2060>()) ? "BE 01 00 00 00 8B E8 85 C0 0F 84 B8" : "BE 01 00 00 00 45 33 C9 40 88 74 24 20"), ((xbr::IsGameBuildOrGreater<2060>()) ? -0x3A : -0x2D)), DoesLocalPlayerOwnWorldGrid);
	}
#elif IS_RDR3
	hook::jump(hook::get_pattern("0F 28 01 4C 8D 44 24 30 0F", -0xA), DoesLocalPlayerOwnWorldGrid);
#endif

	MH_Initialize();

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("44 8A 40 ? 41 80 F8 FF 0F", -0x1B), DoesLocalPlayerOwnWorldGrid, (void**)&g_origDoesLocalPlayerOwnWorldGrid);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("44 0F B6 C0 F3 0F 5F C3 41 0F B6 04 10", -0x35), DoesLocalPlayerOwnWorldGrid, (void**)&g_origDoesLocalPlayerOwnWorldGrid);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 11 B9")), ShouldSkipWaterPopulationSpawn, (void**)&g_origShouldSkipWaterPopulationSpawn);
#endif

	MH_EnableHook(MH_ALL_HOOKS);

	// if population breaks in non-1s, this is possibly why
	//hook::nop(hook::get_pattern("38 05 ? ? ? ? 75 0A 48 8B CF E8", 6), 2);

	// 1493+ of the above patch
	//hook::nop(hook::get_pattern("80 3D ? ? ? ? 00 75 0A 48 8B CF E8", 7), 2);

	// this patch above ^ shouldn't be needed with timeSync properly implemented, gamerIDs being set and RemotePlayer list fixes

	// this should apply to both 1s and non-1s (as participants are - hopefully? - not used by anyone in regular net)
	hook::jump(hook::get_pattern("84 C0 74 06 0F ? 43 38", -0x18), GetScriptParticipantIndexForPlayer);

#ifdef GTA_FIVE
	// don't add 'maybe enough to give all our vehicles drivers' as a constraint for even creating one ped
	// (applies to non-1s too: generally safe)
	hook::call(hook::get_pattern("0F 28 F1 74 15 8D 48 01 E8", 8), PopulationPedCheck);
	hook::call(hook::get_pattern("48 8B D9 74 4C B9 01 00 00 00 E8", 10), PopulationPedCheck);

	dword_1427EA288 = hook::get_address<int*>(hook::get_pattern("F3 0F 2C C0 2B 05 ? ? ? ? 85 C0 0F", 6));
	dword_141D56E08 = hook::get_address<int*>(hook::get_pattern("3B C7 0F 42 F8 83 3D ? ? ? ? 01 75", -4));

	// sometimes it seems to need this to be patched still - hope that won't fail on 2060+
	// b2060+ seem to have this obfuscated, sorry guys
	if (!xbr::IsGameBuildOrGreater<2060>())
	{
		hook::put<uint16_t>(hook::get_pattern("F3 0F 59 C8 F3 48 0F 2C C9 03 CF E8", 9), 0xF989);
	}
#endif
});

int ObjectToEntity(int objectId)
{
	int entityIdx = -1;
	auto object = TheClones->GetNetObject(objectId & 0xFFFF);

	if (object && object->GetGameObject())
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

extern void ArrayManager_Update();

static InitFunction initFunction([]()
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
				console::DPrintf("onesync", "Dropping duplicate player for slotID %d.\n", info.slotId);
				HandleClientDrop(info);
			}

			if (g_playersByNetId[info.netId])
			{
				auto tempInfo = info;
				tempInfo.slotId = g_playersByNetId[info.netId]->physicalPlayerIndex();

				console::Printf("onesync", "Dropping duplicate player for netID %d (slotID %d).\n", info.netId, tempInfo.slotId);
				HandleClientDrop(tempInfo);
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
	});	

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

		ArrayManager_Update();
		rage::EventManager_Update();
		TheClones->Update();
	});

	OnKillNetworkDone.Connect([]()
	{
		trackedObjects.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_ENTITY_OWNER", [](fx::ScriptContext& context)
	{
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

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
		context.SetResult<int>(owner ? owner->physicalPlayerIndex() : 0xFF);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_FROM_STATE_BAG_NAME", [](fx::ScriptContext& context)
	{
		int entityId = 0;
		std::string bagName = context.CheckArgument<const char*>(0);

		if (bagName.find("entity:") == 0)
		{
			int parsedEntityId = atoi(bagName.substr(7).c_str());
			rage::netObjectMgr* netObjectMgr = rage::netObjectMgr::GetInstance();

			if (netObjectMgr)
			{
				rage::netObject* obj = netObjectMgr->GetNetworkObject(parsedEntityId, true);
				if (obj)
				{
					int guid = getScriptGuidForEntity((fwEntity*)obj->GetGameObject());
					if (guid)
					{
						entityId = guid;
					}
				}
			}
		}
		else if (bagName.find("localEntity:") == 0)
		{
			int parsedEntityId = atoi(bagName.substr(12).c_str());
			fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(parsedEntityId);
			// Verify the entity exists before returning the entity id
			if (entity)
			{
				entityId = parsedEntityId;
			}
		}

		context.SetResult<int>(entityId);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_FROM_STATE_BAG_NAME", [](fx::ScriptContext& context)
	{
		int playerId = 0;
		std::string bagName = context.CheckArgument<const char*>(0);

		if (bagName.find("player:") == 0)
		{
			int playerNetId = atoi(bagName.substr(7).c_str());
			if (auto player = GetPlayerByNetId(playerNetId))
			{
				playerId = player->physicalPlayerIndex();
			}
		}

		context.SetResult<int>(playerId);
	});

#ifdef ONESYNC_CLONING_NATIVES
	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_SAVE_CLONE_CREATE", [](fx::ScriptContext& context)
	{
		char* entity = (char*)rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("SAVE_CLONE_CREATE: invalid entity\n");

			context.SetResult<const char*>("");
			return;
		}

		auto netObj = GetNetObjectFromEntity(entity);

		static char blah[90000];

		static char bluh[1000];
		memset(bluh, 0, sizeof(bluh));
		memset(blah, 0, sizeof(blah));

		rage::datBitBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		st = rage::netSyncTree::GetForType((NetObjEntityType)netObj->GetObjectType());
		//st->Write(1, 0, netObj, &buffer, 0, 31, nullptr, nullptr);
		st->WriteTreeCfx(1, 0, netObj, &buffer, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp(), nullptr, 31, nullptr, nullptr);

		trace("create buffer length = %d\n", buffer.GetDataLength());

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
		char* entity = (char*)rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("SAVE_CLONE_SYNC: invalid entity\n");

			context.SetResult<const char*>("");
			return;
		}

		auto netObj = GetNetObjectFromEntity(entity);

		static char blah[90000];

		static char bluh[1000];
		memset(bluh, 0, sizeof(bluh));
		memset(blah, 0, sizeof(blah));

		rage::datBitBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		st = rage::netSyncTree::GetForType((NetObjEntityType)netObj->GetObjectType());
		//st->Write(2, 0, netObj, &buffer, 0, 31, nullptr, nullptr);
		st->WriteTreeCfx(2, 1, netObj, &buffer, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp(), nullptr, 31, nullptr, nullptr);

		static char base64Buffer[2000];

		size_t outLength = sizeof(base64Buffer);
		char* txt = base64_encode((uint8_t*)buffer.m_data, (buffer.m_curBit / 8) + 1, &outLength);

		memcpy(base64Buffer, txt, outLength);
		free(txt);

		// trace("saving netobj %llx\n", (uintptr_t)netObj);

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
		else if (strcmp(objectType, "boat") == 0)
		{
			objType = NetObjEntityType::Boat;
		}
		else if (strcmp(objectType, "trailer") == 0)
		{
			objType = NetObjEntityType::Trailer;
		}
#ifdef IS_RDR3
		else if (strcmp(objectType, "animal") == 0)
		{
			objType = NetObjEntityType::Animal;
		}
		else if (strcmp(objectType, "draftveh") == 0)
		{
			objType = NetObjEntityType::DraftVeh;
		}
		else if (strcmp(objectType, "horse") == 0)
		{
			objType = NetObjEntityType::Horse;
		}
#endif
		else if (strcmp(objectType, "train") == 0)
		{
			objType = NetObjEntityType::Train;
		}
		else if (strcmp(objectType, "player") == 0)
		{
			objType = NetObjEntityType::Player; // until we make native players
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

		rage::datBitBuffer buf(dec, decLen);
		buf.m_f1C = 1;

		auto st = rage::netSyncTree::GetForType(objType);

		st->ReadFromBuffer(1, 0, &buf, nullptr);

		free(dec);

		auto obj = rage::CreateCloneObject(objType, objectId, 31, 0, 32);
		obj->syncData.isRemote = true;

		if (!st->CanApplyToObject(obj))
		{
			trace("Couldn't apply object.\n");

			delete obj;

			context.SetResult(-1);
			return;
		}

#ifdef IS_RDR3
		auto check2 = (*(unsigned __int8(__fastcall**)(void*))(*(uint64_t*)obj + 0x98))(obj);
		auto check = st->InitialiseNode(obj, -1);

		if (!check && !check2)
		{
			trace("Can't apply\n");
			return;
		}

		(*(void(__fastcall**)(void*))(*(uint64_t*)obj + 0x248))(obj);
#endif

		st->ApplyToObject(obj, nullptr);
		rage::netObjectMgr::GetInstance()->RegisterNetworkObject(obj);

		// create a blender, if not existent
		if (!obj->GetBlender())
		{
			trace("Created net blender\n");
			obj->CreateNetBlender();
		}

		if (!obj->GetBlender())
		{
			trace("No blender even after creation, that's bad!\n");

			context.SetResult(-1);
			return;
		}

		(*(void(__fastcall**)(void*, uint32_t))(*(uint64_t*)obj + 0x260))(obj, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp());

		obj->GetBlender()->ApplyBlend();

		//obj->GetBlender()->SetTimestamp(rage::netInterface_queryFunctions::GetInstance()->GetTimestamp());
		//obj->SetBlenderTimestamp(rage::netInterface_queryFunctions::GetInstance()->GetTimestamp());
#ifdef IS_RDR3
		(*(void(__fastcall**)(void*))(*(uint64_t*)obj + 0x250))(obj);

		if ((*(unsigned __int8(__fastcall**)(void*))(*(uint64_t*)obj + 0x280))(obj))
		{
			obj->GetBlender()->m_70();
		}

		obj->GetBlender()->ApplyBlend();

		obj->GetBlender()->m_38();

		(*(void(__fastcall**)(void*))(*(uint64_t*)obj + 0x240))(obj);
#endif

#if 0
		if ((*(unsigned __int8(__fastcall**)(void*))(*(uint64_t*)obj + 0x288))(obj))
		{
			auto v156 = (*(__int64(__fastcall**)(void*))(*(uint64_t*)obj + 0xB0))(obj);
			if (v156)
			{
				auto v159 = (*(__int64(__fastcall**)(void*))(*(uint64_t*)obj + 0xB0))(obj);
				*(DWORD*)(v159 + 144) &= 0xFFFFF7FF;
				auto v160 = (*(__int64(__fastcall**)(void*))(*(uint64_t*)obj + 0xB0))(obj);
				*(DWORD*)(v160 + 144) |= 0x400u;
			}
		}
#endif

		context.SetResult(getScriptGuidForEntity(obj->GetGameObject()));
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_LOAD_CLONE_SYNC", [](fx::ScriptContext& context)
	{
		char* entity = (char*)rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("LOAD_CLONE_SYNC: invalid entity\n");
			return;
		}

		auto obj = GetNetObjectFromEntity(entity);

		auto data = context.GetArgument<const char*>(1);

		size_t decLen;
		uint8_t* dec = base64_decode(data, strlen(data), &decLen);

		rage::datBitBuffer buf(dec, decLen);
		buf.m_f1C = 1;

		auto st = obj->GetSyncTree();

		st->ReadFromBuffer(2, 0, &buf, nullptr);

		free(dec);

		if (!obj->GetBlender())
		{
			trace("LOAD_CLONE_SYNC: no net blender\n");
			return;
		}

#if 0
		(*(void(__fastcall**)(void*, uint32_t))(*(uint64_t*)obj + 0x260))(obj, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp());
		obj->GetBlender()->m_28();

		if (*(uint8_t*)((char*)obj + 84))
		{
			auto init = st->InitialiseTree();

			(*(void(__fastcall**)(void*, uint8_t, void*, void*, void*))(*(uint64_t*)st + 0x48))(
				st,
				2,
				obj,
				(void*)init,
				obj + 1056);
		}
		else
		{

			(*(void(__fastcall**)(void*))(*(uint64_t*)obj + 0x248))(obj);
			st->ApplyToObject(obj, nullptr);
			(*(void(__fastcall**)(void*))(*(uint64_t*)obj + 0x250))(obj);
			//trace("object = %d\n", (uint64_t)obj);
		}
#endif
		(*(void(__fastcall**)(void*, uint32_t))(*(uint64_t*)obj + 0x260))(obj, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp());
		//(*(void(__fastcall**)(void*, uint32_t))(*(uint64_t*)(obj->GetBlender()) + 0x38))(obj->GetBlender(), 0);

		(*(void(__fastcall**)(void*))(*(uint64_t*)obj + 0x248))(obj);
		st->ApplyToObject(obj, nullptr);
		(*(void(__fastcall**)(void*))(*(uint64_t*)obj + 0x250))(obj);

		//obj->GetBlender()->m_30();
		//obj->GetBlender()->m_58();

		//obj->GetBlender()->ApplyBlend();
		//obj->GetBlender()->m_38();

		//obj->PostCreate();
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_BLEND", [](fx::ScriptContext& context)
	{
		char* entity = (char*)rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("LOAD_CLONE_SYNC: invalid entity\n");
			return;
		}

		auto obj = GetNetObjectFromEntity(entity);
		//obj->GetBlender()->m_30();
		obj->GetBlender()->m_58();
	});

	static ConsoleCommand saveCloneCmd("save_clone", [](const std::string& address)
	{
		uintptr_t addressPtr = _strtoui64(address.c_str(), nullptr, 16);
		auto netObj = GetNetObjectFromEntity((void*)addressPtr);

		static char blah[90000];

		static char bluh[1000];

		rage::datBitBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		//st->Write(1, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp(), netObj, &buffer, 0, 31, nullptr, nullptr);

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

		rage::datBitBuffer buf(data, len);
		buf.m_f1C = 1;

		auto st = rage::netSyncTree::GetForType(NetObjEntityType::Ped);

		st->ReadFromBuffer(1, 0, &buf, nullptr);

		auto obj = rage::CreateCloneObject(NetObjEntityType::Ped, rand(), 31, 0, 32);

		if (!st->CanApplyToObject(obj))
		{
			trace("Couldn't apply object.\n");

			delete obj;
			return;
		}

		st->ApplyToObject(obj, nullptr);
		rage::netObjectMgr::GetInstance()->RegisterNetworkObject(obj);

		obj->GetBlender()->SetTimestamp(rage::netInterface_queryFunctions::GetInstance()->GetTimestamp());
		//obj->SetBlenderTimestamp(rage::netInterface_queryFunctions::GetInstance()->GetTimestamp());

		obj->PostSync();

		obj->GetBlender()->ApplyBlend();
		obj->GetBlender()->m_38();

		obj->PostCreate();

		trace("got game object %llx\n", (uintptr_t)obj->GetGameObject());
	});
#endif
});

uint32_t* rage__s_NetworkTimeLastFrameStart;
uint32_t* rage__s_NetworkTimeThisFrameStart;

static void (*g_origSendCloneSync)(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6);

static void SendCloneSync(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6)
{
	auto t = *rage__s_NetworkTimeThisFrameStart;
	*rage__s_NetworkTimeThisFrameStart = *rage__s_NetworkTimeLastFrameStart;

	g_origSendCloneSync(a1, a2, a3, a4, a5, a6);

	*rage__s_NetworkTimeThisFrameStart = t;
}

static void* (*g_origCPickupPlacement_ctor)(void* self, int a2, void* a3, void* a4, uint32_t flags, int a6);

static void* CPickupPlacement_ctor(void* self, int a2, void* a3, void* a4, uint32_t flags, int a6)
{
	if ((flags & 0xCF00000) == 0xCF00000)
	{
		flags &= ~0xCF00001;
	}

	return g_origCPickupPlacement_ctor(self, a2, a3, a4, flags, a6);
}

static HookFunction hookFunctionNative([]()
{
	MH_Initialize();

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("41 56 41 57 48 83 EC 40 0F B6 72 ? 4D 8B E1", -0x14), SendCloneSync, (void**)&g_origSendCloneSync);
	MH_CreateHook(hook::get_pattern("89 41 70 41 8B 40 04 89 41 74 41 8B", -0x64), CPickupPlacement_ctor, (void**)&g_origCPickupPlacement_ctor);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("C6 85 ? ? ? ? 00 ? 8B ? FF 90 ? ? ? ? 84 C0 75", -0x3C), SendCloneSync, (void**)&g_origSendCloneSync);
#endif

	MH_EnableHook(MH_ALL_HOOKS);

#ifdef GTA_FIVE
	if (xbr::IsGameBuildOrGreater<2944>())
	{
		rage__s_NetworkTimeThisFrameStart = hook::get_address<uint32_t*>(hook::get_pattern("49 8B 0C 24 2B C3 3B 1D ? ? ? ? 40 8A D6 45 1B C0", 8));
	}
	else
	{
		rage__s_NetworkTimeThisFrameStart = hook::get_address<uint32_t*>(hook::get_pattern("49 8B 0F 40 8A D6 41 2B C4 44 3B 25", 12));
	}
	rage__s_NetworkTimeLastFrameStart = hook::get_address<uint32_t*>(hook::get_pattern("89 05 ? ? ? ? 48 8B 01 FF 50 10 80 3D", 2));
#elif IS_RDR3
	rage__s_NetworkTimeThisFrameStart = hook::get_address<uint32_t*>(hook::get_pattern("74 ? 8B 05 ? ? ? ? 48 8B 0D ? ? ? ? 89", 4));
	rage__s_NetworkTimeLastFrameStart = hook::get_address<uint32_t*>(hook::get_pattern("89 05 ? ? ? ? 48 8B 01 FF 50 ? 80 3D", 2));
#endif
});

#ifdef GTA_FIVE
static hook::cdecl_stub<const char*(int, uint32_t)> rage__atHashStringNamespaceSupport__GetString([]
{
	return hook::get_pattern("85 D2 75 04 33 C0 EB 61", -0x18);
});

#include <Streaming.h>

struct CGameScriptId
{
	void* vtbl;
	uint32_t hash;
	char scriptName[32];
};

static hook::cdecl_stub<CGameScriptId*(rage::fwEntity*)> _getEntityScript([]()
{
	return hook::get_pattern("74 2A 48 8D 58 08 48 8B CB 48 8B 03", -0x18);
});

static auto PoolDiagDump(const char* poolName)
{
	auto pool = rage::GetPoolBase(poolName);

	std::map<std::string, int> poolCount;

	for (int i = 0; i < pool->GetSize(); i++)
	{
		auto e = pool->GetAt<rage::fwEntity>(i);

		if (e)
		{
			std::string desc;
			auto archetype = e->GetArchetype();
			auto archetypeHash = archetype->hash;
			auto archetypeName = streaming::GetStreamingBaseNameForHash(archetypeHash);

			if (archetypeName.empty())
			{
				archetypeName = va("0x%08x", archetypeHash);
			}

			desc = fmt::sprintf("%s", archetypeName);

			auto script = _getEntityScript(e);
			if (script)
			{
				desc += fmt::sprintf(" (script %s)", script->scriptName);
			}

			if (!e->GetNetObject())
			{
				desc += " (no netobj)";
			}

			poolCount[desc]++;
		}
	}

	std::vector<std::pair<int, std::string>> entries;

	for (const auto& [type, count] : poolCount)
	{
		entries.push_back({ count, type });
	}

	std::sort(entries.begin(), entries.end(), [](const auto& l, const auto& r)
	{
		return r.first < l.first;
	});

	std::string poolSummary;

	for (const auto& [count, type] : entries)
	{
		poolSummary += fmt::sprintf("  %s: %d entries\n", type, count);
	}

	return poolSummary;
}

static void PedPoolDiagError()
{
	FatalError("Ran out of ped pool space while creating network object.\n\nPed pool dump:\n%s", PoolDiagDump("Peds"));
}

static HookFunction hookFunctionDiag([]
{
	// CreatePed
	hook::call(hook::get_pattern("75 0E B1 01 E8 ? ? ? ? 33 C9 E8 ? ? ? ? 48 8B 03", 11), PedPoolDiagError);

	// CreatePlayer
	hook::call(hook::get_pattern("75 0E B1 01 E8 ? ? ? ? 33 C9 E8 ? ? ? ? 4C 8B 03", 11), PedPoolDiagError);

#if _DEBUG
	static ConsoleCommand pedCrashCmd("ped_crash", []()
	{
		PedPoolDiagError();
	});
#endif
});
#endif
