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

#define DECLARE_ACCESSOR(x) \
	decltype(impl.m1311.x)& x() \
	{ \
		return (impl.m1311.x); \
	} \
	const decltype(impl.m1311.x)& x() const \
	{ \
		return (impl.m1311.x); \
	}

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

		virtual void Reset() = 0; // Shutdown

		virtual void IsPhysical() = 0;

		virtual const char* GetLogName() = 0;

		virtual netPeerId* GetRlPeerId() = 0;

		virtual bool IsOnSameTeam(rage::netPlayer const& player) = 0;

		virtual void ActiveUpdate() = 0;

		virtual bool IsHost() = 0;

		virtual void m_unk1() = 0;

		virtual void m_unk2() = 0;

		virtual rlGamerInfo* GetGamerInfo() = 0;

	public:
		const char* GetName()
		{
			return GetGamerInfo()->name;
		}
	};
}

class CNetGamePlayer : public rage::netPlayer
{
private:
	struct Impl
	{
		uint8_t pad[16]; // +8
		uint8_t activePlayerIndex; // +24
		uint8_t physicalPlayerIndex; // +25
		char pad2[270]; // +26;
		void* pedPlayerComponent; // +296
	};

	union
	{
		Impl m1311;
	} impl;

public:
	void* GetPlayerInfo()
	{
		if (auto pedPlayerComponent = (void*)(impl.m1311.pedPlayerComponent))
		{
			return (void*)((char*)pedPlayerComponent + 0x130);
		}

		return nullptr;
	}

public:
	DECLARE_ACCESSOR(activePlayerIndex);
	DECLARE_ACCESSOR(physicalPlayerIndex);
};

class CNetworkPlayerMgr
{
public:
	static GTA_GAME_EXPORT CNetGamePlayer* GetPlayer(int playerIndex);
};
