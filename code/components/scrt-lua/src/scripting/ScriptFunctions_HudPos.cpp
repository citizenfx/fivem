/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceScripting.h"

#if defined(GTA_NY)
#include "HudPos.h"

LUA_FUNCTION(GetHudPosition)
{
	auto hudPos = HudPositions::GetPosition(luaL_checkstring(L, 1));

	if (!hudPos)
	{
		return 0;
	}

	lua_pushnumber(L, hudPos->x);
	lua_pushnumber(L, hudPos->y); 

	return 2;
}

LUA_FUNCTION(GetHudSize)
{
	auto hudPos = HudPositions::GetPosition(luaL_checkstring(L, 1));

	if (!hudPos)
	{
		return 0;
	}

	lua_pushnumber(L, hudPos->w);
	lua_pushnumber(L, hudPos->h);

	return 2;
}
#endif