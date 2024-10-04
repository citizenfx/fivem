/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */
#pragma once

#include "StdInc.h"
#include "LuaScriptRuntime.h"

#include <lua.hpp>

// we fast-path non-FXS using direct RAGE calls
#if defined(GTA_FIVE) || defined(IS_RDR3) || defined(GTA_NY)
#include <scrEngine.h>
#endif

#if defined(_DEBUG)
#define LUA_SCRIPT_LINKAGE LUA_API
#else
#define LUA_SCRIPT_LINKAGE static LUA_INLINE
#endif

#ifdef _WIN32
#define SAFE_BUFFERS __declspec(safebuffers)
#else
#define SAFE_BUFFERS
#endif

/// <summary>
/// </summary>
LUA_SCRIPT_LINKAGE int Lua_LoadNative(lua_State* L);

/// <summary>
/// </summary>
LUA_SCRIPT_LINKAGE lua_CFunction Lua_GetNative(lua_State* L, const char* name);

/// <summary>
/// </summary>
LUA_SCRIPT_LINKAGE int Lua_GetNativeHandler(lua_State* L);

/// <summary>
/// </summary>
LUA_SCRIPT_LINKAGE int SAFE_BUFFERS Lua_InvokeNative(lua_State* L);

#ifndef IS_FXSERVER
LUA_SCRIPT_LINKAGE int SAFE_BUFFERS Lua_InvokeNative2(lua_State* L);
#endif

