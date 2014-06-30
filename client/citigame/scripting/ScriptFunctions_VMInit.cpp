// VM initialization script functions
#include "StdInc.h"
#include "ResourceScripting.h"
#include "ResourceManager.h"

LUA_FUNCTION(RegisterInitHandler)
{
	// reference the argument and hope it's callable
	lua_pushvalue(L, 1);
	g_currentEnvironment->SetInitHandler(luaL_ref(L, LUA_REGISTRYINDEX));

	return 0;
}

LUA_FUNCTION(SetResourceInfo)
{
	// expected environment: preparse
	g_currentEnvironment->GetResource()->SetMetaData(luaL_checkstring(L, 1), luaL_checkstring(L, 2));

	return 0;
}

LUA_FUNCTION(AddResourceDependency)
{
	// expected environment: preparse
	g_currentEnvironment->GetResource()->AddDependency(luaL_checkstring(L, 1));

	return 0;
}

LUA_FUNCTION(AddClientScript)
{
	std::string scriptName = luaL_checkstring(L, 1);
	std::string scriptPath = g_currentEnvironment->GetResource()->GetPath() + "/" + scriptName;

	g_currentEnvironment->DoFile(scriptName, scriptPath);

	return 0;
}

LUA_FUNCTION(AddResourceExport)
{
	g_currentEnvironment->GetResource()->AddExport(luaL_checkstring(L, 1));

	return 0;
}