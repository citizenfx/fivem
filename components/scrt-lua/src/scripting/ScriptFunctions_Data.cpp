#include "StdInc.h"
#include "ResourceScripting.h"

static std::unordered_map<std::string, fwString> g_gameData;

LUA_FUNCTION(SetGameData)
{
	int nargs = lua_gettop(L);

	luaS_serializeArgs(L, 2, nargs - 1);

	const char* jsonString = lua_tostring(L, -1);

	// store the entry in the game data structure
	const char* key = lua_tostring(L, 1);

	g_gameData[std::string(key)] = fwString(jsonString);

	// pop the lua string
	lua_pop(L, 1);

	return 0;
}

LUA_FUNCTION(GetGameData)
{
	const char* key = lua_tostring(L, 1);
	std::string keyStr = std::string(key);

	auto it = g_gameData.find(keyStr);

	if (it == g_gameData.end())
	{
		lua_pushnil(L);

		return 1;
	}
	
	int length;
	int table = luaS_deserializeArgs(L, &length, it->second);

	lua_remove(L, table);

	return length;
}

static InitFunction initFunction([] ()
{
	//ScriptEnvironment::SignalScriptReset.connect(&signalHandler, &GameDataInit::OnScriptReset);
	LuaScriptEnvironment::SignalScriptReset.Connect([] ()
	{
		g_gameData.clear();
	});
});