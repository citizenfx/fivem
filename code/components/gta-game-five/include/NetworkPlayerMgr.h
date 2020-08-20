/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <CrossBuildRuntime.h>

#ifdef COMPILING_GTA_GAME_FIVE
#define GTA_GAME_EXPORT DLL_EXPORT
#else
#define GTA_GAME_EXPORT DLL_IMPORT
#endif

#define DECLARE_ACCESSOR(x) \
	decltype(impl.m2060.x)& x()        \
	{                       \
		return (Is2060() ? impl.m2060.x : impl.m1604.x);   \
	} \
	const decltype(impl.m2060.x)& x() const                         \
	{                                                    \
		return (Is2060() ? impl.m2060.x : impl.m1604.x);  \
	}

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
public:
	struct Impl505
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

	struct Impl2060
	{
		uint64_t unkKey1;
		uint64_t unkKey2;
		uint32_t secKeyTime; // added in 393
		netSocketAddress relayAddr;
		netSocketAddress publicAddr;
		netSocketAddress localAddr;
		netSocketAddress unkAddr; // added in 2060
		uint32_t newVal; // added in 372
		uint64_t rockstarAccountId; // 463/505
	};

	union
	{
		Impl505 m1604;
		Impl2060 m2060;
	} impl;

public:
	DECLARE_ACCESSOR(unkKey1);
	DECLARE_ACCESSOR(unkKey2);
	DECLARE_ACCESSOR(secKeyTime);
	DECLARE_ACCESSOR(relayAddr);
	DECLARE_ACCESSOR(publicAddr);
	DECLARE_ACCESSOR(localAddr);
	DECLARE_ACCESSOR(newVal);
	DECLARE_ACCESSOR(rockstarAccountId);
};

template<int Build>
using PeerAddress = std::conditional_t<(Build >= 2060), netPeerAddress::Impl2060, netPeerAddress::Impl505>;

template<int Build>
struct rlGamerInfo
{
	PeerAddress<Build> peerAddress;
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

		virtual void* GetGamerInfo_raw() = 0;

		template<int Build>
		inline auto GetGamerInfo()
		{
			return (rlGamerInfo<Build>*)GetGamerInfo_raw();
		}
	};
}

class CNetGamePlayer : public rage::netPlayer
{
public:
	virtual void m_38() = 0;

private:
	template<int ExtraPad>
	struct Impl
	{
		uint8_t pad[8];
		void* nonPhysicalPlayerData;
		uint8_t pad2[20 + ExtraPad];
		uint8_t activePlayerIndex;
		uint8_t physicalPlayerIndex;
		uint8_t pad3[2];
		uint8_t pad4[120];
		void* playerInfo;
	};

	union
	{
		Impl<0> m1604;
		Impl<8> m2060;
	} impl;

public:
	DECLARE_ACCESSOR(nonPhysicalPlayerData);
	DECLARE_ACCESSOR(activePlayerIndex);
	DECLARE_ACCESSOR(physicalPlayerIndex);
	DECLARE_ACCESSOR(playerInfo);
};

class CNetworkPlayerMgr
{
public:
	static GTA_GAME_EXPORT CNetGamePlayer* GetPlayer(int playerIndex);
};
