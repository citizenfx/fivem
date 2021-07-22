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

#define DECLARE_ACCESSOR(x) \
	decltype(impl.m2372.x)& x()        \
	{                       \
		return (xbr::IsGameBuildOrGreater<2372>() ? impl.m2372.x : xbr::IsGameBuildOrGreater<2060>() ? impl.m2060.x : impl.m1604.x);   \
	} \
	const decltype(impl.m2372.x)& x() const                         \
	{                                                    \
		return (xbr::IsGameBuildOrGreater<2372>() ? impl.m2372.x : xbr::IsGameBuildOrGreater<2060>() ? impl.m2060.x : impl.m1604.x);  \
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

	union
	{
		Impl505 m1604;
		Impl2060 m2060;
		Impl2372 m2372;
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
using PeerAddress = std::conditional_t<(Build >= 2372), netPeerAddress::Impl2372, std::conditional_t<(Build >= 2060), netPeerAddress::Impl2060, netPeerAddress::Impl505>>;
