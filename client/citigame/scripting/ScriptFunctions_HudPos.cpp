#include "StdInc.h"
#include "ResourceScripting.h"
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