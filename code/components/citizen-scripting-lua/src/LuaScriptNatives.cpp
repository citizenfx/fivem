/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#if defined(_DEBUG) || defined(BUILD_LUA_SCRIPT_NATIVES)
#include <fxScripting.h>

#include <vector>
#include <map>

#include <ResourceManager.h>
#include <Profiler.h>

#include "LuaScriptNatives.h"
#include "LuaScriptRuntime.h"

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

// @TODO: Technically unsafe
extern uint8_t g_metaFields[];

extern uint64_t g_tickTime;
extern bool g_hadProfiler;

/// <summary>
/// </summary>
template<bool IsPtr>
static int SAFE_BUFFERS Lua_PushContextArgument(lua_State* L, int idx, fxLuaNativeContext<IsPtr>& context, fxLuaResult& result);

/// <summary>
/// Pushing function
/// </summary>
template<bool IsPtr, typename T>
static SAFE_BUFFERS LUA_INLINE void fxLuaNativeContext_PushArgument(fxLuaNativeContext<IsPtr>& context, T value)
{
	using TVal = std::decay_t<decltype(value)>;
	const int na = context.numArguments;

	LUA_IF_CONSTEXPR(sizeof(TVal) < sizeof(uintptr_t))
	{
		LUA_IF_CONSTEXPR(std::is_integral_v<TVal>)
		{
			LUA_IF_CONSTEXPR(std::is_signed_v<TVal>)
			{
				*reinterpret_cast<uintptr_t*>(&context.arguments[na]) = (uintptr_t)(uint32_t)value;
			}
			else
			{
				*reinterpret_cast<uintptr_t*>(&context.arguments[na]) = (uintptr_t)value;
			}
		}
		else
		{
			*reinterpret_cast<uintptr_t*>(&context.arguments[na]) = *reinterpret_cast<const uint32_t*>(&value);
		}
	}
	else
	{
		*reinterpret_cast<TVal*>(&context.arguments[na]) = value;
	}

	context.numArguments = na + 1;
}

template<bool IsPtr>
static SAFE_BUFFERS LUA_INLINE int fxLuaNativeContext_PushPointer(lua_State* L, fxLuaNativeContext<IsPtr>& context, fxLuaResult& result, LuaMetaFields metaField)
{
	if (result.numReturnValues >= _countof(result.retvals))
	{
		lua_pushliteral(L, "too many return value arguments");
		return lua_error(L);
	}

	// push the offset and set the type
	fxLuaNativeContext_PushArgument(context, &result.retvals[result.numReturnValues]);
	result.rettypes[result.numReturnValues] = metaField;

	// increment the counter
	if (metaField == LuaMetaFields::PointerValueVector)
		result.numReturnValues += 3;
	else
		result.numReturnValues += 1;

	return 1;
}

// table parsing implementation
template<bool IsPtr>
static SAFE_BUFFERS LUA_INLINE int fxLuaNativeContext_PushTable(lua_State* L, int idx, fxLuaNativeContext<IsPtr>& context, fxLuaResult& result)
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
		Lua_PushContextArgument(L, -1, context, result);
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
			Lua_PushContextArgument(L, -1, context, result);
			lua_pop(L, 1); // [...]
		}
		else
		{
			lua_pop(L, 1);
			lua_pushliteral(L, "Invalid Lua type in __data");
			return lua_error(L);
		}
	}
	return 1;
}

