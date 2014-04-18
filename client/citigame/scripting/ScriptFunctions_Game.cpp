#include "StdInc.h"
#include "ResourceScripting.h"

LUA_FUNCTION(SetLoadingText)
{
	return 0;
}

#if 0
LUA_FUNCTION(SetLoadingText)
{
	mbstowcs((wchar_t*)(dwLoadOffset + 0x11D6550), luaL_checkstring(L, 1), 64);

	return 0;
}

LUA_FUNCTION(GetKeyboardDelays)
{
	int param;
	SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &param, 0);

	lua_pushnumber(L, (param * 250) + 250);

	SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &param, 0);

	lua_pushnumber(L, 1000.0 / (((param / 31.0) * 27.5) + 2.5));

	return 2;
}

const char* GetHostNameForGame(int idx);

LUA_FUNCTION(GetNetworkGameHostName)
{
	int gameIdx = luaL_checkinteger(L, 1);

	lua_pushstring(L, GetHostNameForGame(gameIdx));

	return 1;
}

#include "ScriptManager.h"

LUA_FUNCTION(GetPlayerNPID)
{
	CPlayerInfo* info;

	if (lua_type(L, 1) == LUA_TBOOLEAN)
	{
		info = GetPlayerInfo(lua_toboolean(L, 1) ? 1 : 0);
	}
	else
	{
		info = GetPlayerInfo(luaL_checkinteger(L, 1));
	}

	if (info == nullptr)
	{
		lua_pushinteger(L, 0);
		lua_pushinteger(L, 0);
	}
	else
	{
		NPID npID = *(NPID*)(info->address.abOnline);

		lua_pushinteger(L, npID & 0xFFFFFFFF);
		lua_pushinteger(L, npID >> 32);
	}

	return 2;
}

void* GetAvatarTexture(int i);
static __declspec(naked) void DrawGameSprite(void* texture, int dummy, float x, float y, float w, float h, float rot, unsigned char r, unsigned char g, unsigned char b, unsigned char a, int, int)
{
	__asm
	{
		mov eax, 0A6F3F0h
		jmp eax
	}
}

LUA_FUNCTION(DrawAvatarSprite)
{
	DrawGameSprite(GetAvatarTexture(luaL_checkinteger(L, 1)),
				   0,
				   (float)luaL_checknumber(L, 2),
				   (float)luaL_checknumber(L, 3),
				   (float)luaL_checknumber(L, 4),
				   (float)luaL_checknumber(L, 5),
				   (float)luaL_checknumber(L, 6),
				   luaL_checkinteger(L, 7),
				   luaL_checkinteger(L, 8),
				   luaL_checkinteger(L, 9),
				   luaL_checkinteger(L, 10),
				   0,
				   6);

	return 0;
}
#endif
