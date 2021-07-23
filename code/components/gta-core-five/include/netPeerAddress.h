/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <CrossBuildRuntime.h>

#ifdef COMPILING_GTA_CORE_FIVE
#define GTA_CORE_EXPORT DLL_EXPORT
#else
#define GTA_CORE_EXPORT DLL_IMPORT
#endif

#define DECLARE_PEER_ACCESSOR(x) \
	decltype(Impl2372{}.x)& x()        \
	{                       \
		return (xbr::IsGameBuildOrGreater<2372>() ? ((Impls*)this)->m2372.x : xbr::IsGameBuildOrGreater<2060>() ? ((Impls*)this)->m2060.x : ((Impls*)this)->m1604.x);   \
	} \
	const decltype(Impl2372{}.x)& x() const                         \
	{                                                    \
		return (xbr::IsGameBuildOrGreater<2372>() ? ((Impls*)this)->m2372.x : xbr::IsGameBuildOrGreater<2060>() ? ((Impls*)this)->m2060.x : ((Impls*)this)->m1604.x);  \
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
		uint32_t secKeyTime;
		netSocketAddress relayAddr;
		netSocketAddress publicAddr;
		netSocketAddress localAddr;
		netSocketAddress unkAddr; // added in 2060
		uint32_t newVal;
		uint64_t rockstarAccountId;
	};

	struct Impl2372
	{
		char pad1[8];
		uint64_t rockstarAccountId;
		char pad22[8];
		uint64_t unkKey1;
		uint64_t unkKey2;
		char pad3[16];
		uint32_t secKeyTime;
		netSocketAddress publicAddr;
		netSocketAddress unkAddr;
		netSocketAddress localAddr;
		netSocketAddress relayAddr;
		uint32_t newVal;
	};

	union Impls
	{
		Impl505 m1604;
		Impl2060 m2060;
		Impl2372 m2372;
	};

public:
	DECLARE_PEER_ACCESSOR(unkKey1);
	DECLARE_PEER_ACCESSOR(unkKey2);
	DECLARE_PEER_ACCESSOR(secKeyTime);
	DECLARE_PEER_ACCESSOR(relayAddr);
	DECLARE_PEER_ACCESSOR(publicAddr);
	DECLARE_PEER_ACCESSOR(localAddr);
	DECLARE_PEER_ACCESSOR(newVal);
	DECLARE_PEER_ACCESSOR(rockstarAccountId);
};

template<int Build>
struct netPeerAddressStorage : netPeerAddress
{
};

template<>
struct netPeerAddressStorage<1604> : netPeerAddress
{
	uint8_t data[sizeof(Impl505)];
};

template<>
struct netPeerAddressStorage<2060> : netPeerAddress
{
	uint8_t data[sizeof(Impl2060)];
};

template<>
struct netPeerAddressStorage<2372> : netPeerAddress
{
	uint8_t data[sizeof(Impl2372)];
};

template<int Build>
using PeerAddress = std::conditional_t<(Build >= 2372), netPeerAddressStorage<2372>, std::conditional_t<(Build >= 2060), netPeerAddressStorage<2060>, netPeerAddressStorage<1604>>>;

static_assert(sizeof(PeerAddress<1604>) == 56);
static_assert(sizeof(PeerAddress<2060>) == 64);
static_assert(sizeof(PeerAddress<2372>) == 96);
