#pragma once

#include "NetworkPlayerMgr.h"

#include <unordered_map>

namespace rage
{
	class netPlayerMgrBase
	{
	public:
		virtual ~netPlayerMgrBase() = 0;

		virtual void Initialize() = 0;

		virtual void Shutdown() = 0;

		virtual void m_18() = 0;

	#ifdef GTA_FIVE
	private:
		virtual CNetGamePlayer* AddPlayer_raw(void* scInAddr, void* unkNetValue, void* addedIn1290, void* playerData, void* nonPhysicalPlayerData) = 0;

	public:
	#elif IS_RDR3
		virtual CNetGamePlayer* AddPlayer(void* scInAddr, uint32_t activePlayerIndex, void* playerData, void* playerAccountId) = 0;
	#endif

		virtual void RemovePlayer(CNetGamePlayer* player) = 0;

		void UpdatePlayerListsForPlayer(CNetGamePlayer* player);

	#ifdef GTA_FIVE
	public:
		CNetGamePlayer* AddPlayer(void* scInAddr, void* unkNetValue, void* addedIn1290, void* playerData, void* nonPhysicalPlayerData);
	#endif

		CNetGamePlayer* GetLocalPlayer();
	};

	CNetGamePlayer* GetLocalPlayer();
	CNetGamePlayer* AllocateNetPlayer(void* mgr);
}

extern rage::netPlayerMgrBase* g_playerMgr;
extern CNetGamePlayer* g_players[256];
extern std::unordered_map<uint16_t, CNetGamePlayer*> g_playersByNetId;
extern std::unordered_map<CNetGamePlayer*, uint16_t> g_netIdsByPlayer;
