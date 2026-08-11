/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <NetworkPlayerMgr.h>

#include "Hooking.h"

static hook::cdecl_stub<CNetGamePlayer*(int)> getPlayerFromNetGame([] ()
{
	return hook::pattern("74 0A 83 F9 20 73 05 E8 ? ? ? ? 48").count(1).get(0).get<void>(-12);
});

CNetGamePlayer* CNetworkPlayerMgr::GetPlayer(int playerIndex)
{
	return getPlayerFromNetGame(playerIndex);
}
