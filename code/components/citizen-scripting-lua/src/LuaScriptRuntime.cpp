/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fxScripting.h"

#include "LuaScriptRuntime.h"
#include "LuaScriptNatives.h"

#include <msgpack.hpp>
#include <json.hpp>

#include <CoreConsole.h>

#include <lua.hpp>
#include <lua_cmsgpacklib.h>
#include <lua_rapidjsonlib.h>
#if LUA_VERSION_NUM == 504
#include <lglmlib.hpp>
#endif

extern LUA_INTERNAL_LINKAGE
{
#include <lobject.h>
}

#if defined(GTA_FIVE)
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
	{ "natives_21e43a33.lua", guid_t{ 0 } },
	{ "natives_0193d0af.lua", "f15e72ec-3972-4fe4-9c7d-afc5394ae207" },
	{ "natives_universal.lua", "44febabe-d386-4d18-afbe-5e627f4af937" }
};
#elif defined(IS_RDR3)
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
	{ "rdr3_universal.lua", guid_t{ 0 } }
};
#elif defined(GTA_NY)
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
	{ "ny_universal.lua", guid_t{ 0 } }
};
#else
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
	{ "natives_server.lua", guid_t{ 0 } }
};
#endif

/// <summary>
/// </summary>
uint8_t g_metaFields[(size_t)LuaMetaFields::Max];

/// <summary>
/// </summary>
static fx::OMPtr<fx::LuaScriptRuntime> g_currentLuaRuntime;

/// <summary>
/// </summary>
static IScriptHost* g_lastScriptHost;

// luaL_openlibs version without io/os libs
static const luaL_Reg lualibs[] = {
	{ "_G", luaopen_base },
	{ LUA_TABLIBNAME, luaopen_table },
	{ LUA_STRLIBNAME, luaopen_string },
	{ LUA_MATHLIBNAME, luaopen_math },
	{ LUA_DBLIBNAME, luaopen_debug },
	{ LUA_COLIBNAME, luaopen_coroutine },
	{ LUA_UTF8LIBNAME, luaopen_utf8 },
#ifdef IS_FXSERVER
	{ LUA_IOLIBNAME, luaopen_io },
	{ LUA_OSLIBNAME, luaopen_os },
#endif
	{ "msgpack", luaopen_cmsgpack },
	{ "json", luaopen_rapidjson },
	{ NULL, NULL }
};

/// <summary>
/// Console
/// </summary>
namespace fx
{
void ScriptTraceV(const char* string, fmt::printf_args formatList)
{
	auto t = fmt::vsprintf(string, formatList);
	console::Printf(fmt::sprintf("script:%s", LuaScriptRuntime::GetCurrent()->GetResourceName()), "%s", t);

	LuaScriptRuntime::GetCurrent()->GetScriptHost()->ScriptTrace(const_cast<char*>(t.c_str()));
}

template<typename... TArgs>
void ScriptTrace(const char* string, const TArgs&... args)
{
	ScriptTraceV(string, fmt::make_printf_args(args...));
}

static int Lua_Print(lua_State* L)
{
	const int n = lua_gettop(L); /* number of arguments */

	lua_getglobal(L, "tostring");
	for (int i = 1; i <= n; i++)
	{
		lua_pushvalue(L, -1); /* function to be called */
		lua_pushvalue(L, i); /* value to print */
		lua_call(L, 1, 1);

		size_t l = 0;
		const char* s = lua_tolstring(L, -1, &l); /* get result */
		if (s == NULL)
			return luaL_error(L, "'tostring' must return a string to 'print'");

		if (i > 1)
			ScriptTrace("%s", std::string("\t", 1));
		ScriptTrace("%s", std::string(s, l));
		lua_pop(L, 1); /* pop result */
	}
	ScriptTrace("\n");
	return 0;
}
}