template<bool IsPtr>
static SAFE_BUFFERS LUA_INLINE void fxLuaNativeContext_PushUserdata(lua_State* L, fxLuaNativeContext<IsPtr>& context, fxLuaResult& result, uint8_t* ptr)
{
	// if the pointer is a metafield
	if (ptr >= g_metaFields && ptr < &g_metaFields[(int)LuaMetaFields::Max])
	{
		LuaMetaFields metaField = static_cast<LuaMetaFields>(ptr - g_metaFields);

		// switch on the metafield
		switch (metaField)
		{
			case LuaMetaFields::PointerValueInt:
			case LuaMetaFields::PointerValueFloat:
			case LuaMetaFields::PointerValueVector:
			{
				result.retvals[result.numReturnValues] = 0;

				if (metaField == LuaMetaFields::PointerValueVector)
				{
					result.retvals[result.numReturnValues + 1] = 0;
					result.retvals[result.numReturnValues + 2] = 0;
				}

				fxLuaNativeContext_PushPointer(L, context, result, metaField);
				break;
			}
			case LuaMetaFields::ReturnResultAnyway:
				result.returnResultAnyway = true;
				break;
			case LuaMetaFields::ResultAsInteger:
			case LuaMetaFields::ResultAsLong:
			case LuaMetaFields::ResultAsString:
			case LuaMetaFields::ResultAsFloat:
			case LuaMetaFields::ResultAsVector:
			case LuaMetaFields::ResultAsObject:
				result.returnResultAnyway = true;
				result.returnValueCoercion = metaField;
				break;
			default:
				break;
		}
	}
	// or if the pointer is a runtime pointer field
	else if (ptr >= reinterpret_cast<uint8_t*>(result.pointerFields) && ptr < (reinterpret_cast<uint8_t*>(result.pointerFields) + (sizeof(fx::PointerField) * 2)))
	{
		// guess the type based on the pointer field type
		const intptr_t ptrField = ptr - reinterpret_cast<uint8_t*>(result.pointerFields);
		const LuaMetaFields metaField = static_cast<LuaMetaFields>(ptrField / sizeof(fx::PointerField));

		if (metaField == LuaMetaFields::PointerValueInt || metaField == LuaMetaFields::PointerValueFloat)
		{
			auto ptrFieldEntry = reinterpret_cast<fx::PointerFieldEntry*>(ptr);

			result.retvals[result.numReturnValues] = ptrFieldEntry->value;
			ptrFieldEntry->empty = true;

			fxLuaNativeContext_PushPointer(L, context, result, metaField);
		}
	}
	else
	{
		// @TODO: Throw an error?
		fxLuaNativeContext_PushArgument(context, ptr);
	}
}

#if LUA_VERSION_NUM == 504
/*
**
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
static LUA_INLINE const TValue* __index2value(lua_State* L, int idx)
{
	const CallInfo* ci = L->ci;
	if (idx > 0)
	{
		const StkId o = ci->func + idx;
		api_check(L, idx <= L->ci->top - (ci->func + 1), "unacceptable index");
		return (o >= L->top) ? &G(L)->nilvalue : s2v(o);
	}
	else /* negative index */
	{
		api_check(L, idx != 0 && -idx <= L->top - (ci->func + 1), "invalid index");
		return s2v(L->top + idx);
	}
}
#endif

#if LUA_VERSION_NUM == 504
#define LUA_VALUE(L, I) __index2value((L), (I))
#else
#define LUA_VALUE(L, I) lua_getvalue((L), (I))
#endif

