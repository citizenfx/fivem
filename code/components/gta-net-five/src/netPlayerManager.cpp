#include <StdInc.h>

#include "netPlayerManager.h"

#include "CrossBuildRuntime.h"
#include "Hooking.h"
#include "Hooking.Invoke.h"
#include "ICoreGameInit.h"
#include "MinHook.h"


rage::netPlayerMgrBase* g_playerMgr;
CNetGamePlayer* g_players[256];
std::unordered_map<uint16_t, CNetGamePlayer*> g_playersByNetId;
std::unordered_map<CNetGamePlayer*, uint16_t> g_netIdsByPlayer;

size_t g_CNetGamePlayerSize;

static CNetGamePlayer*(*g_origAllocateNetPlayer)(void*);

static hook::cdecl_stub<CNetGamePlayer*(void*)> g_netPlayerCtor([]()
{
#ifdef GTA_FIVE
	return (xbr::IsGameBuildOrGreater<2944>()) ? hook::get_pattern("83 8B ? 00 00 00 FF 48 8D 05 ? ? ? ? 33 F6", -0x17) : hook::get_pattern("83 8B ? 00 00 00 FF 33 F6", -0x17);
#else // IS_RDR3
	return hook::get_pattern("E8 ? ? ? ? 33 F6 48 8D 05 ? ? ? ? 48 8D 8B", -0x17);
#endif
});

static hook::thiscall_stub<void(rage::netPlayerMgrBase*, CNetGamePlayer*)> g_netPlayerMgrBase_UpdatePlayerListsForPlayer([]
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("FF 57 30 48 8B D6 49 8B CE E8", 9));
#else // IS_RDR3
	return hook::get_call(hook::get_pattern("48 8B 01 FF 50 ? 49 8B D7 48 8B CE E8", 12));
#endif
});

namespace rage
{
#ifdef GTA_FIVE
	CNetGamePlayer* netPlayerMgrBase::AddPlayer(void* scInAddr, void* unkNetValue, void* addedIn1290, void* playerData, void* nonPhysicalPlayerData)
	{
		if (xbr::IsGameBuildOrGreater<2372>())
		{
			static auto addPlayerFunc = *(uint64_t*)(*(char**)this + 0x20);
			return ((CNetGamePlayer*(*)(void*, void*, void*, void*, void*))(addPlayerFunc))(this, scInAddr, unkNetValue, playerData, nonPhysicalPlayerData);
		}

		return AddPlayer_raw(scInAddr, unkNetValue, addedIn1290, playerData, nonPhysicalPlayerData);
	}
#endif

	void netPlayerMgrBase::UpdatePlayerListsForPlayer(CNetGamePlayer* player)
	{
		g_netPlayerMgrBase_UpdatePlayerListsForPlayer(this, player);
	}

	CNetGamePlayer* netPlayerMgrBase::GetLocalPlayer()
	{
#ifdef GTA_FIVE
		const int offset = (xbr::IsGameBuildOrGreater<2944>() ? 240 : 232);
		return *(CNetGamePlayer**)((uint64_t)this + offset);
#elif IS_RDR3
		return *(CNetGamePlayer**)((uint64_t)this + 232);
#endif
	}

	CNetGamePlayer* GetLocalPlayer()
	{
		return g_playerMgr->GetLocalPlayer();
	}

	CNetGamePlayer* AllocateNetPlayer(void* mgr)
	{
		if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
		{
			return g_origAllocateNetPlayer(mgr);
		}

		// We assume this never fails (for now)
		void *plr = malloc(g_CNetGamePlayerSize);
		memset(plr, 0, g_CNetGamePlayerSize);

		auto player = g_netPlayerCtor(plr);

		// RDR3 wants CNetworkPlayerMgr pointer in CNetGamePlayer
	#ifdef IS_RDR3
		*(rage::netPlayerMgrBase**)((uint64_t)player + 288) = g_playerMgr;
	#endif

		return player;
	}
}

static HookFunction hookFunction([]()
{
	MH_Initialize();

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("48 8B F9 48 39 99 ? ? 00 00 74 ? 48 81 C1 ? ? 00 00 48", -12), rage::AllocateNetPlayer, (void**)&g_origAllocateNetPlayer);
#elif IS_RDR3
	MH_CreateHook((xbr::IsGameBuildOrGreater<1436>()) ? hook::get_pattern("33 DB 48 8B F9 48 39 99 ? ? ? ? 75 ? 8D 53 01", -10) : hook::get_pattern("48 39 99 ? ? ? ? 74 ? 48 81 C1 ? ? ? ? 48 8B 19 48 85", -15), rage::AllocateNetPlayer, (void**)&g_origAllocateNetPlayer);
#endif

	// CNetGamePlayer size
	{
#ifdef GTA_FIVE
		g_CNetGamePlayerSize = *hook::get_pattern<int32_t>("48 81 C7 ? ? ? ? FF CD 79 ED 33 ED", 3);
#elif IS_RDR3
		g_CNetGamePlayerSize = *hook::get_pattern<int32_t>("48 21 B3 ? ? ? ? 48 8B C3 48 21 B3 ? ? ? ? 48 21 B3", -10);
#endif
	}

#ifdef GTA_FIVE
	g_playerMgr = *hook::get_address<rage::netPlayerMgrBase**>(hook::get_pattern("40 80 FF 20 72 B3 48 8B 0D", 9));
#elif IS_RDR3
	g_playerMgr = *hook::get_address<rage::netPlayerMgrBase**>(hook::get_pattern("80 E1 07 80 F9 03 0F 84 ? ? ? ? 48 8B 0D", 15));
#endif

	MH_EnableHook(MH_ALL_HOOKS);
});
