/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <fxScripting.h>
#include <Error.h>

#include <vector>
#include <map>

#include <ResourceManager.h>
#include <Profiler.h>

#include "LuaScriptNatives.h"
#include "LuaScriptRuntime.h"

#include <shared_mutex>

extern LUA_INTERNAL_LINKAGE
{
#include <lobject.h>
#include <lapi.h>
#if LUA_VERSION_NUM == 504
#include <lstate.h>
#include <lglm.hpp>
#include <lglmlib.hpp>
#endif
}

using namespace fx::invoker;

extern bool g_hadProfiler;

// Get the pointer to a value local to the current lua function (idx must be > 0)
static CSCRC_INLINE const TValue* LuaGetLocal(lua_State* L, int idx)
{
#if LUA_VERSION_NUM == 504
	const CallInfo* ci = L->ci;
	const StkId o = ci->func + idx;
	api_check(L, idx <= ci->top - (ci->func + 1), "unacceptable index");
	return (o >= L->top) ? &G(L)->nilvalue : s2v(o);
#else
	const CallInfo* ci = L->ci;
	const TValue* o = ci->func + idx;
	api_check(L, idx <= ci->top - (ci->func + 1), "unacceptable index");
	return (o >= L->top) ? luaO_nilobject : o;
#endif
}

struct LuaScriptNativeContext final : ScriptNativeContext
{
	LuaScriptNativeContext(lua_State* L, const ScriptNativeHandler& handler);

	void PushArgument(int idx);
	void PushTableArgument(int idx);

	template<typename T>
	void ProcessResult(T value);

	lua_State* const L;
};

LuaScriptNativeContext::LuaScriptNativeContext(lua_State* L, const ScriptNativeHandler& handler)
	: ScriptNativeContext(handler), L(L)
{
}

CSCRC_INLINE void LuaScriptNativeContext::PushArgument(int idx)
{
	const TValue* value = LuaGetLocal(L, idx);
	size_t slot = ReserveArgs(1); // Check we can push at least 1 argument

#if LUA_VERSION_NUM == 504
	switch (ttypetag(value))
	{
		case LUA_VNIL:
		case LUA_VFALSE:
			PushAt(slot, 0);
			break;

		case LUA_VTRUE:
			PushAt(slot, 1);
			break;

		case LUA_VNUMINT:
			PushAt(slot, ivalue(value));
			break;

		case LUA_VNUMFLT:
			PushAt(slot, fltvalue(value));
			break;

		case LUA_VSHRSTR:
#if defined(GRIT_POWER_BLOB)
		case LUA_VBLOBSTR:
#endif
		case LUA_VLNGSTR:
			PushAt(slot, svalue(value), vslen(value));
			break;

		case LUA_VVECTOR2:
		{
			slot = ReserveArgs(2);
			const auto& v = vvalue(value);
			PushAt(slot + 0, v.v4.x);
			PushAt(slot + 1, v.v4.y);
			break;
		}

		case LUA_VVECTOR3:
		{
			slot = ReserveArgs(3);
			const auto& v = vvalue(value);
			PushAt(slot + 0, v.v4.x);
			PushAt(slot + 1, v.v4.y);
			PushAt(slot + 2, v.v4.z);
			break;
		}

		case LUA_VVECTOR4:
		{
			slot = ReserveArgs(4);
			const auto& v = vvalue(value);
			PushAt(slot + 0, v.v4.x);
			PushAt(slot + 1, v.v4.y);
			PushAt(slot + 2, v.v4.z);
			PushAt(slot + 3, v.v4.w);
			break;
		}

		case LUA_VQUAT: // Support (NOT) GLM_FORCE_QUAT_DATA_XYZW
		{
			slot = ReserveArgs(4);
			const auto& v = vvalue(value);
			PushAt(slot + 0, v.q.x);
			PushAt(slot + 1, v.q.y);
			PushAt(slot + 2, v.q.z);
			PushAt(slot + 3, v.q.w);
			break;
		}

		case LUA_VTABLE: // table (high-level class with __data field)
			PushTableArgument(idx);
			break;

		case LUA_VLIGHTUSERDATA:
			PushMetaPointer(static_cast<uint8_t*>(pvalue(value)));
			break;

		default:
		{
			ScriptErrorf("invalid lua type: %s", lua_typename(L, ttype(value)));
		}
	}
#else
	switch (ttype(value))
	{
		case LUA_TNIL:
			PushAt(slot, 0);
			break;

		case LUA_TBOOLEAN:
			PushAt(slot, bvalue(value));
			break;

		case LUA_TNUMINT:
			PushAt(slot, ivalue(value));
			break;

		case LUA_TNUMFLT:
			PushAt(slot, fltvalue(value));
			break;

		case LUA_TSHRSTR:
			PushAt(slot, svalue(value), tsvalue(value)->shrlen);
			break;

		case LUA_TLNGSTR:
			PushAt(slot, svalue(value), tsvalue(value)->u.lnglen);
			break;

		case LUA_TVECTOR2:
		{
			slot = ReserveArgs(2);
			const auto& v = v2value(value);
			PushAt(slot + 0, v.x);
			PushAt(slot + 1, v.y);
			break;
		}

		case LUA_TVECTOR3:
		{
			slot = ReserveArgs(3);
			const auto& v = v3value(value);
			PushAt(slot + 0, v.x);
			PushAt(slot + 1, v.y);
			PushAt(slot + 2, v.z);
			break;
		}

		case LUA_TVECTOR4:
		{
			slot = ReserveArgs(4);
			const auto& v = v4value(value);
			PushAt(slot + 0, v.x);
			PushAt(slot + 1, v.y);
			PushAt(slot + 2, v.z);
			PushAt(slot + 3, v.w);
			break;
		}

		case LUA_TQUAT:
		{
			slot = ReserveArgs(4);
			const auto& v = qvalue(value);
			PushAt(slot + 0, v.x);
			PushAt(slot + 1, v.y);
			PushAt(slot + 2, v.z);
			PushAt(slot + 3, v.w);
			break;
		}

		case LUA_TTABLE: // table (high-level class with __data field)
			PushTableArgument(idx);
			break;

		case LUA_TLIGHTUSERDATA:
			PushMetaPointer(static_cast<uint8_t*>(pvalue(value)));
			break;

		default:
			ScriptErrorf("invalid lua type: %s", lua_typename(L, ttnov(value)));
	}
#endif
}