/// <summary>
/// Consider the possibly converting SHRSTR's to VLNGSTR's to avoid the handler
/// from invalidating internalized strings.
/// </summary>
/// <param name="L"></param>
/// <param name="idx"></param>
/// <param name="context"></param>
/// <returns></returns>
template<bool IsPtr>
static int SAFE_BUFFERS Lua_PushContextArgument(lua_State* L, int idx, fxLuaNativeContext<IsPtr>& context, fxLuaResult& result)
{
#if LUA_VERSION_NUM == 504
	const TValue* value = LUA_VALUE(L, idx);
	switch (ttypetag(value))
	{
		case LUA_VNIL:
		case LUA_VFALSE:
			fxLuaNativeContext_PushArgument(context, 0);
			break;
		case LUA_VTRUE:
			fxLuaNativeContext_PushArgument(context, 1);
			break;
		case LUA_VNUMINT:
			fxLuaNativeContext_PushArgument(context, ivalue(value));
			break;
		case LUA_VNUMFLT:
			fxLuaNativeContext_PushArgument(context, static_cast<float>(fltvalue(value)));
			break;
		case LUA_VSHRSTR:
#if defined(GRIT_POWER_BLOB)
		case LUA_VBLOBSTR:
#endif
		case LUA_VLNGSTR:
			fxLuaNativeContext_PushArgument(context, svalue(value));
			break;
		case LUA_VVECTOR2:
		{
			const glmVector& v = vvalue(value);

			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.x));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.y));

			break;
		}
		case LUA_VVECTOR3:
		{
			const glmVector& v = vvalue(value);

			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.x));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.y));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.z));

			break;
		}
		case LUA_VVECTOR4:
		{
			const glmVector& v = vvalue(value);

			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.x));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.y));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.z));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.v4.w));

			break;
		}
		case LUA_VQUAT: // Support (NOT) GLM_FORCE_QUAT_DATA_XYZW
		{
			const glmVector& v = vvalue(value);

			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.q.x));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.q.y));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.q.z));
			fxLuaNativeContext_PushArgument(context, static_cast<float>(v.q.w));

			break;
		}
		case LUA_VTABLE: // table (high-level class with __data field)
		{
			fxLuaNativeContext_PushTable(L, idx, context, result);
			break;
		}
		case LUA_VLIGHTUSERDATA:
		{
			fxLuaNativeContext_PushUserdata(L, context, result, reinterpret_cast<uint8_t*>(pvalue(value)));
			break;
		}
		default:
		{
			return luaL_error(L, "Invalid Lua type: %s", lua_typename(L, ttype(value)));
		}
	}
#else
	// get the type and decide what to do based on it
	const auto* value = LUA_VALUE(L, idx);
	int type = lua_valuetype(L, value);

	// nil: add '0'
	switch (type)
	{
		// nil
		case LUA_TNIL:
		{
			fxLuaNativeContext_PushArgument(context, 0);
			break;
		}
		// integer/float
		case LUA_TNUMBER:
		{
			if (lua_valueisinteger(L, value))
			{
				fxLuaNativeContext_PushArgument(context, lua_valuetointeger(L, value));
			}
			else if (lua_valueisfloat(L, value))
			{
				fxLuaNativeContext_PushArgument(context, static_cast<float>(lua_valuetonumber(L, value)));
			}
			break;
		}
		// boolean
		case LUA_TBOOLEAN:
		{
			fxLuaNativeContext_PushArgument(context, lua_valuetoboolean(L, value));
			break;
		}
		// table (high-level class with __data field)
		case LUA_TTABLE:
		{
			fxLuaNativeContext_PushTable(L, idx, context, result);
			break;
		}
		// string
		case LUA_TSTRING:
		{
			fxLuaNativeContext_PushArgument(context, lua_valuetostring(L, value));
			break;
		}
		// vectors
		case LUA_TVECTOR2:
		{
			auto f4 = lua_valuetofloat4(L, value);

			fxLuaNativeContext_PushArgument(context, f4.x);
			fxLuaNativeContext_PushArgument(context, f4.y);

			break;
		}
		case LUA_TVECTOR3:
		{
			auto f4 = lua_valuetofloat4(L, value);

			fxLuaNativeContext_PushArgument(context, f4.x);
			fxLuaNativeContext_PushArgument(context, f4.y);
			fxLuaNativeContext_PushArgument(context, f4.z);

			break;
		}
		case LUA_TVECTOR4:
		case LUA_TQUAT:
		{
			auto f4 = lua_valuetofloat4(L, value);

			fxLuaNativeContext_PushArgument(context, f4.x);
			fxLuaNativeContext_PushArgument(context, f4.y);
			fxLuaNativeContext_PushArgument(context, f4.z);
			fxLuaNativeContext_PushArgument(context, f4.w);

			break;
		}
		// metafield
		case LUA_TLIGHTUSERDATA:
		{
			uint8_t* ptr = reinterpret_cast<uint8_t*>(lua_valuetolightuserdata(L, value));
			fxLuaNativeContext_PushUserdata(L, context, result, ptr);
			break;
		}
		default:
		{
			return luaL_error(L, "Invalid Lua type: %s", lua_typename(L, type));
		}
	}
#endif
	return 1;
}

#ifndef IS_FXSERVER

