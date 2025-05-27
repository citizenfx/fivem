#pragma once

#include "lua.h"

#include "ComponentExport.h"

namespace fx
{
static constexpr uint8_t Lua_EACCES = 13;

#define LUA_FX_OSLIBNAME "os"
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA) int lua_fx_openos(lua_State *L);

#define LUA_FX_IOLIBNAME "io"
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA) int lua_fx_openio(lua_State *L);

#define LUA_FX_DEBUGLIBNAME "debug"
COMPONENT_EXPORT(CITIZEN_SCRIPTING_LUA) int lua_fx_opendebug(lua_State *L);
}
