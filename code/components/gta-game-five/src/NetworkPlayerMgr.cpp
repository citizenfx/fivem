/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <NetworkPlayerMgr.h>

#include "Hooking.h"

static uint8_t vtableOffsetGetGamerInfo = 0;

static hook::cdecl_stub<CNetGamePlayer*(int)> getPlayerFromNetGame([] ()
{
	return hook::pattern("74 0A 83 F9 1F 77 05 E8 ? ? ? ? 48").count(1).get(0).get<void>(-12);
});

CNetGamePlayer* CNetworkPlayerMgr::GetPlayer(int playerIndex)
{
	return getPlayerFromNetGame(playerIndex);
}

static HookFunction hookFunction([]()
{
	vtableOffsetGetGamerInfo = *hook::get_pattern<uint8_t>("FF 52 ? 48 8B C8 E8 ? ? ? ? 48 8D 55 ? 48 8D 0D", 2);
});

void* rage::netPlayer::GetGamerInfoBase()
{
	uintptr_t vtable = *(uintptr_t*)this;
	uintptr_t vmethodAddress = *(uintptr_t*)(vtable + vtableOffsetGetGamerInfo);

	void* (*func)(void*);

	func = (decltype(func))vmethodAddress;

	return (void*)func(this);
}

