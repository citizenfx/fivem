/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#include "StdInc.h"

#include <lua.hpp>

#include "LuaFXLib.h"

#include "LuaScriptRuntime.h"

namespace
{
/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
void checkstack(lua_State* L, lua_State* L1, int n)
{
	if (L != L1 && !lua_checkstack(L1, n))
	{
		luaL_error(L, "stack overflow");
	}
}

int db_getmetatable(lua_State* L)
{
	luaL_checkany(L, 1);
	if (!lua_getmetatable(L, 1))
	{
		lua_pushnil(L); /* no metatable */
	}
	return 1;
}

int db_setmetatable(lua_State* L)
{
	int t = lua_type(L, 2);
	luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
	              "nil or table expected");
	lua_settop(L, 2);
	lua_setmetatable(L, 1);
	return 1; /* return 1st argument */
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
lua_State* getthread(lua_State* L, int* arg)
{
	if (lua_isthread(L, 1))
	{
		*arg = 1;
		return lua_tothread(L, 1);
	}
	else
	{
		*arg = 0;
		return L; /* function will operate over current thread */
	}
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
void settabss(lua_State* L, const char* k, const char* v)
{
	lua_pushstring(L, v);
	lua_setfield(L, -2, k);
}

void settabsi(lua_State* L, const char* k, int v)
{
	lua_pushinteger(L, v);
	lua_setfield(L, -2, k);
}

void settabsb(lua_State* L, const char* k, int v)
{
	lua_pushboolean(L, v);
	lua_setfield(L, -2, k);
}


/*
** In function 'db_getinfo', the call to 'lua_getinfo' may push
** results on the stack; later it creates the result table to put
** these objects. Function 'treatstackoption' puts the result from
** 'lua_getinfo' on top of the result table so that it can call
** 'lua_setfield'.
*/
void treatstackoption(lua_State* L, lua_State* L1, const char* fname)
{
	if (L == L1)
		lua_rotate(L, -2, 1); /* exchange object and table */
	else
		lua_xmove(L1, L, 1); /* move object to the "main" stack */
	lua_setfield(L, -2, fname); /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
#if LUA_VERSION_NUM >= 504 && defined(_WIN32)
static int db_getinfo (lua_State *L)
{
	lua_Debug ar;
	int arg;
	lua_State *L1 = getthread(L, &arg);
	const char *options = luaL_optstring(L, arg+2, "flnSrtu");
	checkstack(L, L1, 3);
	luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
	if (lua_isfunction(L, arg + 1)) {  /* info about a function? */
		options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
		lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
		lua_xmove(L, L1, 1);
	}
	else {  /* stack level */
		if (!lua_getstack(L1, (int)luaL_checkinteger(L, arg + 1), &ar)) {
			luaL_pushfail(L);  /* level out of range */
			return 1;
		}
	}
	if (!lua_getinfo(L1, options, &ar))
		return luaL_argerror(L, arg+2, "invalid option");
	lua_newtable(L);  /* table to collect results */
	if (strchr(options, 'S')) {
		lua_pushlstring(L, ar.source, ar.srclen);
		lua_setfield(L, -2, "source");
		settabss(L, "short_src", ar.short_src);
		settabsi(L, "linedefined", ar.linedefined);
		settabsi(L, "lastlinedefined", ar.lastlinedefined);
		settabss(L, "what", ar.what);
	}
	if (strchr(options, 'l'))
		settabsi(L, "currentline", ar.currentline);
	if (strchr(options, 'u')) {
		settabsi(L, "nups", ar.nups);
		settabsi(L, "nparams", ar.nparams);
		settabsb(L, "isvararg", ar.isvararg);
	}
	if (strchr(options, 'n')) {
		settabss(L, "name", ar.name);
		settabss(L, "namewhat", ar.namewhat);
	}
	if (strchr(options, 'r')) {
		settabsi(L, "ftransfer", ar.ftransfer);
		settabsi(L, "ntransfer", ar.ntransfer);
	}
	if (strchr(options, 't'))
		settabsb(L, "istailcall", ar.istailcall);
	if (strchr(options, 'L'))
		treatstackoption(L, L1, "activelines");
	if (strchr(options, 'f'))
		treatstackoption(L, L1, "func");
	return 1;  /* return table */
}
#else
int db_getinfo(lua_State* L)
{
	lua_Debug ar;
	int arg;
	lua_State* L1 = getthread(L, &arg);
	const char* options = luaL_optstring(L, arg+2, "flnStu");
	checkstack(L, L1, 3);
	if (lua_isfunction(L, arg + 1))
	{
		/* info about a function? */
		options = lua_pushfstring(L, ">%s", options); /* add '>' to 'options' */
		lua_pushvalue(L, arg + 1); /* move function to 'L1' stack */
		lua_xmove(L, L1, 1);
	}
	else
	{
		/* stack level */
		if (!lua_getstack(L1, (int)luaL_checkinteger(L, arg + 1), &ar))
		{
			lua_pushnil(L); /* level out of range */
			return 1;
		}
	}
	if (!lua_getinfo(L1, options, &ar))
		return luaL_argerror(L, arg + 2, "invalid option");
	lua_newtable(L); /* table to collect results */
	if (strchr(options, 'S'))
	{
		settabss(L, "source", ar.source);
		settabss(L, "short_src", ar.short_src);
		settabsi(L, "linedefined", ar.linedefined);
		settabsi(L, "lastlinedefined", ar.lastlinedefined);
		settabss(L, "what", ar.what);
	}
	if (strchr(options, 'l'))
		settabsi(L, "currentline", ar.currentline);
	if (strchr(options, 'u'))
	{
		settabsi(L, "nups", ar.nups);
		settabsi(L, "nparams", ar.nparams);
		settabsb(L, "isvararg", ar.isvararg);
	}
	if (strchr(options, 'n'))
	{
		settabss(L, "name", ar.name);
		settabss(L, "namewhat", ar.namewhat);
	}
	if (strchr(options, 't'))
		settabsb(L, "istailcall", ar.istailcall);
	if (strchr(options, 'L'))
		treatstackoption(L, L1, "activelines");
	if (strchr(options, 'f'))
		treatstackoption(L, L1, "func");
	return 1; /* return table */
}
#endif


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
int auxupvalue(lua_State* L, int get)
{
	const char* name;
	const int n = static_cast<int>(luaL_checkinteger(L, 2)); /* upvalue index */
	luaL_checktype(L, 1, LUA_TFUNCTION); /* closure */
	name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
	if (name == nullptr) return 0;
	lua_pushstring(L, name);
	lua_insert(L, -(get+1)); /* no-op if get is false */
	return get + 1;
}

int db_getupvalue(lua_State* L)
{
	return auxupvalue(L, 1);
}

int db_traceback(lua_State* L)
{
	int arg;
	lua_State* L1 = getthread(L, &arg);
	const char* msg = lua_tostring(L, arg + 1);
	if (msg == nullptr && !lua_isnoneornil(L, arg + 1))
	{
		/* non-string 'msg'? */
		lua_pushvalue(L, arg + 1); /* return it untouched */
	}
	else
	{
		const int level = static_cast<int>(luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0));
		luaL_traceback(L, L1, msg, level);
	}

	return 1;
}

/*static int db_setcstacklimit (lua_State *L) {
  int limit = (int)luaL_checkinteger(L, 1);
  int res = lua_setcstacklimit(L, limit);
  lua_pushinteger(L, res);
  return 1;
}*/

const luaL_Reg dblib[] = {
	{"getinfo", db_getinfo},
	{"getmetatable", db_getmetatable},
	{"getupvalue", db_getupvalue},
	{"setmetatable", db_setmetatable},
	{"traceback", db_traceback},
	//{"setcstacklimit", db_setcstacklimit},
	{nullptr, nullptr}
};
}

namespace fx
{
int lua_fx_opendebug(lua_State* L)
{
	luaL_newlib(L, dblib);
	return 1;
}
}