#include <CL2LaunchMode.h>
struct FastNativeHandler
{
	uint64_t hash;
	rage::scrEngine::NativeHandler handler;
};

LUA_SCRIPT_LINKAGE int SAFE_BUFFERS Lua_GetNativeHandler(lua_State* L)
{
	auto hash = lua_tointeger(L, 1);
	auto handler = (!launch::IsSDK()) ? rage::scrEngine::GetNativeHandler(hash) : nullptr;

	auto nativeHandler = (FastNativeHandler*)lua_newuserdata(L, sizeof(FastNativeHandler));
	nativeHandler->hash = hash;
	nativeHandler->handler = handler;

	return 1;
}

static LONG ShouldHandleUnwind(DWORD exceptionCode, uint64_t identifier);

static LUA_INLINE void CallHandler(void* handler, uint64_t nativeIdentifier, rage::scrNativeCallContext& rageContext)
{
	// call the original function
	static void* exceptionAddress;

	__try
	{
		auto rageHandler = (rage::scrEngine::NativeHandler)handler;
		rageHandler(&rageContext);
	}
	__except (exceptionAddress = (GetExceptionInformation())->ExceptionRecord->ExceptionAddress, ShouldHandleUnwind((GetExceptionInformation())->ExceptionRecord->ExceptionCode, nativeIdentifier))
	{
		throw std::exception(va("Error executing native 0x%016llx at address %p.", nativeIdentifier, exceptionAddress));
	}
}

static void __declspec(safebuffers) CallHandlerSdk(void* handler, uint64_t nativeIdentifier, rage::scrNativeCallContext& rageContext)
{
	fxNativeContext context;
	memcpy(context.arguments, rageContext.GetArgumentBuffer(), sizeof(void*) * rageContext.GetArgumentCount());

	auto& luaRuntime = fx::LuaScriptRuntime::GetCurrent();
	auto scriptHost = luaRuntime->GetScriptHost();

	HRESULT hr = scriptHost->InvokeNative(context);

	if (!FX_SUCCEEDED(hr))
	{
		char* error = "Unknown";
		scriptHost->GetLastErrorText(&error);

		throw std::runtime_error(va("%s", error));
	}

	memcpy(rageContext.GetArgumentBuffer(), context.arguments, sizeof(void*) * 3);
}

static void __declspec(safebuffers) CallHandlerUniversal(void* handler, uint64_t nativeIdentifier, rage::scrNativeCallContext& rageContext);
static decltype(&CallHandlerUniversal) g_callHandler = &CallHandlerUniversal;

static void __declspec(safebuffers) CallHandlerUniversal(void* handler, uint64_t nativeIdentifier, rage::scrNativeCallContext& rageContext)
{
	if (launch::IsSDK())
	{
		g_callHandler = &CallHandlerSdk;
	}
	else
	{
		g_callHandler = &CallHandler;
	}

	return g_callHandler(handler, nativeIdentifier, rageContext);
}
#endif

