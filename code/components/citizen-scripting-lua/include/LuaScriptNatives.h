/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */
#pragma once

#include "StdInc.h"
#include "LuaScriptRuntime.h"

#include <lua.hpp>

int Lua_LoadNative(lua_State* L);

lua_CFunction Lua_GetNative(lua_State* L, const char* name);

int Lua_InvokeNative(lua_State* L);

#ifndef IS_FXSERVER
int Lua_GetNativeHandler(lua_State* L);

int Lua_InvokeNative2(lua_State* L);
#endif
