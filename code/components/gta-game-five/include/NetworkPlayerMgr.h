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
	decltype(impl.m2245.x)& x()        \
	{                       \
		return (xbr::IsGameBuildOrGreater<2245>() ? impl.m2245.x : xbr::IsGameBuildOrGreater<2060>() ? impl.m2060.x : impl.m1604.x);   \
	} \
	const decltype(impl.m2245.x)& x() const                         \
	{                                                    \
		return (xbr::IsGameBuildOrGreater<2245>() ? impl.m2245.x : xbr::IsGameBuildOrGreater<2060>() ? impl.m2060.x : impl.m1604.x);  \
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

	struct Impl2245
	{
		char pad1[8];
		uint64_t rockstarAccountId; // 463/505
		char pad22[8];
		uint64_t unkKey1;
		uint64_t unkKey2;
		char pad3[16];
		uint32_t secKeyTime; // added in 393
		netSocketAddress publicAddr;
		netSocketAddress unkAddr;
		netSocketAddress localAddr;
		netSocketAddress relayAddr;
		uint32_t newVal; // added in 372
	};

	union
	{
		Impl505 m1604;
		Impl2060 m2060;
		Impl2245 m2245;
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

// #TODO2372: cleanup
static_assert(offsetof(netPeerAddress::Impl2060, relayAddr) == 20);
static_assert(offsetof(netPeerAddress::Impl2060, publicAddr) == 28);
static_assert(offsetof(netPeerAddress::Impl2060, localAddr) == 36);
static_assert(offsetof(netPeerAddress::Impl2060, unkAddr) == 44);
static_assert(offsetof(netPeerAddress::Impl2060, newVal) == 52);
static_assert(offsetof(netPeerAddress::Impl2060, rockstarAccountId) == 56);

static_assert(offsetof(netPeerAddress::Impl2245, publicAddr) == 60);
static_assert(offsetof(netPeerAddress::Impl2245, unkAddr) == 68);
static_assert(offsetof(netPeerAddress::Impl2245, localAddr) == 76);
static_assert(offsetof(netPeerAddress::Impl2245, relayAddr) == 84);
static_assert(offsetof(netPeerAddress::Impl2245, newVal) == 92);

template<int Build>
using PeerAddress = std::conditional_t<(Build >= 2245), netPeerAddress::Impl2245, std::conditional_t<(Build >= 2060), netPeerAddress::Impl2060, netPeerAddress::Impl505>>;

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
		uint8_t pad2[8 + ExtraPad];
		uint8_t activePlayerIndex; // 1604: +44, 2060: +52, 2372: +32
		uint8_t physicalPlayerIndex;
		uint8_t pad3[2];
		uint8_t pad4[120];
		void* playerInfo;
	};

	union
	{
		Impl<12> m1604;
		Impl<20> m2060;
		Impl<0> m2245;
	} impl;

public:
	void* GetPlayerInfo()
	{
		return (xbr::IsGameBuildOrGreater<2060>()) ? impl.m2245.playerInfo : (xbr::IsGameBuildOrGreater<2060>()) ? impl.m2060.playerInfo : impl.m1604.playerInfo;
	}

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