// table parsing implementation
void LuaScriptNativeContext::PushTableArgument(int idx)
{
	const int t_idx = lua_absindex(L, idx);
	luaL_checkstack(L, 2, "table arguments");
	lua_pushliteral(L, "__data");

	// get the type and decide what to do based on it
	auto validType = [](int t)
	{
#if LUA_VERSION_NUM == 504
		return t == LUA_TBOOLEAN || t == LUA_TNUMBER || t == LUA_TSTRING || t == LUA_TVECTOR;
#else
		return t == LUA_TBOOLEAN || t == LUA_TNUMBER || t == LUA_TSTRING || t == LUA_TVECTOR2 || t == LUA_TVECTOR3 || t == LUA_TVECTOR4 || t == LUA_TQUAT;
#endif
	};

	if (validType(lua_rawget(L, t_idx))) // Account for pushstring if idx < 0
	{
		PushArgument(lua_gettop(L));
		lua_pop(L, 1);
	}
	else
	{
		lua_pop(L, 1); // [...]
		if (luaL_getmetafield(L, idx, "__data") == LUA_TFUNCTION) // [..., metafield]
		{
			// The __data function can only allow one return value (no LUA_MULTRET)
			// to avoid additional implicitly expanded types during native execution.
			lua_pushvalue(L, t_idx); // [..., function, argument]
			lua_call(L, 1, 1); // [..., value]
		}

		if (validType(lua_type(L, -1)))
		{
			PushArgument(lua_gettop(L));
			lua_pop(L, 1); // [...]
		}
		else
		{
			lua_pop(L, 1);
			ScriptError("invalid lua type in __data");
		}
	}
}

