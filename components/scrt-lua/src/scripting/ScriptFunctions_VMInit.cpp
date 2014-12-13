/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

// VM initialization script functions
#include "StdInc.h"
#include "ResourceScripting.h"
#include "ResourceManager.h"

LUA_FUNCTION(RegisterInitHandler)
{
	// reference the argument and hope it's callable
	lua_pushvalue(L, 1);
	GetCurrentLuaEnvironment()->SetInitHandler(luaL_ref(L, LUA_REGISTRYINDEX));

	return 0;
}

LUA_FUNCTION(SetResourceInfo)
{
	// expected environment: preparse
	GetCurrentLuaEnvironment()->GetResource()->SetMetaData(luaL_checkstring(L, 1), luaL_checkstring(L, 2));

	return 0;
}

LUA_FUNCTION(AddResourceDependency)
{
	// expected environment: preparse
	GetCurrentLuaEnvironment()->GetResource()->AddDependency(luaL_checkstring(L, 1));

	return 0;
}

LUA_FUNCTION(AddClientScript)
{
	auto environment = GetCurrentLuaEnvironment();

	fwString scriptName = luaL_checkstring(L, 1);
	fwString scriptPath = environment->GetResource()->GetPath() + "/" + scriptName;

	environment->DoFile(scriptName, scriptPath);

	return 0;
}

LUA_FUNCTION(AddResourceExport)
{
	GetCurrentLuaEnvironment()->GetResource()->AddExport(luaL_checkstring(L, 1), GetCurrentLuaEnvironment());

	return 0;
}