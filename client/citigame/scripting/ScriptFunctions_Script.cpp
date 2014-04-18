#include "StdInc.h"
#include "ResourceScripting.h"

static int*& _scriptGlobals = *(int**)0x1849AEC;

LUA_FUNCTION(GetScriptGlobal)
{
	lua_pushinteger(L, _scriptGlobals[luaL_checkinteger(L, 1) / 4]);

	return 1;
}

LUA_FUNCTION(SetScriptGlobal)
{
	int global = luaL_checkinteger(L, 1) / 4;
	_scriptGlobals[global] = luaL_checkinteger(L, 2);

	return 0;
}