template<typename T>
CSCRC_INLINE void LuaScriptNativeContext::ProcessResult(T value)
{
	if constexpr (std::is_same_v<T, bool>)
	{
		lua_pushboolean(L, value);
	}
	else if constexpr (std::is_same_v<T, int32_t>)
	{
		lua_pushinteger(L, value);
	}
	else if constexpr (std::is_same_v<T, int64_t>)
	{
		lua_pushinteger(L, value);
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		lua_pushnumber(L, value);
	}
	else if constexpr (std::is_same_v<T, ScrVector>)
	{
#if LUA_VERSION_NUM == 504
		glm_pushvec3(L, glm::vec<3, glm_Float>(value.x, value.y, value.z));
#else
		lua_pushvector3(L, value.x, value.y, value.z);
#endif
	}
	else if constexpr (std::is_same_v<T, const char*>)
	{
		lua_pushstring(L, value);
	}
	else if constexpr (std::is_same_v<T, ScrString>)
	{
		lua_pushlstring(L, value.str, value.len);
	}
	else if constexpr (std::is_same_v<T, ScrObject>)
	{
		fx::LuaScriptRuntime::GetCurrent()->ResultAsObject(L, std::string_view{ value.data, value.length });
	}
	else
	{
		static_assert(always_false_v<T>, "Invalid return type");
	}
}

static int Lua_DoInvokeNative(lua_State* L, int arg_base, const ScriptNativeHandler& handler)
{
	LuaScriptNativeContext context(L, handler);

	for (int arg = arg_base, nargs = lua_gettop(L); arg <= nargs; ++arg)
	{
		context.PushArgument(arg);
	}

	context.Invoke();

	int nrets = lua_gettop(L);

	context.ProcessResults([&](auto value)
	{
		context.ProcessResult(value);
	});

	nrets = lua_gettop(L) - nrets;

	return nrets;
}

// Invoke a native, with the hash passed in the first argument
int Lua_InvokeNative(lua_State* L)
{
	try
	{
		uint64_t hash = lua_tointeger(L, 1);

		return Lua_DoInvokeNative(L, 2, ScriptNativeHandler::FromHash(hash));
	}
	catch (const std::exception& ex)
	{
		lua_pushstring(L, ex.what());
	}

	// Throw the error after unwinding the stack
	return lua_error(L);
}

#ifndef IS_FXSERVER
// Invoke a native, with the index passed in the first upvalue of the first argument
// Deprecated, GetNative(Handler) now returns a function to invoke directly
int Lua_InvokeNative2(lua_State* L)
{
	try
	{
		luaL_argcheck(L, (lua_getupvalue(L, 1, 1) != NULL), 1, "invalid native handler");

		size_t index = luaL_checkinteger(L, -1);
		lua_pop(L, 1);

		return Lua_DoInvokeNative(L, 2, ScriptNativeHandler::FromCacheIndex(index));
	}
	catch (const std::exception& ex)
	{
		lua_pushstring(L, ex.what());
	}

	// Throw the error after unwinding the stack
	return lua_error(L);
}

#ifdef GTA_FIVE
static int Lua_GetHashKey(lua_State* L)
{
	// if NULL or an integer, return 0
	if (lua_isnil(L, 1) || lua_type(L, 1) == LUA_TNUMBER)
	{
		lua_pushinteger(L, 0);
	}
	else
	{
		const char* str = luaL_checkstring(L, 1);
		lua_pushinteger(L, static_cast<lua_Integer>(static_cast<int32_t>(HashString(str))));
	}

	return 1;
}
#endif

static int Lua_InvokeNativeHandler(lua_State* L)
{
	try
	{
		size_t index = lua_tointeger(L, lua_upvalueindex(1));

		return Lua_DoInvokeNative(L, 1, ScriptNativeHandler::FromCacheIndex(index));
	}
	catch (const std::exception& ex)
	{
		lua_pushstring(L, ex.what());
	}

	// Throw the error after unwinding the stack
	return lua_error(L);
};

int Lua_GetNativeHandler(lua_State* L)
{
	uint64_t hash = lua_tointeger(L, 1); // TODO: Use luaL_checkinteger

	// TOOD: Store the handler pointer directly? Not sure if we can trust userdata
	size_t index = ScriptNativeHandler::FromHash(hash).cache_index;
	lua_pushinteger(L, index);

	lua_CFunction handler = Lua_InvokeNativeHandler;

#ifdef GTA_FIVE
	// super fast path for 323 GET_HASH_KEY in GTA
	if (hash == 0xD24D37CC275948CC)
	{
		handler = Lua_GetHashKey;
	}
#endif

	lua_pushcclosure(L, handler, 1);
	return 1;
}

#endif

#pragma region Lua_LoadNative

