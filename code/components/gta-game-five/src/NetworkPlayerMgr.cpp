/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <NetworkPlayerMgr.h>

#include "Hooking.h"

static hook::cdecl_stub<CNetGamePlayer*(int)> getPlayerFromNetGame([] ()
{
	return hook::pattern("74 0A 83 F9 1F 77 05 E8 ? ? ? ? 48").count(1).get(0).get<void>(-12);
});

CNetGamePlayer* CNetworkPlayerMgr::GetPlayer(int playerIndex)
{
	return getPlayerFromNetGame(playerIndex);
}