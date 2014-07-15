#include "StdInc.h"
#include "ResourceScripting.h"
#include "ResourceManager.h"

int lua_error_handler (lua_State *l);

void luaS_serializeArgs(lua_State* L, int firstArg, int numArgs)
{
	STACK_BASE;

	lua_pushcfunction(L, lua_error_handler);
	int eh = lua_gettop(L);

	// create a table with the arguments passed
	lua_createtable(L, numArgs, 0);

	int table = lua_gettop(L);

	// save the arguments in the table
	for (int i = firstArg; i < (firstArg + numArgs); i++)
	{
		lua_pushvalue(L, i);
		lua_rawseti(L, table, (i - firstArg) + 1);
	}

	// push c function and argument (for json decode)
	lua_getglobal(L, "msgpack");

	int jsonTable = lua_gettop(L);

	lua_pushstring(L, "pack");
	lua_gettable(L, -2);

	lua_remove(L, jsonTable);

	lua_pushvalue(L, table);

	// important parts on the stack should be 'json.encode', 'argument table' now

	if (lua_pcall(L, 1, 1, eh) != 0)
	{
		// error parsing json? what? we'll just push nil
		lua_pushstring(L, "[]");
	}

	lua_remove(L, table);
	lua_remove(L, eh);

	STACK_CHECK_N(1);
}

void luaS_serializeArgsJSON(lua_State* L, int firstArg, int numArgs)
{
	STACK_BASE;

	lua_pushcfunction(L, lua_error_handler);
	int eh = lua_gettop(L);

	// create a table with the arguments passed
	lua_createtable(L, numArgs, 0);

	int table = lua_gettop(L);

	// save the arguments in the table
	for (int i = firstArg; i < (firstArg + numArgs); i++)
	{
		lua_pushvalue(L, i);
		lua_rawseti(L, table, (i - firstArg) + 1);
	}

	// push c function and argument (for json decode)
	lua_getglobal(L, "json");

	int jsonTable = lua_gettop(L);

	lua_pushstring(L, "encode");
	lua_gettable(L, -2);

	lua_remove(L, jsonTable);

	lua_pushvalue(L, table);

	// important parts on the stack should be 'json.encode', 'argument table' now

	if (lua_pcall(L, 1, 1, eh) != 0)
	{
		// error parsing json? what? we'll just push nil
		lua_pushstring(L, "[]");
	}

	lua_remove(L, table);
	lua_remove(L, eh);

	STACK_CHECK_N(1);
}

int luaS_deserializeArgs(lua_State* L, int* numArgs, std::string& argsSerialized)
{
	STACK_BASE;

	lua_pushcfunction(L, lua_error_handler);
	int eh = lua_gettop(L);

	// stack: 1

	// push c function and argument (for json decode)
	lua_getglobal(L, "msgpack");

	int jsonTable = lua_gettop(L);

	lua_pushstring(L, "unpack");
	lua_gettable(L, -2);

	lua_remove(L, jsonTable);

	lua_pushlstring(L, argsSerialized.c_str(), argsSerialized.size());

	// stack: 3

	if (lua_pcall(L, 1, 1, eh) != 0)
	{
		// error parsing json? what? we'll just push nil
		lua_pushnil(L);
	}

	// stack: 3 (table on top)
	int type = lua_type(L, -1);
	//assert(!"NOTE: MSGPACK LIBRARY RETURNS OFFSET FIRST");
	assert(type == LUA_TTABLE);

	// store a ref to the table
	int table = lua_gettop(L);

	// iterate through the table
	int length = lua_objlen(L, table);

	*numArgs = length;

	for (int i = 1; i <= length; i++) // lua C API also involves 1-basedness
	{
		// note how this pushes too
		lua_rawgeti(L, table, i);
	}

	lua_remove(L, eh);

	STACK_CHECK_N(length + 1);

	return table - 1;
}

int luaS_deserializeArgsJSON(lua_State* L, int* numArgs, std::string& argsSerialized)
{
	STACK_BASE;

	lua_pushcfunction(L, lua_error_handler);
	int eh = lua_gettop(L);

	// stack: 1

	// push c function and argument (for json decode)
	lua_getglobal(L, "json");

	int jsonTable = lua_gettop(L);

	lua_pushstring(L, "decode");
	lua_gettable(L, -2);

	lua_remove(L, jsonTable);

	lua_pushlstring(L, argsSerialized.c_str(), argsSerialized.size());

	// stack: 3
	if (lua_pcall(L, 1, 1, eh) != 0)
	{
		// error parsing json? what? we'll just push nil
		lua_pushnil(L);
	}

	// stack: 3 (table on top)
	int type = lua_type(L, -1);
	assert(type == LUA_TTABLE);

	// store a ref to the table
	int table = lua_gettop(L);

	// iterate through the table
	int length = lua_objlen(L, table);

	*numArgs = length;

	for (int i = 1; i <= length; i++) // lua C API also involves 1-basedness
	{
		// note how this pushes too
		lua_rawgeti(L, table, i);
	}

	lua_remove(L, eh);

	STACK_CHECK_N(length + 1);

	return table - 1;
}