int Lua_LoadNative(lua_State* L)
{
	const char* fn = luaL_checkstring(L, 1);
	auto fnHash = HashRageString(fn);
	auto& runtime = fx::LuaScriptRuntime::GetCurrent();
	auto& nonExistentNatives = runtime->GetNonExistentNativesList();

	{
		if (nonExistentNatives.find(fnHash) != nonExistentNatives.end())
		{
			lua_pushnil(L);
			return 1;
		}
	}

	try
	{
		int isCfxv2 = 0;
		runtime->GetScriptHost2()->GetNumResourceMetaData("is_cfxv2", &isCfxv2);

		if (isCfxv2) // TODO/TEMPORARY: fxv2 oal is disabled by default for now
		{
			runtime->GetScriptHost2()->GetNumResourceMetaData("use_experimental_fxv2_oal", &isCfxv2);
		}

		// #if !defined(GTA_FIVE) || (LUA_VERSION_NUM == 504)
		if (isCfxv2)
		// #endif
		{
			auto nativeImpl = Lua_GetNative(L, fn);

			if (nativeImpl)
			{
				lua_pushcfunction(L, nativeImpl);
				return 1;
			}
		}

		fx::OMPtr<fxIStream> stream;

		result_t hr = runtime->GetScriptHost()->OpenSystemFile(const_cast<char*>(va("%s0x%08x.lua", runtime->GetNativesDir(), fnHash)), stream.GetAddressOf());

		auto invalid = [&nonExistentNatives, fnHash]()
		{
			nonExistentNatives.insert(fnHash);
		};

		if (!FX_SUCCEEDED(hr))
		{
			invalid();

			lua_pushnil(L);
			return 1;
		}

		// read file data
		uint64_t length;

		if (FX_FAILED(hr = stream->GetLength(&length)))
		{
			invalid();

			lua_pushnil(L);
			return 1;
		}

		std::vector<char> fileData(length + 1);
		if (FX_FAILED(hr = stream->Read(&fileData[0], length, nullptr)))
		{
			invalid();

			lua_pushnil(L);
			return 1;
		}

		fileData[length] = '\0';

		lua_pushlstring(L, fileData.data(), length);

		return 1;
	}

	// @TODO: Fix memory leak due to interop
	catch (const std::exception& stl_e)
	{
		lua_pushstring(L, stl_e.what()); // may LUAI_THROW
		return lua_error(L);
	}
	catch (...)
	{
		lua_pushliteral(L, "Unknown exception handled!");
		return lua_error(L);
	}
}

#pragma endregion

#pragma region LuaGetNative

#include <lua_cmsgpacklib.h>
#define sc_nvalue(o, T) (ttisinteger(o) ? static_cast<T>(ivalue(o)) : static_cast<T>(fltvalue(o)))

using Lua_NativeMap = std::map<std::string, lua_CFunction, std::less<>>;

using scrVectorLua = ScrVector;
using scrObject = ScrObject;
using scrString = ScrString;

struct LuaArgumentParser
{
	// Parsing function arguments.
	template<typename T, size_t S = sizeof(T)>
	static LUA_INLINE T ParseArgument(lua_State* L, int idx)
	{
		static_assert(sizeof(T) == 0, "Invalid ParseArgument");
	}

	template<typename T, size_t S = sizeof(T)>
	static LUA_INLINE void PushObject(lua_State* L, const T val)
	{
		static_assert(sizeof(T) == 0, "Invalid PushObject");
	}

	template<typename T>
	static LUA_INLINE T ParseInteger(lua_State* L, int idx)
	{
		const TValue* value = LuaGetLocal(L, idx);

		if (ttisinteger(value))
		{
			return static_cast<T>(ivalue(value));
		}
		else if (ttisnumber(value))
		{
			return static_cast<T>(fltvalue(value));
		}

		return (!l_isfalse(value) ? 1 : 0);
	}

	static LUA_INLINE const char* ParseFunctionReference(lua_State* L, int idx)
	{
		return lua_tostring(L, idx); // @TODO: maybe?
	}

	static LUA_INLINE const char* ParseObject(lua_State* L, int idx)
	{
		lua_pushcfunction(L, mp_pack); // [..., idx, ..., mp_pack]
		lua_pushvalue(L, idx); // [..., idx, ..., mp_pack, object]
		lua_call(L, 1, 1); // [..., idx, ..., encoding]
		lua_replace(L, idx); // [..., encoding, ...]
		return lua_tostring(L, idx);
	}

