#pragma once

#include "lua.h"

#include "ComponentExport.h"

namespace fx
{

static constexpr uint8_t Lua_EACCES = 13;

#define LUA_FX_OSLIBNAME "os"
#if LUA_VERSION_NUM == 503
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA) int lua_fx_openos(lua_State *L);
#else
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA54) int lua_fx_openos(lua_State *L);
#endif

#define LUA_FX_IOLIBNAME "io"
#if LUA_VERSION_NUM == 503
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA) int lua_fx_openio(lua_State *L);
#else
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA54) int lua_fx_openio(lua_State *L);
#endif

#define LUA_FX_DEBUGLIBNAME "debug"
#if LUA_VERSION_NUM == 503
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA) int lua_fx_opendebug(lua_State *L);
#else
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA54) int lua_fx_opendebug(lua_State *L);
#endif
}
