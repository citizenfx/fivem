#include "StdInc.h"
#include "ResourceManager.h"
#include "ResourceScripting.h"

/*static int*& _scriptGlobals = *(int**)0x1849AEC;

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
}*/

LUA_FUNCTION(GetInvokingResource)
{
	lua_pushstring(L, GetInvokingEnvironment()->GetResource()->GetName().c_str());

	return 1;
}

LUA_FUNCTION(LoadScriptFile)
{
	std::string passedPath = luaL_checkstring(L, 1);
	std::string scriptName;
	
	int colon = passedPath.find(':');

	if (colon == std::string::npos)
	{
		passedPath = g_currentEnvironment->GetResource()->GetPath() + passedPath;
		scriptName = passedPath;
	}
	else
	{
		std::string resName = passedPath.substr(0, colon);
		std::string subPath = passedPath.substr(colon + 1);

		auto resource = TheResources.GetResource(resName);

		if (!resource.get())
		{
			lua_pushstring(L, va("No such resource in path %s.", passedPath.c_str()));
			lua_error(L);
		}

		scriptName = subPath;
		passedPath = resource->GetPath() + subPath;
	}

	if (!g_currentEnvironment->LoadFile(scriptName, passedPath))
	{
		lua_pushnil(L);
	}

	return 1;
}