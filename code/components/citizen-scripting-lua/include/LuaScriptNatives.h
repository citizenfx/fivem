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

// scrString corresponds to a binary string: may contain null-terminators, i.e,
// lua_pushlstring != lua_pushstring, and/or non-UTF valid characters.
#define SCRSTRING_MAGIC_BINARY 0xFEED1212

// we fast-path non-FXS using direct RAGE calls
#if defined(GTA_FIVE) || defined(IS_RDR3) || defined(GTA_NY)
#include <scrEngine.h>
#endif

#if defined(IS_FXSERVER) || defined(GTA_NY)
struct scrVector
{
	float x;

private:
	uint32_t pad;

public:
	float y;

private:
	uint32_t pad2;

public:
	float z;

private:
	uint32_t pad3;
};
#endif

struct scrObject
{
	const char* data;
	uintptr_t length;
};

struct scrString
{
	const char* str;
	size_t len;
	uint32_t magic;
};

struct scrVectorLua
{
	alignas(8) float x;
	alignas(8) float y;
	alignas(8) float z;

	scrVectorLua()
		: x(0.f), y(0.f), z(0.f)
	{
	}

	scrVectorLua(float x, float y, float z)
		: x(x), y(y), z(z)
	{
	}
};

template<bool IsPtr>
struct alignas(16) fxLuaNativeContext : fxNativeContext
{
	fxLuaNativeContext()
		: fxNativeContext()
	{
		memset(arguments, 0, sizeof(arguments));
		numArguments = 0;
		numResults = 0;
	}
};

#if defined(GTA_FIVE) || defined(IS_RDR3) || defined(GTA_NY)
template<>
struct alignas(16) fxLuaNativeContext<true> : NativeContextRaw
{
	int numArguments;
	alignas(16) uintptr_t arguments[32];

	fxLuaNativeContext()
		: NativeContextRaw(arguments, 0), numArguments(0)
	{
	}
};
#endif

typedef struct alignas(16) fxLuaResult
{
	fx::PointerField* pointerFields;

	int numReturnValues; // return values and their types
	uintptr_t retvals[16];
	LuaMetaFields rettypes[16];
	LuaMetaFields returnValueCoercion; // coercion for the result value
	bool returnResultAnyway; // flag to return a result even if a pointer return value is passed

	fxLuaResult(fx::PointerField* _fields)
		: pointerFields(_fields), numReturnValues(0), returnValueCoercion(LuaMetaFields::Max), returnResultAnyway(false)
	{
		auto* const __restrict rv = retvals;
		for (size_t i = 0; i < std::size(retvals); ++i)
		{
			rv[i] = 0;
		}

		auto* const __restrict rt = rettypes;
		for (size_t i = 0; i < std::size(rettypes); ++i)
		{
			rt[i] = LuaMetaFields::Max;
		}
	}
} fxLuaResult;

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

