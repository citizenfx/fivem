#include "StdInc.h"
#include "ResourceScripting.h"

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

	lua_pushstring(L, argsSerialized.c_str());

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