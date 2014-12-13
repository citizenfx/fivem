/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

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
	lua_pushstring(L, dynamic_cast<LuaScriptEnvironment*>(GetInvokingEnvironment())->GetResource()->GetName().c_str());

	return 1;
}

LUA_FUNCTION(LoadScriptFile)
{
	auto currentEnvironment = GetCurrentLuaEnvironment();

	fwString passedPath = luaL_checkstring(L, 1);
	fwString scriptName;
	
	int colon = passedPath.find(':');

	if (colon == std::string::npos)
	{
		passedPath = currentEnvironment->GetResource()->GetPath() + passedPath;
		scriptName = passedPath;
	}
	else
	{
		fwString resName = passedPath.substr(0, colon);
		fwString subPath = passedPath.substr(colon + 1);

		auto resource = TheResources.GetResource(resName);

		if (!resource.GetRef())
		{
			lua_pushstring(L, va("No such resource in path %s.", passedPath.c_str()));
			lua_error(L);
		}

		scriptName = subPath;
		passedPath = resource->GetPath() + subPath;
	}

	if (!currentEnvironment->LoadFile(scriptName, passedPath))
	{
		lua_pushnil(L);
	}

	return 1;
}