/// <summary>
/// CitizenLib
/// </summary>
namespace fx
{
struct LuaBoundary
{
	lua_Integer hint;
	lua_State* thread;
};

static int Lua_Trace(lua_State* L)
{
	ScriptTrace("%s", luaL_checkstring(L, 1));
	return 0;
}

static int Lua_SetTickRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	const int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the tick callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetTickRoutine([=]()
	{
		// set the error handler
		lua_pushcfunction(L, luaRuntime->GetDbTraceback());

		const int eh = lua_gettop(L);

		// get the referenced function
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// invoke the tick routine
		if (lua_pcall(L, 0, 0, eh) != 0)
		{
			// Copies the contents of the string, ensuring the pointer remains
			// valid after lua_pop()
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			ScriptTrace("Error running system tick function for resource %s: %s\n", luaRuntime->GetResourceName(), err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

static int Lua_SetStackTraceRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	const int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the tick callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetStackTraceRoutine([=](void* start, void* end, char** blob, size_t* size)
	{
		// static array for retval output (sadly)
		static std::vector<char> retvalArray(32768);

		// set the error handler
		lua_pushcfunction(L, luaRuntime->GetDbTraceback());

		const int eh = lua_gettop(L);

		// get the referenced function
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// push arguments on the stack
		if (start)
		{
			auto startRef = (LuaBoundary*)start;
			lua_pushinteger(L, startRef->hint);

			if (startRef->thread)
			{
				lua_pushthread(startRef->thread);
				lua_xmove(startRef->thread, L, 1);
			}
			else
			{
				lua_pushnil(L);
			}
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}

		if (end)
		{
			auto endRef = (LuaBoundary*)end;
			lua_pushinteger(L, endRef->hint);

			if (endRef->thread)
			{
				lua_pushthread(endRef->thread);
				lua_xmove(endRef->thread, L, 1);
			}
			else
			{
				lua_pushnil(L);
			}
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}

		// invoke the tick routine
		if (lua_pcall(L, 4, 1, eh) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			ScriptTrace("Error running stack trace function for resource %s: %s\n", luaRuntime->GetResourceName(), err.c_str());

			*blob = nullptr;
			*size = 0;
		}
		else
		{
			const char* retvalString = lua_tolstring(L, -1, size);

			if (*size > retvalArray.size())
			{
				retvalArray.resize(*size);
			}

			memcpy(&retvalArray[0], retvalString, fwMin(retvalArray.size(), *size));

			*blob = &retvalArray[0];

			lua_pop(L, 1); // as there's a result
		}

		lua_pop(L, 1);
	});

	return 0;
}

static int Lua_SetEventRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetEventRoutine([=](const char* eventName, const char* eventPayload, size_t payloadSize, const char* eventSource)
	{
		// set the error handler
		lua_pushcfunction(L, luaRuntime->GetDbTraceback());

		int eh = lua_gettop(L);

		// get the referenced function
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// push arguments on the stack
		lua_pushstring(L, eventName);
		lua_pushlstring(L, eventPayload, payloadSize);
		lua_pushstring(L, eventSource);

		// invoke the tick routine
		if (lua_pcall(L, 3, 0, eh) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			ScriptTrace("Error running system event handling function for resource %s: %s\n", luaRuntime->GetResourceName(), err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

static int Lua_SetCallRefRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	const int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetCallRefRoutine([=](int32_t refId, const char* argsSerialized, size_t argsSize, char** retval, size_t* retvalLength)
	{
		// static array for retval output (sadly)
		static std::vector<char> retvalArray(32768);

		// set the error handler
		lua_pushcfunction(L, luaRuntime->GetDbTraceback());

		const int eh = lua_gettop(L);

		// get the referenced function
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// push arguments on the stack
		lua_pushinteger(L, refId);
		lua_pushlstring(L, argsSerialized, argsSize);

		// invoke the tick routine
		if (lua_pcall(L, 2, 1, eh) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			ScriptTrace("Error running call reference function for resource %s: %s\n", luaRuntime->GetResourceName(), err.c_str());

			*retval = nullptr;
			*retvalLength = 0;
		}
		else
		{
			const char* retvalString = lua_tolstring(L, -1, retvalLength);

			if (*retvalLength > retvalArray.size())
			{
				retvalArray.resize(*retvalLength);
			}

			memcpy(&retvalArray[0], retvalString, fwMin(retvalArray.size(), *retvalLength));

			*retval = &retvalArray[0];

			lua_pop(L, 1); // as there's a result
		}

		lua_pop(L, 1);
	});

	return 0;
}

static int Lua_SetDeleteRefRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	const int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetDeleteRefRoutine([=](int32_t refId)
	{
		// set the error handler
		lua_pushcfunction(L, luaRuntime->GetDbTraceback());

		const int eh = lua_gettop(L);

		// get the referenced function
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// push arguments on the stack
		lua_pushinteger(L, refId);

		// invoke the routine
		if (lua_pcall(L, 1, 0, eh) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			ScriptTrace("Error running system ref deletion function for resource %s: %s\n", luaRuntime->GetResourceName(), err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

static int Lua_SetDuplicateRefRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	const int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetDuplicateRefRoutine([=](int32_t refId)
	{
		// set the error handler
		lua_pushcfunction(L, luaRuntime->GetDbTraceback());

		const int eh = lua_gettop(L);

		// get the referenced function
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// push arguments on the stack
		lua_pushinteger(L, refId);

		// return value holder
		int32_t retval;

		// invoke the routine
		if (lua_pcall(L, 1, 1, eh) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			ScriptTrace("Error running system ref duplication function for resource %s: %s\n", luaRuntime->GetResourceName(), err.c_str());
		}
		else
		{
			retval = lua_tointeger(L, -1);
			lua_pop(L, 1);
		}

		lua_pop(L, 1);

		return retval;
	});

	return 0;
}

static int Lua_CanonicalizeRef(lua_State* L)
{
	auto& luaRuntime = LuaScriptRuntime::GetCurrent();

	char* refString;
	result_t hr = luaRuntime->GetScriptHost()->CanonicalizeRef(luaL_checkinteger(L, 1), luaRuntime->GetInstanceId(), &refString);

	lua_pushstring(L, refString);
	fwFree(refString);

	return 1;
}

static int Lua_InvokeFunctionReference(lua_State* L)
{
	// get required entries
	auto& luaRuntime = LuaScriptRuntime::GetCurrent();
	auto scriptHost = luaRuntime->GetScriptHost();

	// variables to hold state
	fxNativeContext context = { 0 };

	context.numArguments = 4;
	context.nativeIdentifier = 0xe3551879; // INVOKE_FUNCTION_REFERENCE

	// identifier string
	context.arguments[0] = reinterpret_cast<uintptr_t>(luaL_checkstring(L, 1));

	// argument data
	size_t argLength;
	const char* argString = luaL_checklstring(L, 2, &argLength);

	context.arguments[1] = reinterpret_cast<uintptr_t>(argString);
	context.arguments[2] = static_cast<uintptr_t>(argLength);

	// return value length
	size_t retLength = 0;
	context.arguments[3] = reinterpret_cast<uintptr_t>(&retLength);

	// invoke
	if (FX_FAILED(scriptHost->InvokeNative(context)))
	{
		char* error = "Unknown";
		scriptHost->GetLastErrorText(&error);

		lua_pushstring(L, va("Execution of native %016x in script host failed: %s", 0xe3551879, error));
		lua_error(L);
	}

	// get return values
	lua_pushlstring(L, reinterpret_cast<const char*>(context.arguments[0]), retLength);

	// return as such
	return 1;
}

static int Lua_SubmitBoundaryStart(lua_State* L)
{
	// get required entries
	auto& luaRuntime = LuaScriptRuntime::GetCurrent();
	auto scriptHost = luaRuntime->GetScriptHost();

	lua_Integer val = lua_tointeger(L, 1);
	lua_State* thread = lua_tothread(L, 2);

	LuaBoundary b;
	b.hint = val;
	b.thread = thread;

	scriptHost->SubmitBoundaryStart((char*)&b, sizeof(b));

	return 0;
}

static int Lua_SubmitBoundaryEnd(lua_State* L)
{
	// get required entries
	auto& luaRuntime = LuaScriptRuntime::GetCurrent();
	auto scriptHost = luaRuntime->GetScriptHost();

	lua_Integer val = lua_tointeger(L, 1);
	lua_State* thread = lua_tothread(L, 2);

	LuaBoundary b;
	b.hint = val;
	b.thread = thread;

	scriptHost->SubmitBoundaryEnd((char*)&b, sizeof(b));

	return 0;
}

template<LuaMetaFields metaField>
static int Lua_GetMetaField(lua_State* L)
{
	lua_pushlightuserdata(L, &g_metaFields[(int)metaField]);

	return 1;
}

template<LuaMetaFields MetaField>
static int Lua_GetPointerField(lua_State* L)
{
	auto& runtime = LuaScriptRuntime::GetCurrent();

	auto pointerFields = runtime->GetPointerFields();
	auto pointerFieldStart = &pointerFields[(int)MetaField];

	static uintptr_t dummyOut;
	PointerFieldEntry* pointerField = nullptr;

	for (int i = 0; i < _countof(pointerFieldStart->data); i++)
	{
		if (pointerFieldStart->data[i].empty)
		{
			pointerField = &pointerFieldStart->data[i];
			pointerField->empty = false;

			// to prevent accidental passing of arguments like _r, we check if this is a userdata
			const int type = lua_type(L, 1);
			if (type == LUA_TNIL || type == LUA_TLIGHTUSERDATA || type == LUA_TUSERDATA)
			{
				pointerField->value = 0;
			}
			else if (MetaField == LuaMetaFields::PointerValueFloat)
			{
				float value = static_cast<float>(luaL_checknumber(L, 1));

				pointerField->value = *reinterpret_cast<uint32_t*>(&value);
			}
			else if (MetaField == LuaMetaFields::PointerValueInt)
			{
				intptr_t value = luaL_checkinteger(L, 1);

				pointerField->value = value;
			}

			break;
		}
	}

	lua_pushlightuserdata(L, (pointerField) ? static_cast<void*>(pointerField) : &dummyOut);
	return 1;
}

static int Lua_Require(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	lua_settop(L, 1); /* LOADED table will be at index 2 */
#if LUA_VERSION_NUM >= 504
	lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
#else
	lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
#endif
	lua_getfield(L, 2, name); /* LOADED[name] */
	if (lua_toboolean(L, -1)) /* is it there? */
	{
		return 1; /* package is already loaded */
	}

#if LUA_VERSION_NUM >= 504
	if (strcmp(name, LUA_GLMLIBNAME) == 0)
	{
		luaL_requiref(L, LUA_GLMLIBNAME, luaopen_glm, 1);
		return 1;
	}
#endif

	// @TODO: Consider implementing a custom 'loadlib' module that uses VFS,
	// for example, LoadSystemFile("citizen:/scripting/lua/json.lua"). Server
	// side luarocks integration would be pretty neat.

	return 0;
}

static const struct luaL_Reg g_citizenLib[] = {
	{ "SetTickRoutine", Lua_SetTickRoutine },
	{ "SetEventRoutine", Lua_SetEventRoutine },
	{ "Trace", Lua_Trace },
	{ "InvokeNative", Lua_InvokeNative },
#ifndef IS_FXSERVER
	{ "GetNative", Lua_GetNativeHandler },
	{ "InvokeNative2", Lua_InvokeNative2 },
#endif
	{ "LoadNative", Lua_LoadNative },
	// ref things
	{ "SetCallRefRoutine", Lua_SetCallRefRoutine },
	{ "SetDeleteRefRoutine", Lua_SetDeleteRefRoutine },
	{ "SetDuplicateRefRoutine", Lua_SetDuplicateRefRoutine },
	{ "CanonicalizeRef", Lua_CanonicalizeRef },
	{ "InvokeFunctionReference", Lua_InvokeFunctionReference },
	// boundary
	{ "SubmitBoundaryStart", Lua_SubmitBoundaryStart },
	{ "SubmitBoundaryEnd", Lua_SubmitBoundaryEnd },
	{ "SetStackTraceRoutine", Lua_SetStackTraceRoutine },
	// metafields
	{ "PointerValueIntInitialized", Lua_GetPointerField<LuaMetaFields::PointerValueInt> },
	{ "PointerValueFloatInitialized", Lua_GetPointerField<LuaMetaFields::PointerValueFloat> },
	{ "PointerValueInt", Lua_GetMetaField<LuaMetaFields::PointerValueInt> },
	{ "PointerValueFloat", Lua_GetMetaField<LuaMetaFields::PointerValueFloat> },
	{ "PointerValueVector", Lua_GetMetaField<LuaMetaFields::PointerValueVector> },
	{ "ReturnResultAnyway", Lua_GetMetaField<LuaMetaFields::ReturnResultAnyway> },
	{ "ResultAsInteger", Lua_GetMetaField<LuaMetaFields::ResultAsInteger> },
	{ "ResultAsLong", Lua_GetMetaField<LuaMetaFields::ResultAsLong> },
	{ "ResultAsFloat", Lua_GetMetaField<LuaMetaFields::ResultAsFloat> },
	{ "ResultAsString", Lua_GetMetaField<LuaMetaFields::ResultAsString> },
	{ "ResultAsVector", Lua_GetMetaField<LuaMetaFields::ResultAsVector> },
	{ "ResultAsObject", Lua_GetMetaField<LuaMetaFields::ResultAsObject> },
	{ nullptr, nullptr }
};
}

/// <summary>
/// LuaScriptRuntime
/// </summary>
namespace fx
{
class LuaPushEnvironment
{
private:
	fx::PushEnvironment m_pushEnvironment;

	OMPtr<LuaScriptRuntime> m_lastLuaRuntime;

public:
	LUA_INLINE LuaPushEnvironment(LuaScriptRuntime* runtime)
		: m_pushEnvironment(runtime)
	{
		g_lastScriptHost = runtime->GetScriptHost();

		m_lastLuaRuntime = g_currentLuaRuntime;
		g_currentLuaRuntime = runtime;
	}

	LUA_INLINE ~LuaPushEnvironment()
	{
		g_currentLuaRuntime = m_lastLuaRuntime;
	}
};

LuaScriptRuntime::~LuaScriptRuntime()
{
	// If == g_lastScriptHost
}

const OMPtr<LuaScriptRuntime>& LuaScriptRuntime::GetCurrent()
{
#if _DEBUG
	LuaScriptRuntime* luaRuntime;
	OMPtr<IScriptRuntime> runtime;

	assert(FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)));
	assert(luaRuntime = static_cast<LuaScriptRuntime*>(runtime.GetRef()));

	assert(luaRuntime == g_currentLuaRuntime.GetRef());
#endif

	return g_currentLuaRuntime;
}

IScriptHost* LuaScriptRuntime::GetLastHost()
{
	return g_lastScriptHost;
}

void LuaScriptRuntime::SetTickRoutine(const std::function<void()>& tickRoutine)
{
	if (!m_tickRoutine)
	{
		m_tickRoutine = tickRoutine;
	}
}

void LuaScriptRuntime::SetEventRoutine(const TEventRoutine& eventRoutine)
{
	if (!m_eventRoutine)
	{
		m_eventRoutine = eventRoutine;
	}
}

result_t LuaScriptRuntime::Create(IScriptHost* scriptHost)
{
	m_scriptHost = scriptHost;

	{
		fx::OMPtr<IScriptHost> ptr(scriptHost);
		fx::OMPtr<IScriptHostWithResourceData> resourcePtr;
		ptr.As(&resourcePtr);

		m_resourceHost = resourcePtr.GetRef();

		fx::OMPtr<IScriptHostWithManifest> manifestPtr;
		ptr.As(&manifestPtr);

		m_manifestHost = manifestPtr.GetRef();
	}

	std::string nativesBuild = "natives_21e43a33.lua";

	{
		for (const auto& versionPair : g_scriptVersionPairs)
		{
			bool isGreater;

			if (FX_SUCCEEDED(m_manifestHost->IsManifestVersionBetween(std::get<ManifestVersion>(versionPair).guid, guid_t{ 0 }, &isGreater)) && isGreater)
			{
				nativesBuild = std::get<const char*>(versionPair);
			}
		}
	}

	{
		bool isGreater;

		if (FX_SUCCEEDED(m_manifestHost->IsManifestVersionV2Between("adamant", "", &isGreater)) && isGreater)
		{
			nativesBuild =
#if defined(GTA_FIVE)
			"natives_universal.lua"
#elif defined(IS_RDR3)
			"rdr3_universal.lua"
#elif defined(GTA_NY)
			"ny_universal.lua"
#else
			"natives_server.lua"
#endif
			;
		}
	}

	// safe_openlibs
	const luaL_Reg* lib = lualibs;
	for (; lib->func; lib++)
	{
		luaL_requiref(m_state, lib->name, lib->func, 1);
		lua_pop(m_state, 1);
	}

	{
		// 0
		lua_getglobal(m_state, "debug");

		// 1
		lua_getfield(m_state, -1, "traceback");

		// 2
		m_dbTraceback = lua_tocfunction(m_state, -1);
		lua_pop(m_state, 2);

		// 0
	}

	// register the 'Citizen' library
	lua_newtable(m_state);
	luaL_setfuncs(m_state, g_citizenLib, 0);
	lua_setglobal(m_state, "Citizen");

	// load the system scheduler script
	result_t hr;

	if (FX_FAILED(hr = LoadNativesBuild(nativesBuild)))
	{
		return hr;
	}

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/deferred.lua")))
	{
		return hr;
	}

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/scheduler.lua")))
	{
		return hr;
	}

	lua_pushnil(m_state);
	lua_setglobal(m_state, "dofile");

	lua_pushnil(m_state);
	lua_setglobal(m_state, "loadfile");

	lua_pushcfunction(m_state, Lua_Print);
	lua_setglobal(m_state, "print");

	lua_pushcfunction(m_state, Lua_Require);
	lua_setglobal(m_state, "require");

	return FX_S_OK;
}

result_t LuaScriptRuntime::LoadNativesBuild(const std::string& nativesBuild)
{
	result_t hr = FX_S_OK;

	bool useLazyNatives = false;

#if !defined(IS_FXSERVER)
	useLazyNatives = true;
#endif

	if (!useLazyNatives)
	{
		if (FX_FAILED(hr = LoadSystemFile(const_cast<char*>(va("citizen:/scripting/lua/%s", nativesBuild)))))
		{
			return hr;
		}
	}
	else
	{
		m_nativesDir = "nativesLua:/" + nativesBuild.substr(0, nativesBuild.length() - 4) + "/";

		if (FX_FAILED(hr = LoadSystemFile(const_cast<char*>(va("citizen:/scripting/lua/%s", "natives_loader.lua")))))
		{
			return hr;
		}
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::Destroy()
{
	// destroy any routines that may be referencing the Lua state
	m_eventRoutine = TEventRoutine();
	m_tickRoutine = std::function<void()>();
	m_callRefRoutine = TCallRefRoutine();
	m_deleteRefRoutine = TDeleteRefRoutine();
	m_duplicateRefRoutine = TDuplicateRefRoutine();

	// we need to push the environment before closing as items may have __gc callbacks requiring a current runtime to be set
	// in addition, we can't do this in the destructor due to refcounting odditiies (PushEnvironment adds a reference, causing infinite deletion loops)
	LuaPushEnvironment pushed(this);
	m_state.Close();

	return FX_S_OK;
}

int32_t LuaScriptRuntime::GetInstanceId()
{
	return m_instanceId;
}

result_t LuaScriptRuntime::LoadFileInternal(OMPtr<fxIStream> stream, char* scriptFile)
{
	// read file data
	uint64_t length;
	result_t hr;

	if (FX_FAILED(hr = stream->GetLength(&length)))
	{
		return hr;
	}

	std::vector<char> fileData(length + 1);
	if (FX_FAILED(hr = stream->Read(&fileData[0], length, nullptr)))
	{
		return hr;
	}

	fileData[length] = '\0';

	// create a chunk name prefixed with @ (suppresses '[string "..."]' formatting)
	fwString chunkName("@");
	chunkName.append(scriptFile);

	if (luaL_loadbufferx(m_state, &fileData[0], length, chunkName.c_str(), "t") != 0)
	{
		std::string err = luaL_checkstring(m_state, -1);
		lua_pop(m_state, 1);

		ScriptTrace("Error parsing script %s in resource %s: %s\n", scriptFile, GetResourceName(), err.c_str());

		// TODO: change?
		return FX_E_INVALIDARG;
	}

	if (m_debugListener.GetRef())
	{
		auto idIt = m_scriptIds.find(scriptFile);

		if (idIt != m_scriptIds.end())
		{
			std::vector<int> lineNums;

			int numProtos = lua_toprotos(m_state, -1);

			for (int i = 0; i < numProtos; i++)
			{
				lua_Debug debug;
				lua_getinfo(m_state, ">L", &debug);

				lua_pushnil(m_state);

				while (lua_next(m_state, -2) != 0)
				{
					int lineNum = lua_tointeger(m_state, -2); // 'whose indices are the numbers of the lines that are valid on the function'
					lineNums.push_back(lineNum - 1);

					lua_pop(m_state, 1);
				}

				lua_pop(m_state, 1);
			}

			if (m_debugListener.GetRef())
			{
				auto j = nlohmann::json::array();

				for (auto& line : lineNums)
				{
					j.push_back(line);
				}

				m_debugListener->OnBreakpointsDefined(idIt->second, const_cast<char*>(j.dump().c_str()));
			}
		}
	}

	return true;
}

result_t LuaScriptRuntime::LoadHostFileInternal(char* scriptFile)
{
	// open the file
	OMPtr<fxIStream> stream;

	result_t hr = m_scriptHost->OpenHostFile(scriptFile, stream.GetAddressOf());

	if (FX_FAILED(hr))
	{
		// TODO: log this?
		return hr;
	}

	char* resourceName;
	m_resourceHost->GetResourceName(&resourceName);

	return LoadFileInternal(stream, (scriptFile[0] != '@') ? const_cast<char*>(fmt::sprintf("@%s/%s", resourceName, scriptFile).c_str()) : scriptFile);
}

result_t LuaScriptRuntime::LoadSystemFileInternal(char* scriptFile)
{
	// open the file
	OMPtr<fxIStream> stream;

	result_t hr = m_scriptHost->OpenSystemFile(scriptFile, stream.GetAddressOf());

	if (FX_FAILED(hr))
	{
		// TODO: log this?
		return hr;
	}

	return LoadFileInternal(stream, scriptFile);
}

result_t LuaScriptRuntime::RunFileInternal(char* scriptName, std::function<result_t(char*)> loadFunction)
{
	LuaPushEnvironment pushed(this);
	lua_pushcfunction(m_state, GetDbTraceback());

	int eh = lua_gettop(m_state);

	result_t hr;

	if (FX_FAILED(hr = loadFunction(scriptName)))
	{
		return hr;
	}

	if (lua_pcall(m_state, 0, 0, eh) != 0)
	{
		std::string err = luaL_checkstring(m_state, -1);
		lua_pop(m_state, 1);

		ScriptTrace("Error loading script %s in resource %s: %s\n", scriptName, GetResourceName(), err.c_str());

		return FX_E_INVALIDARG;
	}

	lua_pop(m_state, 1);

	return FX_S_OK;
}

result_t LuaScriptRuntime::LoadFile(char* scriptName)
{
	return RunFileInternal(scriptName, std::bind(&LuaScriptRuntime::LoadHostFileInternal, this, std::placeholders::_1));
}

result_t LuaScriptRuntime::LoadSystemFile(char* scriptName)
{
	return RunFileInternal(scriptName, std::bind(&LuaScriptRuntime::LoadSystemFileInternal, this, std::placeholders::_1));
}

int32_t LuaScriptRuntime::HandlesFile(char* fileName, IScriptHostWithResourceData* metadata)
{
	if (strstr(fileName, ".lua") != 0)
	{
		int isLua54 = 0;
		metadata->GetNumResourceMetaData("lua54", &isLua54);

#if LUA_VERSION_NUM == 504
		return isLua54 > 0;
#else
		return isLua54 == 0;
#endif
	}
	return false;
}

result_t LuaScriptRuntime::Tick()
{
	if (m_tickRoutine)
	{
		LuaPushEnvironment pushed(this);

		m_tickRoutine();
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::WalkStack(char* boundaryStart, uint32_t boundaryStartLength, char* boundaryEnd, uint32_t boundaryEndLength, IScriptStackWalkVisitor* visitor)
{
	if (m_stackTraceRoutine)
	{
		char* out = nullptr;
		size_t outLen = 0;

		m_stackTraceRoutine(boundaryStart, boundaryEnd, &out, &outLen);

		if (out)
		{
			msgpack::unpacked up = msgpack::unpack(out, outLen);

			auto o = up.get().as<std::vector<msgpack::object>>();

			for (auto& e : o)
			{
				msgpack::sbuffer sb;
				msgpack::pack(sb, e);

				visitor->SubmitStackFrame(sb.data(), sb.size());
			}
		}
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::TriggerEvent(char* eventName, char* eventPayload, uint32_t payloadSize, char* eventSource)
{
	if (m_eventRoutine)
	{
		LuaPushEnvironment pushed(this);

		m_eventRoutine(eventName, eventPayload, payloadSize, eventSource);
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::CallRef(int32_t refIdx, char* argsSerialized, uint32_t argsLength, char** retvalSerialized, uint32_t* retvalLength)
{
	*retvalLength = 0;
	*retvalSerialized = nullptr;

	if (m_callRefRoutine)
	{
		LuaPushEnvironment pushed(this);

		size_t retvalLengthS;
		m_callRefRoutine(refIdx, argsSerialized, argsLength, retvalSerialized, &retvalLengthS);

		*retvalLength = retvalLengthS;
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::DuplicateRef(int32_t refIdx, int32_t* outRefIdx)
{
	*outRefIdx = -1;

	if (m_duplicateRefRoutine)
	{
		LuaPushEnvironment pushed(this);

		*outRefIdx = m_duplicateRefRoutine(refIdx);
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::RemoveRef(int32_t refIdx)
{
	if (m_deleteRefRoutine)
	{
		LuaPushEnvironment pushed(this);

		m_deleteRefRoutine(refIdx);
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::RequestMemoryUsage()
{
	// Lua instantly allows returning per-runtime GC memory load
	return FX_S_OK;
}

result_t LuaScriptRuntime::GetMemoryUsage(int64_t* memoryUsage)
{
	LuaPushEnvironment pushed(this);
	*memoryUsage = (int64_t(lua_gc(m_state, LUA_GCCOUNT, 0)) * 1024) + int64_t(lua_gc(m_state, LUA_GCCOUNTB, 0));

	return FX_S_OK;
}

result_t LuaScriptRuntime::SetScriptIdentifier(char* fileName, int32_t scriptId)
{
	m_scriptIds[fileName] = scriptId;

	return FX_S_OK;
}

result_t LuaScriptRuntime::SetDebugEventListener(IDebugEventListener* listener)
{
	m_debugListener = listener;

	return FX_S_OK;
}

void* LuaScriptRuntime::GetParentObject()
{
	return m_parentObject;
}

void LuaScriptRuntime::SetParentObject(void* object)
{
	m_parentObject = object;
}

#if LUA_VERSION_NUM == 504
// {91A81564-E5F1-4FD6-BC6A-9865A081011D}
FX_DEFINE_GUID(CLSID_LuaScriptRuntime,
0x91a81564, 0xe5f1, 0x4fd6, 0xbc, 0x6a, 0x98, 0x65, 0xa0, 0x81, 0x01, 0x1d);
#else
// {A7242855-0350-4CB5-A0FE-61021E7EAFAA}
FX_DEFINE_GUID(CLSID_LuaScriptRuntime,
0xa7242855, 0x350, 0x4cb5, 0xa0, 0xfe, 0x61, 0x2, 0x1e, 0x7e, 0xaf, 0xaa);
#endif

FX_NEW_FACTORY(LuaScriptRuntime);

FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptFileHandlingRuntime);
}

#if !defined(_DEBUG) && !defined(BUILD_LUA_SCRIPT_NATIVES)
	#define BUILD_LUA_SCRIPT_NATIVES
	#include "LuaScriptNatives.cpp"
#endif
