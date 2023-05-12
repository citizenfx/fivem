/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fxScripting.h"

#include <ScriptEngine.h>

#include "LuaScriptRuntime.h"
#include "LuaScriptNatives.h"
#include "ResourceScriptingComponent.h"

#include <msgpack.hpp>
#include <json.hpp>

#include <CoreConsole.h>
#include <Profiler.h>

#include <lua.hpp>
#include <lua_cmsgpacklib.h>
#include <lua_rapidjsonlib.h>
#include <lmprof_lib.h>
#if LUA_VERSION_NUM == 504
#include <lglmlib.hpp>
#endif

extern LUA_INTERNAL_LINKAGE
{
#include <lobject.h>

#include <lmprof_state.h>
#include <collections/lmprof_record.h>
#include <collections/lmprof_traceevent.h>
#include <collections/lmprof_stack.h>
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

// Additional flag to denote the profiler was created by IScriptProfiler
#define LMPROF_STATE_FX_PROFILER 0x80000000

// Utility for sanitizing error messages in an unprotected states. Avoid string coercion (cvt2str) to ensure errors 
// do not compound.
#define LUA_SCRIPT_TRACE(L, MSG, ...)                     \
    try                                                   \
    {                                                     \
        const char* err = "error object is not a string"; \
        if (lua_type((L), -1) == LUA_TSTRING)             \
        {                                                 \
            err = lua_tostring((L), -1);                  \
        }                                                 \
        ScriptTrace(MSG ": %s\n", ##__VA_ARGS__, err);    \
    }                                                     \
    catch (...)                                           \
    {                                                     \
    }                                                     \
    lua_pop((L), 1)

/// <summary>
/// </summary>
uint8_t g_metaFields[(size_t)LuaMetaFields::Max];

/// <summary>
/// </summary>
static fx::OMPtr<fx::LuaScriptRuntime> g_currentLuaRuntime;

/// <summary>
/// </summary>
static IScriptHost* g_lastScriptHost;

uint64_t g_tickTime;
bool g_hadProfiler;

#if defined(LUA_USE_RPMALLOC)
#include "rpmalloc/rpmalloc.h"

// Global static object handling rpmalloc initializing and finalizing
struct rpmalloc_GlobalGuard
{
	rpmalloc_GlobalGuard() { rpmalloc_initialize(); }
	~rpmalloc_GlobalGuard() { rpmalloc_finalize(); }
} static rp_global_guard;

lua_State* fx::LuaStateHolder::lua_rpmalloc_state(void*& opaque)
{
	static auto lua_rpmalloc = [](void* ud, void* ptr, size_t osize, size_t nsize) -> void*
	{
		heap_t* heap = static_cast<heap_t*>(ud);
		if (nsize == 0)
		{
			rpmalloc_heap_free(heap, ptr);
			return NULL;
		}
		else if (ptr == NULL)
		{
			return rpmalloc_heap_aligned_alloc(heap, 16, nsize);
		}
		else
		{
			return rpmalloc_heap_aligned_realloc(heap, ptr, 16, nsize, osize, 0);
		}
	};

	static auto lua_panic = [](lua_State* L) -> int
	{
		const char* msg = lua_tostring(L, -1);
		if (msg == NULL)
		{
			msg = "error object is not a string";
		}

		lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n", msg);
		return 0; /* return to Lua to abort */
	};

	heap_t* rpmalloc_heap = rpmalloc_heap_acquire();
	if (rpmalloc_heap != nullptr)
	{
		lua_State* L = lua_newstate(lua_rpmalloc, static_cast<void*>(rpmalloc_heap));
		if (L != nullptr)
		{
			opaque = static_cast<void*>(rpmalloc_heap);
			lua_atpanic(L, lua_panic);
		}
		else
		{
			rpmalloc_heap_release(rpmalloc_heap);
		}
		return L;
	}
	return nullptr;
}

void fx::LuaStateHolder::lua_rpmalloc_free(void* opaque)
{
	rpmalloc_heap_free_all(static_cast<heap_t*>(opaque));
	rpmalloc_heap_release(static_cast<heap_t*>(opaque));
}

#endif

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

#if LUA_VERSION_NUM >= 504
static void Lua_Warn(void* ud, const char* msg, int tocont)
{
	static bool cont = false;

	if (!cont)
	{
		bool newline = (msg[0] != '\0' && msg[strlen(msg) - 1] != '\n');
		console::PrintWarning(fmt::sprintf("script:%s:warning", LuaScriptRuntime::GetCurrent()->GetResourceName()), "%s%s", msg, newline ? "\n" : "");
	}
	else
	{
		console::Printf(fmt::sprintf("script:%s:warning", LuaScriptRuntime::GetCurrent()->GetResourceName()), "%s", msg);
	}

	cont = (tocont) ? true : false;
}
#endif
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

class LuaProfilerScope
{
private:
	fx::LuaScriptRuntime* luaRuntime;
	const bool begin;
	bool requiresClose;

public:
	LuaProfilerScope(fx::LuaScriptRuntime* luaRuntime_, bool begin_ = true)
		: luaRuntime(luaRuntime_), begin(begin_), requiresClose(false)
	{
		requiresClose = luaRuntime->IScriptProfiler_Tick(begin) != 0;
	}

	~LuaProfilerScope()
	{
		Close();
	}

	void LUA_INLINE Close()
	{
		if (requiresClose)
		{
			requiresClose = false;
			luaRuntime->IScriptProfiler_Tick(!begin);
		}
	}
};

static int Lua_Trace(lua_State* L)
{
	ScriptTrace("%s", luaL_checkstring(L, 1));
	return 0;
}

static int Lua_SetTickRoutine(lua_State* L)
{
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
		LuaProfilerScope _profile(luaRuntime);

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
			else if (auto thread = luaRuntime->GetRunningThread())
			{
				lua_pushthread(thread);
				lua_xmove(thread, L, 1);
			}
			else
			{
				lua_pushnil(L);
			}
		}
		else
		{
			lua_pushnil(L);

			if (auto thread = luaRuntime->GetRunningThread())
			{
				lua_pushthread(thread);
				lua_xmove(thread, L, 1);
			}
			else
			{
				lua_pushnil(L);
			}
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
			LUA_SCRIPT_TRACE(L, "Error running stack trace function for resource %s", luaRuntime->GetResourceName());

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

static int Lua_SetBoundaryRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetBoundaryRoutine(ref);

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
		LuaProfilerScope _profile(luaRuntime);

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
			LUA_SCRIPT_TRACE(L, "Error running system event handling function for resource %s", luaRuntime->GetResourceName());
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
		LuaProfilerScope _profile(luaRuntime);

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
			LUA_SCRIPT_TRACE(L, "Error running call reference function for resource %s", luaRuntime->GetResourceName());

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
		LuaProfilerScope _profile(luaRuntime);

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
			LUA_SCRIPT_TRACE(L, "Error running system ref deletion function for resource %s", luaRuntime->GetResourceName());
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
		LuaProfilerScope _profile(luaRuntime);

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
			LUA_SCRIPT_TRACE(L, "Error running system ref duplication function for resource %s", luaRuntime->GetResourceName());
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
	fx::OMPtr scriptHost = luaRuntime->GetScriptHost();
	LuaProfilerScope _profile(luaRuntime.GetRef(), false);

	// variables to hold state
	fxNativeContext context = { 0 };

	context.numArguments = 4;
	context.nativeIdentifier = HashString("INVOKE_FUNCTION_REFERENCE");

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

		_profile.Close();
		lua_pushstring(L, va("Execution of native %016x in script host failed: %s", 0xe3551879, error));
		return lua_error(L);
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

static int Lua_ResultAsObject(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetResultAsObjectRoutine([luaRuntime, ref](lua_State* L, std::string_view object)
	{
		LuaProfilerScope _profile(luaRuntime);

		// set the error handler
		lua_pushcfunction(L, luaRuntime->GetDbTraceback());

		int eh = lua_gettop(L);

		// get the referenced function
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// push arguments on the stack
		lua_pushlstring(L, object.data(), object.size());

		// invoke the tick routine
		if (lua_pcall(L, 1, 1, eh) != 0)
		{
			LUA_SCRIPT_TRACE(L, "Error running deserialization function for resource %s", luaRuntime->GetResourceName());
			lua_pushnil(L);
		}

		lua_remove(L, eh);
	});

	return Lua_GetMetaField<LuaMetaFields::ResultAsObject>(L);
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

	if (strcmp(name, LUA_LMPROF_LIBNAME) == 0)
	{
		luaL_requiref(L, LUA_LMPROF_LIBNAME, luaopen_lmprof, 1);
		return 1;
	}
#if LUA_VERSION_NUM >= 504
	else if (strcmp(name, LUA_GLMLIBNAME) == 0)
	{
		luaL_requiref(L, LUA_GLMLIBNAME, luaopen_glm, 1);
		return 1;
	}
#endif

	// @TODO: Consider implementing a custom 'loadlib' module that uses VFS,
	// for example, LoadSystemFile("citizen:/scripting/lua/json.lua"). Server
	// side luarocks integration would be pretty neat.

	return luaL_error(L, "module '%s' not found", name);
}

//
// Scheduling
//
lua_State* LuaScriptRuntime::GetRunningThread()
{
	if (!m_runningThreads.empty())
	{
		return m_runningThreads.front();
	}

	return nullptr;
}

static int Lua_Resume(lua_State* L)
{
	LuaScriptRuntime* lsRT = (LuaScriptRuntime*)lua_touserdata(L, lua_upvalueindex(1));

	auto bookmark = lua_tointeger(L, lua_upvalueindex(2));
	lsRT->RunBookmark(bookmark);

	return 0;
}

bool LuaScriptRuntime::RunBookmark(uint64_t bookmark)
{
	// --- get the ref
	lua_State* L = m_state;
	lua_rawgeti(L, LUA_REGISTRYINDEX, bookmark);
	// Lua stack: [t]

	// -- get the thread from the ref
	lua_rawgeti(L, -1, 1);
	// Lua stack: [coro, t]

	std::string_view name;
	lua_State* thread = lua_tothread(L, -1);

	// -- boundary ID
	lua_pop(L, 1);

	lua_rawgeti(L, -1, 3);
	// Lua stack: [bid, t]

	auto bid = lua_tointeger(L, -1);

	// -- profiler meta (name)
	bool hadProfiler = g_hadProfiler;

	if (hadProfiler)
	{
		lua_pop(L, 1);
		// Lua stack: [t]

		// -- get the name from the ref
		lua_rawgeti(L, -1, 2);
		// Lua stack: [name, t]

		{
			size_t len = 0;
			const char* nameRef = lua_tolstring(L, -1, &len);

			if (nameRef)
			{
				name = { nameRef, len };
			}
		}
	}

	// since we're done getting stuff, remove both 't' and the last field
	lua_pop(L, 2);
	// Lua stack: []

	// --- if coroutine status is dead...
	bool coroDead = false;

	{
		auto status = lua_status(thread);

		if (status == LUA_OK)
		{
			lua_Debug ar;
			if (lua_getstack(thread, 0, &ar) <= 0 && lua_gettop(thread) == 0)
			{
				coroDead = true;
			}
		}
		else if (status != LUA_YIELD)
		{
			coroDead = true;
		}
	}

	// -- kill the thread.
	if (coroDead)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, bookmark);
		return false;
	}

	// -- running stack
	m_runningThreads.push_front(thread);

	// --- enter profiler scope
	if (hadProfiler)
	{
		static auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		profiler->EnterScope(std::string{ name });
	}

	// --- submit boundary start
	{
		LuaBoundary b;
		b.hint = bid;
		b.thread = thread;

		m_scriptHost->SubmitBoundaryStart((char*)&b, sizeof(b));
	}

#if LUA_VERSION_NUM >= 504
	int nrv;
	int resumeValue = lua_resume(thread, L, 0, &nrv);
#else
	int resumeValue = lua_resume(thread, L, 0);
#endif

	if (resumeValue == LUA_YIELD)
	{
		auto type = lua_type(thread, -1);

		if (type == LUA_TNUMBER || type == LUA_TNIL)
		{
			auto wakeTime = (type == LUA_TNUMBER) ? (int64_t)lua_tonumber(thread, -1) : 0;
			lua_pop(thread, 1);

			m_bookmarkHost->ScheduleBookmark(this, bookmark, -wakeTime);
		}
		else if (type == LUA_TLIGHTUSERDATA)
		{
			auto userData = lua_touserdata(thread, -1);
			lua_pop(thread, 1);

			if (userData == &g_metaFields[(int)LuaMetaFields::AwaitSentinel])
			{
				// resume again with a reattach callback
				lua_pushlightuserdata(thread, this);
				// Lua stack: [runtime]

				lua_pushinteger(thread, bookmark);
				// Lua stack: [bookmark, runtime]

				lua_pushcclosure(thread, Lua_Resume, 2);
				// Lua stack: [resume func]

#if LUA_VERSION_NUM >= 504
				resumeValue = lua_resume(thread, L, 1, &nrv);
#else
				resumeValue = lua_resume(thread, L, 1);
#endif

				// if LUA_YIELD, cya later!
				if (resumeValue != LUA_YIELD)
				{
					// bye, thread
					luaL_unref(L, LUA_REGISTRYINDEX, bookmark);
				}
			}
		}
	}
	else
	{
		if (resumeValue != LUA_OK)
		{
			std::string err = "error object is not a string";
			if (lua_type(thread, -1) == LUA_TSTRING)
			{
				err = lua_tostring(thread, -1);
			}                        

			static auto formatStackTrace = fx::ScriptEngine::GetNativeHandler(HashString("FORMAT_STACK_TRACE"));
			auto stack = FxNativeInvoke::Invoke<const char*>(formatStackTrace, nullptr, 0);
			std::string stackData = "(nil stack trace)";
			
			if (stack)
			{
				stackData = stack;
			}
			ScriptTrace("^1SCRIPT ERROR: %s^7\n", err);
			ScriptTrace("%s", stackData);
		}

		luaL_unref(L, LUA_REGISTRYINDEX, bookmark);
	}

	m_runningThreads.pop_front();

	if (hadProfiler)
	{
		static auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		profiler->ExitScope();
	}

	return (resumeValue == LUA_YIELD);
}

int Lua_Wait(lua_State* L)
{
	// push argument 1
	lua_pushvalue(L, 1);

	// yield with argument
	lua_yield(L, 1);

	// no return values
	return 0;
}

static int Lua_CreateThreadInternal(lua_State* L, bool now, int timeout, int funcArg = 1, const std::string& nameOverride = {})
{
	// Lua stack: [a1]
	const auto& luaRuntime = LuaScriptRuntime::GetCurrent();
	
	// --- get debug info
	lua_pushvalue(L, 1);
	// Lua stack: [a1, a1]

	lua_Debug dbgInfo;
	lua_getinfo(L, ">S", &dbgInfo);
	// Lua stack: [a1]

	// -- format and store debug info
	auto name = (nameOverride.empty())
		? fmt::sprintf("thread %s[%d..%d]", dbgInfo.short_src, dbgInfo.linedefined, dbgInfo.lastlinedefined)
		: nameOverride;

	// --- create coroutine
	lua_State* thread = lua_newthread(L);
	// Lua stack: [thread, a1]

	lua_rawgeti(L, LUA_REGISTRYINDEX, luaRuntime->GetBoundaryRoutine());
	// Lua stack: [br, thread, a1]

	lua_pushvalue(L, funcArg);
	// Lua stack: [a1, br, thread, a1]

	lua_call(L, 1, 2);
	// Lua stack: [fn, bid, thread, a1]
	
	lua_xmove(L, thread, 1);
	// Lua stack: [bid, thread, a1]
	// thread stack: [fn]

	// -- get boundary ID
	auto bid = lua_tointeger(L, -1);
	lua_pop(L, 1);
	// Lua stack: [thread, a1]

	// --- store name and coroutine

	lua_createtable(L, 3, 0);
	// Lua stack: [t, thread, a1]

	// -- set thread

	lua_pushvalue(L, -2);
	// Lua stack: [thread, t, thread, a1]

	// t[1] = thread
	lua_rawseti(L, -2, 1);
	// Lua stack: [t, thread, a1]

	// -- set name

	lua_pushlstring(L, name.c_str(), name.length());
	// Lua stack: [name, t, thread, a1]

	// t[2] = name
	lua_rawseti(L, -2, 2);
	// Lua stack: [t, thread, a1]

	// -- set bid
	lua_pushinteger(L, bid);
	// Lua stack: [bid, t, thread, a1]

	// t[2] = name
	lua_rawseti(L, -2, 3);
	// Lua stack: [t, thread, a1]

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	// Lua stack: [thread, a1]

	lua_pop(L, 1);

	// Lua stack: [a1]
	// thread stack: same

	if (!now)
	{
		luaRuntime->ScheduleBookmarkSoon(ref, -timeout);
		return 0;
	}
	else
	{
		lua_pushboolean(L, luaRuntime->RunBookmark(ref));
		return 1;
	}
}

static int Lua_CreateThread(lua_State* L)
{
	return Lua_CreateThreadInternal(L, false, 0);
}

static int Lua_CreateThreadNow(lua_State* L)
{
	std::string name;

	if (lua_gettop(L) >= 2)
	{
		name = lua_tostring(L, 2);
	}

	return Lua_CreateThreadInternal(L, true, 0, 1, name);
}

static int Lua_SetTimeout(lua_State* L)
{
	auto timeout = (int)luaL_checknumber(L, 1);
	return Lua_CreateThreadInternal(L, false, timeout, 2);
}

static int Lua_Noop(lua_State* L)
{
	return 0;
}

static const struct luaL_Reg g_citizenLib[] = {
	{ "SetBoundaryRoutine", Lua_SetBoundaryRoutine },
	{ "SetTickRoutine", Lua_SetTickRoutine },
	{ "SetEventRoutine", Lua_SetEventRoutine },
	{ "Trace", Lua_Trace },
	{ "CreateThread", Lua_CreateThread },
	{ "CreateThreadNow", Lua_CreateThreadNow },
	{ "Wait", Lua_Wait },
	{ "SetTimeout", Lua_SetTimeout },
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
	{ "ResultAsObject", Lua_Noop }, // for compatibility
	{ "ResultAsObject2", Lua_ResultAsObject },
	{ "AwaitSentinel", Lua_GetMetaField<LuaMetaFields::AwaitSentinel> },
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
		g_currentLuaRuntime->SchedulePendingBookmarks();
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

void LuaScriptRuntime::ScheduleBookmarkSoon(uint64_t bookmark, int timeout)
{
	m_pendingBookmarks.emplace_back(bookmark, timeout);
}

void LuaScriptRuntime::SchedulePendingBookmarks()
{
	if (!m_pendingBookmarks.empty())
	{
		for (auto [ref, timeout] : m_pendingBookmarks)
		{
			GetScriptHostWithBookmarks()->ScheduleBookmark(this, ref, timeout);
		}

		m_pendingBookmarks.clear();
	}
}

void LuaScriptRuntime::SetTickRoutine(const std::function<void(uint64_t, bool)>& tickRoutine)
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

	{
		fx::OMPtr<IScriptHost> ptr(scriptHost);
		fx::OMPtr<IScriptHostWithBookmarks> resourcePtr;
		ptr.As(&resourcePtr);

		m_bookmarkHost = resourcePtr.GetRef();
		m_bookmarkHost->CreateBookmarks(this);
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

	// Graph script loaded into Citizen.Graph
	// @TODO: Only load graphing utility on Lua_Require
	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/graph.lua")))
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

#if LUA_VERSION_NUM >= 504
	lua_setwarnf(m_state, Lua_Warn, nullptr);
#endif

	return FX_S_OK;
}

extern bool mountedAnyNatives;

result_t LuaScriptRuntime::LoadNativesBuild(const std::string& nativesBuild)
{
	result_t hr = FX_S_OK;

	bool useLazyNatives = mountedAnyNatives;

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
	if (m_bookmarkHost)
	{
		m_bookmarkHost->RemoveBookmarks(this);
	}

	// destroy any routines that may be referencing the Lua state
	m_eventRoutine = TEventRoutine();
	m_tickRoutine = {};
	m_callRefRoutine = TCallRefRoutine();
	m_deleteRefRoutine = TDeleteRefRoutine();
	m_duplicateRefRoutine = TDuplicateRefRoutine();

	// we need to push the environment before closing as items may have __gc callbacks requiring a current runtime to be set
	// in addition, we can't do this in the destructor due to refcounting oddities (PushEnvironment adds a reference, causing infinite deletion loops)
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

	std::vector<char> fileData(length);
	if (FX_FAILED(hr = stream->Read(&fileData[0], length, nullptr)))
	{
		return hr;
	}

	std::string_view fn = scriptFile;
	if (fn.length() > 1 && fn[0] == '@' && fn.find_first_of('/') != std::string::npos)
	{
		std::string_view resName = fn.substr(1, fn.find_first_of('/') - 1);
		fn = fn.substr(1 + resName.length() + 1);

		auto resourceManager = fx::ResourceManager::GetCurrent();
		auto resource = resourceManager->GetResource(std::string(resName));

		if (resource.GetRef())
		{
			resource->OnBeforeLoadScript(&fileData);
		}
	}

	fx::Resource* resource = reinterpret_cast<fx::Resource*>(GetParentObject());
	resource->OnBeforeLoadScript(&fileData);

	length = fileData.size();

	fileData.push_back('\0');

	// create a chunk name prefixed with @ (suppresses '[string "..."]' formatting)
	fwString chunkName("@");
	chunkName.append(scriptFile);

	if (luaL_loadbuffer(m_state, &fileData[0], length, chunkName.c_str()) != 0)
	{
		LUA_SCRIPT_TRACE(m_state, "Error parsing script %s in resource %s", scriptFile, GetResourceName());

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
		LUA_SCRIPT_TRACE(m_state, "Error loading script %s in resource %s", scriptName, GetResourceName());

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

result_t LuaScriptRuntime::TickBookmarks(uint64_t* bookmarks, int numBookmarks)
{
	if (numBookmarks > 0)
	{
		LuaPushEnvironment pushed(this);
		LuaProfilerScope _profile(this);

		for (auto bookmarkIdx = 0; bookmarkIdx < numBookmarks; bookmarkIdx++)
		{
			RunBookmark(bookmarks[bookmarkIdx]);
		}
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

result_t LuaScriptRuntime::EmitWarning(char* channel, char* message)
{
#if LUA_VERSION_NUM >= 504
	lua_warning(m_state, va("[%s] %s", channel, message), 0);
	return FX_S_OK;
#else
	// Lua < 5.4 does not support warnings
	return FX_E_NOTIMPL;
#endif
}

void* LuaScriptRuntime::GetParentObject()
{
	return m_parentObject;
}

void LuaScriptRuntime::SetParentObject(void* object)
{
	m_parentObject = object;
}

/// <summary>
/// Helper function for initializing the fx::ProfilerComponent bridge.
/// </summary>
static LuaProfilingMode IScriptProfiler_Initialize(lua_State* L, int m_profilingId);

bool LuaScriptRuntime::IScriptProfiler_Tick(bool begin)
{
	// Preempt
	if (m_profilingMode == LuaProfilingMode::None)
	{
		return false;
	}

	lua_State* L = m_state.Get();
	switch (m_profilingMode)
	{
		case LuaProfilingMode::None:
			break;

		// Flagged to initialize the fx::ProfilerComponent bridge.
		case LuaProfilingMode::Setup:
		{
			m_profilingMode = IScriptProfiler_Initialize(L, m_profilingId);
			return begin && m_profilingMode == LuaProfilingMode::Profiling; // Requires an 'exit' scope.
		}

		// The Lua runtime is either resuming execution or yielding until the
		// next Tick/Event/CallRef routine.
		//
		// To improve clarity of the DevTools timeline: artificial ENTER_SCOPE/EXIT_SCOPE
		// events are generated to ensure DevTools only renders timeline bars
		// while the script runtime is active.
		case LuaProfilingMode::Profiling:
		{
			lmprof_State* st = lmprof_singleton(L);
			return begin ? lmprof_resume_execution(L, st)
						 : lmprof_pause_execution(L, st);
		}

		// Flagged to shutdown the profiler.
		case LuaProfilingMode::Shutdown:
		{
			m_profilingId = 0;
			m_profilingMode = LuaProfilingMode::None;

			// Ensure the profiler was initialized by IScriptProfiler. Must also
			// consider the edge-case of it being preempted by the exposed
			// lmprof.quit script function.
			lmprof_State* st = lmprof_singleton(L);
			if (st != l_nullptr && BITFIELD_TEST(st->state, LMPROF_STATE_FX_PROFILER))
			{
				lmprof_finalize_profiler(L, st, 0);
				lmprof_shutdown_profiler(L, st);
				if (lua_gc(L, LUA_GCISRUNNING, 0))
				{
					// Force collect & restart the garbage collection, ensuring
					// all profiler overheads have been managed.
					lua_gc(L, LUA_GCCOLLECT, 0);
					lua_gc(L, LUA_GCRESTART, 0);
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
	return false;
}

result_t LuaScriptRuntime::SetupFxProfiler(void* obj, int32_t resourceId)
{
	lua_State* L = m_state.Get();
	if (L != l_nullptr
		&& lua_gethook(L) == l_nullptr // will be replacing debug.sethook: avoid it
		&& lmprof_singleton(L) == l_nullptr) // Invalid profiler state: already being profiled.
	{
		m_profilingId = resourceId;
		m_profilingMode = LuaProfilingMode::Setup; // @TODO: See comment in ShutdownFxProfiler.
		return FX_S_OK;
	}

	return FX_E_INVALIDARG; // @TODO: Eventually create a unique error code.
}

result_t LuaScriptRuntime::ShutdownFxProfiler()
{
	if (m_profilingMode == LuaProfilingMode::Profiling)
		m_profilingMode = LuaProfilingMode::Shutdown;
	else
		m_profilingMode = LuaProfilingMode::None;

	return FX_S_OK;
}

/// <summary>
/// Data required by lmprof to forward/bridge events to fx::ProfilerComponent.
/// </summary>
struct ProfilerState
{
	const fwRefContainer<fx::ProfilerComponent>& profiler;
};

static LuaProfilingMode IScriptProfiler_Initialize(lua_State* L, int m_profilingId)
{
	const fwRefContainer<fx::ProfilerComponent>& profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
	lmprof_clock_init();

	lmprof_State* st = lmprof_new(L, LMPROF_MODE_EXT_CALLBACK | LMPROF_MODE_INSTRUMENT | LMPROF_MODE_MEMORY, lmprof_default_error);
	st->state |= LMPROF_STATE_FX_PROFILER;
	st->conf = LMPROF_OPT_COMPRESS_GRAPH | LMPROF_OPT_LOAD_STACK | LMPROF_OPT_STACK_MISMATCH | LMPROF_OPT_GC_COUNT_INIT;
	st->thread.mainproc.pid = TRACE_PROCESS_MAIN;
	st->thread.mainproc.tid = m_profilingId;
	st->i.counterFrequency = 1;

	// fx::ProfilerComponent uses std::chrono::[...].time_since_epoch(). Timing
	// function requires hooking.
	st->time = []() -> lu_time
	{
		return static_cast<lu_time>(fx::usec().count());
	};

	/// Initialize profiler state
	st->i.trace.arg = new ProfilerState{ profiler };
	st->i.trace.free = [](lua_State* L, void* args)
	{
		ProfilerState* state = static_cast<ProfilerState*>(args);
		if (state != nullptr)
		{
			delete state;
		}
	};

	/// Entering/Exiting a function: map to ENTER_SCOPE/EXIT_SCOPE.
	st->i.trace.scope = [](lua_State* L, lmprof_State* st, struct lmprof_StackInst* inst, int enter) -> int
	{
		ProfilerState* state = static_cast<ProfilerState*>(st->i.trace.arg);

		// Measurements
		const fx::ProfilerEvent::thread_t tid(st->thread.mainproc.tid);
		const std::chrono::microseconds when = std::chrono::microseconds(inst->trace.call.s.time);

		// To reduce bloat within the fx::Profiler output, periodically report
		// memory events.
		fx::ProfilerEvent::memory_t much = 0;
		if (--st->i.counterFrequency <= 0)
		{
			much = unit_allocated(&inst->trace.call.s);
			st->i.counterFrequency = 16;
		}

		// Do not report the 'root' (dummy) record: That timeline event will be
		// covered by trace.routine.
		if (BITFIELD_TEST(inst->trace.record->info.event, LMPROF_RECORD_IGNORED | LMPROF_RECORD_ROOT))
		{
		}
		else if (enter)
		{
			const char* source = inst->trace.record->info.source;
			const std::string name = (source == NULL) ? "?" : source;
			state->profiler->PushEvent(tid, ProfilerEventType::ENTER_SCOPE, when, name, "", much);
		}
		else
		{
			state->profiler->PushEvent(tid, ProfilerEventType::EXIT_SCOPE, when, much);
		}

		return TRACE_EVENT_OK;
	};

	/// Resuming/Yielding a coroutine. In lmprof these callbacks have their own
	/// specialized TraceEvent format. This implementation will treat them as
	/// scope events.
	st->i.trace.routine = [](lua_State* L, lmprof_State* st, lmprof_EventProcess thread, int begin) -> int
	{
		ProfilerState* state = static_cast<ProfilerState*>(st->i.trace.arg);

		// Measurements
		const std::chrono::microseconds when = std::chrono::microseconds(st->thread.r.s.time);
		const fx::ProfilerEvent::memory_t much(unit_allocated(&st->thread.r.s));
		const fx::ProfilerEvent::thread_t tid(st->thread.mainproc.tid);

		if (begin)
		{
			const std::string name = lmprof_thread_name(L, thread.tid, "Thread");
			state->profiler->PushEvent(tid, ProfilerEventType::ENTER_SCOPE, when, name, "", much);
		}
		else
		{
			state->profiler->PushEvent(tid, ProfilerEventType::EXIT_SCOPE, when, much);
		}

		return TRACE_EVENT_OK;
	};

	// If hook initialization failed the profiler userdata will be thrown to the
	// garbage collector.
	LuaProfilingMode result = LuaProfilingMode::None;
	if (lmprof_initialize_only_hooks(L, st, -1))
	{
		result = LuaProfilingMode::Profiling;
	}

	lua_pop(L, 1);
	return result;
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
