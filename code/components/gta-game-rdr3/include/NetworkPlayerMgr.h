/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <CoreNetworking.h>

#ifdef COMPILING_GTA_GAME_RDR3
#define GTA_GAME_EXPORT DLL_EXPORT
#else
#define GTA_GAME_EXPORT DLL_IMPORT
#endif

namespace rage
{
	struct netNatType;
	struct rlRosPlayerAccountId;

	class netPlayer
	{
	public:
		virtual ~netPlayer() = 0;

		virtual void Init(rage::netNatType natType) = 0;

		virtual void Init(rage::rlRosPlayerAccountId const& accountId, uint32_t, rage::netNatType natType) = 0;

		virtual void Shutdown() = 0;

		virtual void IsPhysical() = 0;

		virtual const char* GetLogName() = 0;

		virtual netPeerId* GetRlPeerId() = 0;

		virtual bool IsOnSameTeam(rage::netPlayer const& player) = 0;

		virtual void ActiveUpdate() = 0;

		virtual bool IsHost() = 0;

		virtual void m_unk1() = 0;

		virtual void m_unk2() = 0;

		virtual rlGamerInfo* GetGamerInfo() = 0;
	};
}

class CNetGamePlayer : public rage::netPlayer
{
public:
	uint8_t pad[16];
	uint8_t activePlayerIndex;
	uint8_t physicalPlayerIndex;
};

class CNetworkPlayerMgr
{
public:
	static GTA_GAME_EXPORT CNetGamePlayer* GetPlayer(int playerIndex);
};