LUA_FUNCTION(GetFuncRef)
{
	lua_pushvalue(L, 1);
	int luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

	ResourceRef ref = g_currentEnvironment->GetRef(luaRef);

	lua_pushinteger(L, ref.reference);
	lua_pushstring(L, ref.resource->GetName().c_str());

	return 2;
}

std::shared_ptr<Resource> ValidateResourceAndRef(lua_State* L, int reference, std::string resourceName)
{
	auto resource = TheResources.GetResource(resourceName);

	if (!resource.get())
	{
		lua_pushstring(L, "no such resource");
		lua_error(L);
	}

	if (resource->GetState() != ResourceStateRunning && resource->GetState() != ResourceStateStarting)
	{
		lua_pushstring(L, "resource isn't running");
		lua_error(L);
	}

	if (!resource->HasRef(reference))
	{
		lua_pushstring(L, "invalid reference");
		lua_error(L);
	}

	return resource;
}

LUA_FUNCTION(GetFuncRefByData)
{
	const char* data = (const char*)lua_topointer(L, 1);

	if (data[0] == 1)
	{
		int reference = *(uint32_t*)(data + 1);
		std::string resourceName = &data[5];

		auto resource = ValidateResourceAndRef(L, reference, resourceName);

		lua_pushinteger(L, resource->DuplicateRef(reference));
		lua_pushstring(L, resource->GetName().c_str());
	}
	else
	{
		lua_pushstring(L, "invalid userdata");
		lua_error(L);
	}

	return 2;
}

LUA_FUNCTION(GetFuncFromRef)
{
	STACK_BASE;

	// get arguments
	int reference = luaL_checkinteger(L, 1);
	std::string resourceName = luaL_checkstring(L, 2);

	// get resource and validate it
	auto resource = ValidateResourceAndRef(L, reference, resourceName);

	// create the base metatable
	lua_createtable(L, 0, 2);

	int metatable = lua_gettop(L);

	STACK_CHECK_N(1);

	// create a gc function closure and set it
	lua_pushstring(L, "__gc");

	lua_pushinteger(L, reference);
	lua_pushstring(L, resourceName.c_str());

	STACK_CHECK_N(4);

	lua_pushcclosure(L, [] (lua_State* L)
	{
		// get and validate
		int reference = lua_tointeger(L, lua_upvalueindex(1));
		std::string resourceName = lua_tostring(L, lua_upvalueindex(2));

		auto resource = ValidateResourceAndRef(L, reference, resourceName);

		// and dereference the reference
		resource->RemoveRef(reference);

		return 0;
	}, 2);

	STACK_CHECK_N(3);

	lua_settable(L, metatable);

	STACK_CHECK_N(1);

	// create a call function closure too
	lua_pushstring(L, "__call");

	lua_pushinteger(L, reference);
	lua_pushstring(L, resourceName.c_str());

	lua_pushcclosure(L, [] (lua_State* L)
	{
		// get and validate
		int reference = lua_tointeger(L, lua_upvalueindex(1));
		std::string resourceName = lua_tostring(L, lua_upvalueindex(2));

		auto resource = ValidateResourceAndRef(L, reference, resourceName);

		STACK_BASE;

		// get number of arguments
		int nargs = lua_gettop(L);

		// stack: 0
		luaS_serializeArgs(L, 2, nargs - 1);
		int argTable = lua_gettop(L);

		size_t len;
		const char* jsonString = lua_tolstring(L, -1, &len);

		// call the function in the resource
		std::string retVal = resource->CallRef(reference, std::string(jsonString, len));

		lua_remove(L, argTable);

		// deserialize the return value
		int nArgs;
		int table = luaS_deserializeArgs(L, &nArgs, retVal);

		lua_remove(L, table);

		STACK_CHECK_N(nArgs);

		return nArgs;
	}, 2);

	lua_settable(L, metatable);

	// now create a dummy userdata
	STACK_CHECK_N(1);

	char* udata = (char*)lua_newuserdata(L, sizeof(uint32_t) + resourceName.length() + 2);

	// fill the udata with food
	*(uint8_t*)udata = 1;
	*(uint32_t*)(udata + 1) = reference;
	strcpy(udata + 2, resourceName.c_str());

	// and set the metatable
	lua_pushvalue(L, metatable);

	STACK_CHECK_N(3);

	lua_setmetatable(L, -2);

	STACK_CHECK_N(2);

	lua_remove(L, metatable);

	STACK_CHECK_N(1);

	return 1;
}
