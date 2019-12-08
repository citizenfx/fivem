/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_GTA_GAME_FIVE
#define GTA_GAME_EXPORT DLL_EXPORT
#else
#define GTA_GAME_EXPORT DLL_IMPORT
#endif

struct netIpAddress
{
	union
	{
		uint32_t addr;
		uint8_t bytes[4];
	};
};

struct netSocketAddress
{
	netIpAddress ip;
	uint16_t port;
};

struct netPeerAddress
{
	uint64_t unkKey1;
	uint64_t unkKey2;
	uint32_t secKeyTime; // added in 393
	netSocketAddress relayAddr;
	netSocketAddress publicAddr;
	netSocketAddress localAddr;
	uint32_t newVal; // added in 372
	uint64_t rockstarAccountId; // 463/505
};

struct rlGamerInfo
{
	netPeerAddress peerAddress;
	uint64_t systemKey;
	uint64_t gamerId;
};

namespace rage
{
	class netPlayer
	{
	public:
		virtual ~netPlayer() = 0;

		virtual void Reset() = 0;

		virtual void m_10() = 0;

		virtual const char* GetName() = 0;

		virtual void m_20() = 0;

		virtual void m_28() = 0;

		virtual rlGamerInfo* GetGamerInfo() = 0;
	};
}

class CNetGamePlayer : public rage::netPlayer
{
public:
	virtual void m_38() = 0;

public:
	uint8_t pad[8];
	void* nonPhysicalPlayerData;
	uint8_t pad2[20];
	uint8_t activePlayerIndex;
	uint8_t physicalPlayerIndex;
	uint8_t pad3[2];
	uint8_t pad4[120];
	void* playerInfo;
};

class CNetworkPlayerMgr
{
public:
	static GTA_GAME_EXPORT CNetGamePlayer* GetPlayer(int playerIndex);
};