	static LUA_INLINE void PushStringObject(lua_State* L, const char* val, size_t len)
	{
		lua_pushlstring(L, val, len);
	}
};

template<>
LUA_INLINE bool LuaArgumentParser::ParseArgument<bool>(lua_State* L, int idx)
{
	const TValue* value = LuaGetLocal(L, idx);
	if (ttisinteger(value))
		return ivalue(value) != 0;
	return l_isfalse(value) ? false : true;
}

template<>
LUA_INLINE float LuaArgumentParser::ParseArgument<float>(lua_State* L, int idx)
{
	const TValue* value = LuaGetLocal(L, idx);
	return ttisnumber(value) ? sc_nvalue(value, float) : 0.f;
}

template<>
LUA_INLINE double LuaArgumentParser::ParseArgument<double>(lua_State* L, int idx)
{
	const TValue* value = LuaGetLocal(L, idx);
	return ttisnumber(value) ? sc_nvalue(value, double) : 0.0;
}

template<>
LUA_INLINE int16_t LuaArgumentParser::ParseArgument<int16_t>(lua_State* L, int idx)
{
	return ParseInteger<int16_t>(L, idx);
}

template<>
LUA_INLINE int32_t LuaArgumentParser::ParseArgument<int32_t>(lua_State* L, int idx)
{
	return ParseInteger<int32_t>(L, idx);
}

template<>
LUA_INLINE int64_t LuaArgumentParser::ParseArgument<int64_t>(lua_State* L, int idx)
{
	return ParseInteger<int64_t>(L, idx);
}

#if defined(__GNUC__)
template<>
LUA_INLINE lua_Integer LuaArgumentParser::ParseArgument<lua_Integer>(lua_State* L, int idx)
{
	return ParseInteger<lua_Integer>(L, idx);
}
#endif

/// <summary>
/// Codegen uses unsigned integer types to denote hashes.
/// </summary>
template<>
LUA_INLINE uint32_t LuaArgumentParser::ParseArgument<uint32_t>(lua_State* L, int idx)
{
	const TValue* o = LuaGetLocal(L, idx);
#if LUA_VERSION_NUM == 504
	if (ttisstring(o))
		return HashString(svalue(o));
	else if (ttisinteger(o))
		return ivalue(o);
	return 0;
#else
	if (lua_valuetype(L, o) == LUA_TSTRING)
	{
		return HashString(lua_valuetostring(L, o));
	}

	return lua_valuetointeger(L, o);
#endif
}

/// <summary>
/// _ts: Workaround for users calling string parameters with '0' and also nil
/// being translated.
///
/// @NOTE lua_tolstring also changes the actual value in the stack to a string
/// </summary>
template<>
LUA_INLINE const char* LuaArgumentParser::ParseArgument<const char*>(lua_State* L, int idx)
{
	const TValue* value = LuaGetLocal(L, idx);
	switch (ttype(value))
	{
		case LUA_TNIL:
			return NULL;
		case LUA_TSTRING:
			return svalue(value);
		case LUA_TNUMBER:
		{
			if ((ttisinteger(value) && ivalue(value) == 0)
				|| (ttisnumber(value) && nvalue(value) == 0.0))
				return NULL;

			return lua_tostring(L, idx);
		}
		default:
		{
			return lua_tostring(L, idx);
		}
	}
}

