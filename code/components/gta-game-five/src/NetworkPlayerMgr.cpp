/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <NetworkPlayerMgr.h>

#include "Hooking.h"
#include "Hooking.FlexStruct.h"

static uint8_t vtableOffsetGetGamerInfo = 0;
static uint32_t g_netGamePlayerNonPhysicalDataOffset = 0;
static uint32_t g_netGamePlayerActiveIndexOffset = 0;
static uint32_t g_netGamePlayerPhysicalIndexOffset = 0;
static uint32_t g_netGamePlayerInfoOffset = 0;
static uint32_t g_netGamePlayerClassSize = 0;

static hook::cdecl_stub<CNetGamePlayer*(int)> getPlayerFromNetGame([] ()
{
	return hook::pattern("74 0A 83 F9 1F 77 05 E8 ? ? ? ? 48").count(1).get(0).get<void>(-12);
});

CNetGamePlayer* CNetworkPlayerMgr::GetPlayer(int playerIndex)
{
	return getPlayerFromNetGame(playerIndex);
}

void*& CNetGamePlayer::nonPhysicalPlayerData()
{
	hook::FlexStruct* self = reinterpret_cast<hook::FlexStruct*>(this);
	return self->At<void*>(g_netGamePlayerNonPhysicalDataOffset);
}

uint8_t& CNetGamePlayer::activePlayerIndex()
{
	hook::FlexStruct* self = reinterpret_cast<hook::FlexStruct*>(this);
	return self->At<uint8_t>(g_netGamePlayerActiveIndexOffset);
}

uint8_t& CNetGamePlayer::physicalPlayerIndex()
{
	hook::FlexStruct* self = reinterpret_cast<hook::FlexStruct*>(this);
	return self->At<uint8_t>(g_netGamePlayerPhysicalIndexOffset);
}

void*& CNetGamePlayer::GetPlayerInfo()
{
	hook::FlexStruct* self = reinterpret_cast<hook::FlexStruct*>(this);
	return self->At<void*>(g_netGamePlayerInfoOffset);
}

size_t CNetGamePlayer::GetClassSize()
{
	return g_netGamePlayerClassSize;
}

static HookFunction hookFunction([]()
{
	vtableOffsetGetGamerInfo = *hook::get_pattern<uint8_t>("FF 52 ? 48 8B C8 E8 ? ? ? ? 48 8D 55 ? 48 8D 0D", 2);

	g_netGamePlayerClassSize = *hook::get_pattern<uint32_t>("48 81 C7 ? ? ? ? FF CD 79 ? 33 ED", 3);

	if (xbr::IsGameBuildOrGreater<xbr::Build::Summer_2025>())
	{
		g_netGamePlayerPhysicalIndexOffset = *hook::get_pattern<uint32_t>("8A 88 ? ? ? ? 80 F9 ? 74 ? BA", 2);
		g_netGamePlayerActiveIndexOffset = *hook::get_pattern<uint32_t>("41 8A 80 ? ? ? ? 3C ? 73", 3);
		g_netGamePlayerNonPhysicalDataOffset = *hook::get_pattern<uint32_t>("48 8B 89 ? ? ? ? 48 8B FA 48 85 C9 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 83 A3", 3);
	}
	else
	{
		g_netGamePlayerPhysicalIndexOffset = *hook::get_pattern<uint8_t>("8A 48 ? 80 F9 ? 74 ? BA", 2);
		g_netGamePlayerActiveIndexOffset = *hook::get_pattern<uint8_t>("41 80 78 ? ? 73", 3);
		g_netGamePlayerNonPhysicalDataOffset = *hook::get_pattern<uint8_t>("48 8B 49 ? 48 8B FA 48 85 C9 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 83 63", 3);
	}

	g_netGamePlayerInfoOffset = *hook::get_pattern<uint32_t>("4C 8B C7 33 D2 E8 ? ? ? ? 48 89 83", 13);
});

void* rage::netPlayer::GetGamerInfoBase()
{
	uintptr_t vtable = *(uintptr_t*)this;
	uintptr_t vmethodAddress = *(uintptr_t*)(vtable + vtableOffsetGetGamerInfo);

	void* (*func)(void*);

	func = (decltype(func))vmethodAddress;

	return (void*)func(this);
}