template<bool IsPtr>
static int SAFE_BUFFERS __Lua_InvokeNative(lua_State* L)
{
	std::conditional_t<IsPtr, void*, uint64_t> hash;
	uint64_t origHash;

	LUA_IF_CONSTEXPR(!IsPtr)
	{
		// get the hash
		origHash = hash = lua_tointeger(L, 1);
	}
#ifndef IS_FXSERVER
	else
	{
		auto handlerRef = (FastNativeHandler*)lua_touserdata(L, 1);
		hash = handlerRef->handler;
		origHash = handlerRef->hash;
	}
#endif

#ifdef GTA_FIVE
	// hacky super fast path for 323 GET_HASH_KEY in GTA
	if (origHash == 0xD24D37CC275948CC)
	{
		// if NULL or an integer, return 0
		if (lua_isnil(L, 2) || lua_type(L, 2) == LUA_TNUMBER)
		{
			lua_pushinteger(L, 0);

			return 1;
		}

		const char* str = luaL_checkstring(L, 2);
		lua_pushinteger(L, static_cast<lua_Integer>(static_cast<int32_t>(HashString(str))));

		return 1;
	}
#endif

	// get required entries
	auto& luaRuntime = fx::LuaScriptRuntime::GetCurrent();
	auto scriptHost = luaRuntime->GetScriptHost();

	// variables to hold state
	fxLuaNativeContext<IsPtr> context;
	fxLuaResult result(luaRuntime->GetPointerFields());

	LUA_IF_CONSTEXPR(!IsPtr)
	{
		context.nativeIdentifier = (uint64_t)hash;
	}

	// get argument count for the loop
	const int numArgs = lua_gettop(L);
	for (int arg = 2; arg <= numArgs; arg++)
	{
		if (!Lua_PushContextArgument(L, arg, context, result))
		{
			return luaL_error(L, "Unexpected context result");
		}
	}

	// invoke the native on the script host
#ifndef IS_FXSERVER
	LUA_IF_CONSTEXPR(IsPtr)
	{
		// zero out three following arguments
		if (context.numArguments <= 29)
		{
			context.arguments[context.numArguments + 0] = 0;
			context.arguments[context.numArguments + 1] = 0;
			context.arguments[context.numArguments + 2] = 0;
		}

		auto handler = hash;
		context.SetArgumentCount(context.numArguments);

		try
		{
			if (handler)
			{
				g_callHandler(handler, origHash, context);
			}

			// append vector3 result components
			context.SetVectorResults();
		}
		catch (std::exception& e)
		{
			trace("%s: execution failed: %s\n", __func__, e.what());
			lua_pushstring(L, va("Execution of native %016llx in script host failed: %s", origHash, e.what()));
			lua_error(L);
		}
	}
	else
#endif
	{
		result_t hr = scriptHost->InvokeNative(context);

		if (!FX_SUCCEEDED(hr))
		{
			char* error = "Unknown";
			scriptHost->GetLastErrorText(&error);

			lua_pushstring(L, va("Execution of native %016llx in script host failed: %s", origHash, error));
			lua_error(L);
		}
	}

	// number of Lua results
	int numResults = 0;

	// if no other result was requested, or we need to return the result anyway, push the result
	if (result.numReturnValues == 0 || result.returnResultAnyway)
	{
		// increment the result count
		numResults++;

		// handle the type coercion
		switch (result.returnValueCoercion)
		{
			case LuaMetaFields::ResultAsString:
			{
				auto strString = reinterpret_cast<scrString*>(&context.arguments[0]);

				if (strString->magic == SCRSTRING_MAGIC_BINARY)
				{
					lua_pushlstring(L, strString->str, strString->len);
				}
				else if (strString->str)
				{
					lua_pushstring(L, strString->str);
				}
				else
				{
					lua_pushnil(L);
				}

				break;
			}
			case LuaMetaFields::ResultAsFloat:
			{
				lua_pushnumber(L, *reinterpret_cast<float*>(&context.arguments[0]));
				break;
			}
			case LuaMetaFields::ResultAsVector:
			{
				const scrVector& vector = *reinterpret_cast<scrVector*>(&context.arguments[0]);
#if LUA_VERSION_NUM == 504
				glm_pushvec3(L, glm::vec<3, glm_Float>(vector.x, vector.y, vector.z));
#else
				lua_pushvector3(L, vector.x, vector.y, vector.z);
#endif
				break;
			}
			case LuaMetaFields::ResultAsObject:
			{
				scrObject object = *reinterpret_cast<scrObject*>(&context.arguments[0]);
				lua_pushlstring(L, object.data, object.length);
				break;
			}
			case LuaMetaFields::ResultAsInteger:
			{
				lua_pushinteger(L, *reinterpret_cast<int32_t*>(&context.arguments[0]));
				break;
			}
			case LuaMetaFields::ResultAsLong:
			{
				lua_pushinteger(L, *reinterpret_cast<int64_t*>(&context.arguments[0]));
				break;
			}
			default:
			{
				const int32_t integer = *reinterpret_cast<int32_t*>(&context.arguments[0]);

				if ((integer & 0xFFFFFFFF) == 0)
				{
					lua_pushboolean(L, false);
				}
				else
				{
					lua_pushinteger(L, integer);
				}
				break;
			}
		}
	}

	// loop over the return value pointers
	{
		int i = 0;

		while (i < result.numReturnValues)
		{
			switch (result.rettypes[i])
			{
				case LuaMetaFields::PointerValueInt:
				{
					lua_pushinteger(L, *reinterpret_cast<int32_t*>(&result.retvals[i]));
					i++;
					break;
				}
				case LuaMetaFields::PointerValueFloat:
				{
					lua_pushnumber(L, *reinterpret_cast<float*>(&result.retvals[i]));
					i++;
					break;
				}
				case LuaMetaFields::PointerValueVector:
				{
					const scrVector& vector = *reinterpret_cast<scrVector*>(&result.retvals[i]);
#if LUA_VERSION_NUM == 504
					glm_pushvec3(L, glm::vec<3, glm_Float>(vector.x, vector.y, vector.z));
#else
					lua_pushvector3(L, vector.x, vector.y, vector.z);
#endif

					i += 3;
					break;
				}
				default:
					break;
			}

			numResults++;
		}
	}

	// and return with the 'desired' amount of results
	return numResults;
}