template<>
LUA_INLINE scrVectorLua LuaArgumentParser::ParseArgument<scrVectorLua>(lua_State* L, int idx)
{
	const TValue* o = LuaGetLocal(L, idx);
#if LUA_VERSION_NUM == 504
	if (ttisvector(o))
	{
		const glmVector& v = vvalue(o);
		if (ttisquat(o)) // Support (NOT) GLM_FORCE_QUAT_DATA_XYZW
			return scrVectorLua{ v.q.x, v.q.y, v.q.z };
		else
			return scrVectorLua{ v.v4.x, v.v4.y, v.v4.z };
	}
	return scrVectorLua{ 0.f, 0.f, 0.f };
#else
	auto f4 = lua_valuetofloat4(L, o);
	return scrVectorLua{ f4.x, f4.y, f4.z };
#endif
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<bool>(lua_State* L, bool val)
{
	lua_pushboolean(L, val);
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<float>(lua_State* L, float val)
{
	lua_pushnumber(L, static_cast<lua_Number>(val));
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<double>(lua_State* L, double val)
{
	lua_pushnumber(L, static_cast<lua_Number>(val));
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<int32_t>(lua_State* L, int32_t val)
{
	lua_pushinteger(L, static_cast<lua_Integer>(val));
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<int64_t>(lua_State* L, int64_t val)
{
	lua_pushinteger(L, static_cast<lua_Integer>(val));
}

#if defined(__GNUC__)
template<>
LUA_INLINE void LuaArgumentParser::PushObject<lua_Integer>(lua_State* L, lua_Integer val)
{
	lua_pushinteger(L, static_cast<lua_Integer>(val));
}
#endif

template<>
LUA_INLINE void LuaArgumentParser::PushObject<uint32_t>(lua_State* L, uint32_t val)
{
	lua_pushinteger(L, static_cast<lua_Integer>(val));
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<uint64_t>(lua_State* L, uint64_t val)
{
	lua_pushinteger(L, static_cast<lua_Integer>(val));
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<const char*>(lua_State* L, const char* val)
{
	lua_pushstring(L, val);
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<const scrVectorLua&>(lua_State* L, const scrVectorLua& val)
{
#if LUA_VERSION_NUM == 504
	glm_pushvec3(L, glm::vec<3, glm_Float>(val.x, val.y, val.z));
#else
	lua_pushvector3(L, val.x, val.y, val.z);
#endif
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<const scrObject&>(lua_State* L, const scrObject& val)
{
	// @NOTE: Prevent scripts that override msgpack.unpack() from
	// manipulating data internally.
	lua_pushcfunction(L, mp_unpack_compat);
	lua_pushlstring(L, val.data, val.length);
	lua_call(L, 1, 1);
}

template<>
LUA_INLINE void LuaArgumentParser::PushObject<const scrString&>(lua_State* L, const scrString& val)
{
	if (val.magic == ScrString::Signature)
		lua_pushlstring(L, val.str, val.len);
	else if (val.str)
		lua_pushstring(L, val.str);
	else
		lua_pushnil(L);
}

#if defined(IS_FXSERVER)
#define LUA_EXC_WRAP_START(hash)
#define LUA_EXC_WRAP_END(hash)
#define ASSERT_LUA_ARGS(count)    \
	if (!lua_asserttop(L, count)) \
		return 0;

// TODO: Remove this!
struct LuaNativeWrapper
{
	LUA_INLINE LuaNativeWrapper(uint64_t)
	{
	}
};

struct LuaNativeContext : public fxNativeContext
{
	LUA_INLINE LuaNativeContext(void*, int numArguments)
	{
		this->numArguments = numArguments;
		numResults = 0;
	}

	LUA_INLINE void Invoke(lua_State* L, uint64_t hash)
	{
		nativeIdentifier = hash;

		IScriptHost* lastScriptHost = fx::LuaScriptRuntime::GetLastHost();
		if (lastScriptHost == nullptr || FX_FAILED(lastScriptHost->InvokeNative(*this)))
		{
			lua_pushliteral(L, "Native invocation failed.");
			lua_error(L);
		}
	}

	template<typename TVal>
	LUA_INLINE TVal GetResult()
	{
		return *(TVal*)(&arguments[0]);
	}

	template<typename TVal>
	LUA_INLINE void SetArgument(size_t offset, const TVal& val)
	{
		LUA_IF_CONSTEXPR(sizeof(TVal) < 4)
		{
			*reinterpret_cast<uintptr_t*>(&arguments[offset]) = 0;
		}

		*reinterpret_cast<TVal*>(&arguments[offset]) = val;
	}

	template<typename TVal>
	LUA_INLINE void Push(const TVal& val)
	{
		LUA_IF_CONSTEXPR(sizeof(TVal) < 4)
		{
			*reinterpret_cast<uintptr_t*>(&arguments[numArguments]) = 0;
		}

		*reinterpret_cast<TVal*>(&arguments[numArguments]) = val;

		LUA_IF_CONSTEXPR(sizeof(TVal) == sizeof(scrVectorLua))
		{
			numArguments += 3;
		}
		else
		{
			numArguments++;
		}
	}
};

#else

#include <scrEngine.h>
#include <ExceptionToModuleHelper.h>
#include <CL2LaunchMode.h>

#define LUA_EXC_WRAP_START(hash) \
	try                          \
	{

#define LUA_EXC_WRAP_END(hash)                                          \
	}                                                                   \
	catch (std::exception & e)                                          \
	{                                                                   \
		lua_pushstring(L, e.what());                                    \
		lua_error(L);                                                   \
	}                                                                   \
	catch (...)                                                         \
	{                                                                   \
		lua_pushstring(L, va("Error executing native %016llx.", hash)); \
		lua_error(L);                                                   \
	}

#define ASSERT_LUA_ARGS(count)    \
	if (!lua_asserttop(L, count)) \
		return 0;

struct LuaNativeWrapper
{
	rage::scrEngine::NativeHandler handler;

	LUA_INLINE LuaNativeWrapper(uint64_t hash)
	{
		handler = rage::scrEngine::GetNativeHandler(hash);
	}
};

static LONG ShouldHandleUnwind(PEXCEPTION_POINTERS ep, DWORD exceptionCode, uint64_t identifier)
{
	if (IsErrorException(ep))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// C++ exceptions?
	if (exceptionCode == 0xE06D7363)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// INVOKE_FUNCTION_REFERENCE crashing as top-level is usually related to native state corruption,
	// we'll likely want to crash on this instead rather than on an assertion down the chain
	if (identifier == HashString("INVOKE_FUNCTION_REFERENCE"))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

struct LuaNativeContext
{
	NativeContextRaw rawCxt;

	int numArguments;
	uintptr_t arguments[32];

	LuaNativeWrapper* nw;

	LUA_INLINE LuaNativeContext(LuaNativeWrapper* nw, int numArguments)
		: rawCxt(arguments, arguments, numArguments), numArguments(numArguments), nw(nw)
	{
	}

	LUA_INLINE void Invoke(lua_State* L, uint64_t hash)
	{
		static void* exceptionAddress;

		__try
		{
			nw->handler(&rawCxt);
			// rawCxt.SetVectorResults(); // codegen_out_native_lua should handle this operation
		}
		__except (exceptionAddress = (GetExceptionInformation())->ExceptionRecord->ExceptionAddress, ShouldHandleUnwind(GetExceptionInformation(), (GetExceptionInformation())->ExceptionRecord->ExceptionCode, hash))
		{
			throw std::exception(va("Error executing native 0x%016llx at address %s.", hash, FormatModuleAddress(exceptionAddress)));
		}
	}

	template<typename TVal>
	LUA_INLINE TVal GetResult()
	{
		return *(TVal*)(&arguments[0]);
	}

	template<typename TVal>
	LUA_INLINE void SetArgument(size_t offset, const TVal& val)
	{
		LUA_IF_CONSTEXPR(sizeof(TVal) < 4)
		{
			*reinterpret_cast<uintptr_t*>(&arguments[offset]) = 0;
		}

		*reinterpret_cast<TVal*>(&arguments[offset]) = val;
	}

	template<typename TVal>
	LUA_INLINE void Push(const TVal& val)
	{
		LUA_IF_CONSTEXPR(sizeof(TVal) < 4)
		{
			*reinterpret_cast<uintptr_t*>(&arguments[numArguments]) = 0;
		}

		*reinterpret_cast<TVal*>(&arguments[numArguments]) = val;

		LUA_IF_CONSTEXPR(sizeof(TVal) == sizeof(scrVectorLua))
		{
			numArguments += 3;
		}
		else
		{
			numArguments++;
		}
	}
};
#endif

#if LUA_VERSION_NUM == 504
#define INCLUDE_FXV2_NATIVES 1
#else
#define INCLUDE_FXV2_NATIVES 0
#endif

#if INCLUDE_FXV2_NATIVES
#if defined(GTA_FIVE)
#include "Natives.h"
#elif defined(IS_RDR3)
#include "NativesRDR.h"
#elif defined(IS_FXSERVER)
#include "NativesServer.h"
#endif
#endif

lua_CFunction Lua_GetNative(lua_State* L, const char* name)
{
#if INCLUDE_FXV2_NATIVES && !defined(GTA_NY)
	auto it = natives.find(name);
	return (it != natives.end()) ? it->second : nullptr;
#else
	return nullptr;
#endif
}

static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* self)
	{
		self->OnTick.Connect([self]()
		{
			g_hadProfiler = self->GetComponent<fx::ProfilerComponent>()->IsRecording();
		},
		INT32_MIN);
	});
});

#pragma endregion
