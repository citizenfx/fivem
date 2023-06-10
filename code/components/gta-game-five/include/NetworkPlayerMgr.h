/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <CrossBuildRuntime.h>
#include <XBRVirtual.h>
#include <netPeerAddress.h>

#define DECLARE_ACCESSOR(x) \
	decltype(impl.m2372.x)& x()        \
	{                       \
		return (xbr::IsGameBuildOrGreater<2372>()) ? impl.m2372.x : (xbr::IsGameBuildOrGreater<2060>()) ? impl.m2060.x : impl.m1604.x;   \
	} \
	const decltype(impl.m2372.x)& x() const                         \
	{                                                    \
		return (xbr::IsGameBuildOrGreater<2372>()) ? impl.m2372.x : (xbr::IsGameBuildOrGreater<2060>()) ? impl.m2060.x : impl.m1604.x;  \
	}

#ifdef COMPILING_GTA_GAME_FIVE
#define GTA_GAME_EXPORT DLL_EXPORT
#else
#define GTA_GAME_EXPORT DLL_IMPORT
#endif

template<int Build>
struct rlGamerInfo
{
	PeerAddress<Build> peerAddress;
	uint64_t systemKey;
	uint64_t gamerId;
};

namespace rage
{
	class netPlayer : XBR_VIRTUAL_BASE_2802(0)
	{
	public:
		//virtual ~netPlayer() = 0;
		// TODO: real dtors
		XBR_VIRTUAL_METHOD(void, Dtor, ())

		XBR_VIRTUAL_METHOD(void, Reset, ())

		XBR_VIRTUAL_METHOD(void, m_10, ())

		XBR_VIRTUAL_METHOD(const char*, GetName, ())

		XBR_VIRTUAL_METHOD(void, m_20, ())

		XBR_VIRTUAL_METHOD(void, m_28, ())

		XBR_VIRTUAL_METHOD(void*, GetGamerInfo_raw, ())

		template<int Build>
		inline auto GetGamerInfo()
		{
			return (rlGamerInfo<Build>*)GetGamerInfo_raw();
		}
	};
}

// using XBRVirt is safe here because it's right below so the counter increments right away
class CNetGamePlayer : public rage::netPlayer
{
public:
	XBR_VIRTUAL_METHOD(void, m_38, ())

private:
	template<int ActiveIndexPad, int PlayerInfoPad, int EndPad>
	struct Impl
	{
		uint8_t pad[8];
		void* nonPhysicalPlayerData;
		uint8_t pad2[8 + ActiveIndexPad];
		uint8_t activePlayerIndex; // 1604: +44, 2060: +52, 2372: +32
		uint8_t physicalPlayerIndex;
		uint8_t pad3[2];
		uint8_t pad4[120 + PlayerInfoPad];
		void* playerInfo; // 1604: +148, 2060: +176, 2372: +160
		char end[EndPad];
	};

	union
	{
		Impl<12, 0, 28> m1604;
		Impl<20, 0, 0> m2060;
		Impl<0, 4, 16> m2372;
	} impl;

public:
	void* GetPlayerInfo()
	{
		return (xbr::IsGameBuildOrGreater<2372>()) ? impl.m2372.playerInfo : (xbr::IsGameBuildOrGreater<2060>()) ? impl.m2060.playerInfo : impl.m1604.playerInfo;
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
