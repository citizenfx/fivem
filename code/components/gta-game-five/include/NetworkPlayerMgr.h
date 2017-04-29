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

 // needs to be kept in sync with gta:net:five
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

struct ScPlayerData
{
	ScInAddr addr;
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

		// contains ScInAddr as well
		virtual ScPlayerData* GetPlatformPlayerData() = 0;
	};
}

class CNetGamePlayer : public rage::netPlayer
{
public:
	virtual void m_38() = 0;
};

class CNetworkPlayerMgr
{
public:
	static GTA_GAME_EXPORT CNetGamePlayer* GetPlayer(int playerIndex);
};