LUA_SCRIPT_LINKAGE int SAFE_BUFFERS Lua_InvokeNative(lua_State* L)
{
	return __Lua_InvokeNative<false>(L);
}

LUA_SCRIPT_LINKAGE int SAFE_BUFFERS Lua_InvokeNative2(lua_State* L)
{
	return __Lua_InvokeNative<true>(L);
}

#pragma region Lua_LoadNative

LUA_SCRIPT_LINKAGE int Lua_LoadNative(lua_State* L)
{
	const char* fn = luaL_checkstring(L, 1);

	auto& runtime = fx::LuaScriptRuntime::GetCurrent();

	try
	{
		int isCfxv2 = 0;
		runtime->GetScriptHost2()->GetNumResourceMetaData("is_cfxv2", &isCfxv2);

		if (isCfxv2) // TODO/TEMPORARY: fxv2 oal is disabled by default for now
		{
			runtime->GetScriptHost2()->GetNumResourceMetaData("use_fxv2_oal", &isCfxv2);
		}

		//#if !defined(GTA_FIVE) || (LUA_VERSION_NUM == 504)
		if (isCfxv2)
		//#endif
		{
			auto nativeImpl = Lua_GetNative(L, fn);

			if (nativeImpl)
			{
				lua_pushcfunction(L, nativeImpl);
				return 1;
			}
		}

		fx::OMPtr<fxIStream> stream;

		result_t hr = runtime->GetScriptHost()->OpenSystemFile(const_cast<char*>(va("%s0x%08x.lua", runtime->GetNativesDir(), HashRageString(fn))), stream.GetAddressOf());

		if (!FX_SUCCEEDED(hr))
		{
			lua_pushnil(L);
			return 1;
		}

		// read file data
		uint64_t length;

		if (FX_FAILED(hr = stream->GetLength(&length)))
		{
			lua_pushnil(L);
			return 1;
		}

		std::vector<char> fileData(length + 1);
		if (FX_FAILED(hr = stream->Read(&fileData[0], length, nullptr)))
		{
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
	const TValue* value = LUA_VALUE(L, idx);
	if (ttisinteger(value))
		return ivalue(value) != 0;
	return l_isfalse(value) ? false : true;
}

template<>
LUA_INLINE float LuaArgumentParser::ParseArgument<float>(lua_State* L, int idx)
{
	const TValue* value = LUA_VALUE(L, idx);
	return ttisnumber(value) ? sc_nvalue(value, float) : 0.f;
}

template<>
LUA_INLINE double LuaArgumentParser::ParseArgument<double>(lua_State* L, int idx)
{
	const TValue* value = LUA_VALUE(L, idx);
	return ttisnumber(value) ? sc_nvalue(value, double) : 0.0;
}

template<>
LUA_INLINE int16_t LuaArgumentParser::ParseArgument<int16_t>(lua_State* L, int idx)
{
	const TValue* value = LUA_VALUE(L, idx);
	return ttisnumber(value) ? sc_nvalue(value, int16_t) : (!l_isfalse(value) ? 1 : 0);
}

template<>
LUA_INLINE int32_t LuaArgumentParser::ParseArgument<int32_t>(lua_State* L, int idx)
{
	const TValue* value = LUA_VALUE(L, idx);
	return ttisnumber(value) ? sc_nvalue(value, int32_t) : (!l_isfalse(value) ? 1 : 0);
}

template<>
LUA_INLINE int64_t LuaArgumentParser::ParseArgument<int64_t>(lua_State* L, int idx)
{
	const TValue* value = LUA_VALUE(L, idx);
	return ttisnumber(value) ? sc_nvalue(value, int64_t) : (!l_isfalse(value) ? 1 : 0);
}

#if defined(__GNUC__)
template<>
LUA_INLINE lua_Integer LuaArgumentParser::ParseArgument<lua_Integer>(lua_State* L, int idx)
{
	const TValue* value = LUA_VALUE(L, idx);
	return ttisnumber(value) ? sc_nvalue(value, lua_Integer) : (!l_isfalse(value) ? 1 : 0);
}
#endif

/// <summary>
/// Codegen uses unsigned integer types to denote hashes.
/// </summary>
template<>
LUA_INLINE uint32_t LuaArgumentParser::ParseArgument<uint32_t>(lua_State* L, int idx)
{
	const TValue* o = LUA_VALUE(L, idx);
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
	const TValue* value = LUA_VALUE(L, idx);
	switch (ttype(value))
	{
		case LUA_TNIL: return NULL;
		case LUA_TSTRING: return svalue(value);
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
	const TValue* o = LUA_VALUE(L, idx);
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
	if (val.magic == SCRSTRING_MAGIC_BINARY)
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
		numArguments = numArguments;
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

		LUA_IF_CONSTEXPR(sizeof(TVal) == sizeof(scrVector))
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

static LONG ShouldHandleUnwind(DWORD exceptionCode, uint64_t identifier)
{
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
		: rawCxt(arguments, numArguments), numArguments(numArguments), nw(nw)
	{
	}

	LUA_INLINE void Invoke(lua_State* L, uint64_t hash)
	{
		static void* exceptionAddress;

		__try
		{
			nw->handler(&rawCxt);
		}
		__except (exceptionAddress = (GetExceptionInformation())->ExceptionRecord->ExceptionAddress, ShouldHandleUnwind((GetExceptionInformation())->ExceptionRecord->ExceptionCode, hash))
		{
			throw std::exception(va("Error executing native 0x%016llx at address %p.", hash, exceptionAddress));
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

		LUA_IF_CONSTEXPR(sizeof(TVal) == sizeof(scrVector))
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
#if !defined(IS_FXSERVER) && defined(GTA_FIVE)
#include "Natives.h"
#elif defined(IS_FXSERVER)
#include "NativesServer.h"
#endif
#endif

LUA_SCRIPT_LINKAGE lua_CFunction Lua_GetNative(lua_State* L, const char* name)
{
#if INCLUDE_FXV2_NATIVES && (defined(GTA_FIVE) || defined(IS_FXSERVER))
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
			constexpr uint64_t GET_GAME_TIMER =
#ifdef IS_FXSERVER
			0xa4ea0691
#elif defined(GTA_FIVE)
			0x9CD27B0045628463
#elif defined(IS_RDR3)
			0x4F67E8ECA7D3F667
#elif defined(GTA_NY)
			0x022B2DA9
#else
#error Undefined GET_GAME_TIMER
#endif
			;

#ifdef IS_FXSERVER
			if (fx::LuaScriptRuntime::GetLastHost())
#endif
			{
				static LuaNativeWrapper nW(GET_GAME_TIMER);
				LuaNativeContext nCtx(&nW, 0);
				nCtx.Invoke(nullptr, GET_GAME_TIMER);

				g_tickTime = nCtx.GetResult<int32_t>();
			}

			g_hadProfiler = self->GetComponent<fx::ProfilerComponent>()->IsRecording();
		},
		INT32_MIN);
	});
});

#pragma endregion

#endif
