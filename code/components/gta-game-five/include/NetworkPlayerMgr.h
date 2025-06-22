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
#include <Hooking.h>

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
	class GTA_GAME_EXPORT netPlayer : XBR_VIRTUAL_BASE_2802(0)
	{
	public:
		//virtual ~netPlayer() = 0;
		// TODO: real dtors
		XBR_VIRTUAL_METHOD(void, Dtor, ())

		XBR_VIRTUAL_METHOD(void, Reset, ())

		XBR_VIRTUAL_METHOD(void, m_10, ())

		XBR_VIRTUAL_METHOD(const char*, GetName, ())

		void* GetGamerInfoBase();

		template<int Build>
		inline auto GetGamerInfo()
		{
			return (rlGamerInfo<Build>*)GetGamerInfoBase();
		}
	};
}

// using XBRVirt is safe here because it's right below so the counter increments right away
class GTA_GAME_EXPORT CNetGamePlayer : public rage::netPlayer
{
public:
	XBR_VIRTUAL_METHOD(void, m_38, ())

public:
	void*& nonPhysicalPlayerData();
	uint8_t& activePlayerIndex();
	uint8_t& physicalPlayerIndex();
	void*& GetPlayerInfo();
	static size_t GetClassSize();
};

class CNetworkPlayerMgr
{
public:
	static GTA_GAME_EXPORT CNetGamePlayer* GetPlayer(int playerIndex);
};

#undef DECLARE_ACCESSOR
