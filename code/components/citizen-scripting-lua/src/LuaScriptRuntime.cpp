/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fxScripting.h"

#include <Resource.h>
#include <ManifestVersion.h>

#include <msgpack.hpp>
#include <json.hpp>

#include <CoreConsole.h>

using json = nlohmann::json;

#if defined(GTA_FIVE)
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
	{ "natives_21e43a33.lua",  guid_t{0} },
	{ "natives_0193d0af.lua",  "f15e72ec-3972-4fe4-9c7d-afc5394ae207" },
	{ "natives_universal.lua", "44febabe-d386-4d18-afbe-5e627f4af937" }
};

// we fast-path non-FXS using direct RAGE calls
#include <scrEngine.h>
#elif defined(IS_RDR3)
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
	{ "rdr3_universal.lua", guid_t{ 0 } }
};

// we fast-path non-FXS using direct RAGE calls
#include <scrEngine.h>
#else
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
	{ "natives_server.lua", guid_t{ 0 } }
};
#endif

#include <lua.hpp>

extern "C" {
#include <lobject.h>
}

#include <om/OMComponent.h>

namespace fx
{
class LuaStateHolder
{
private:
	lua_State* m_state;

public:
	LuaStateHolder()
	{
		m_state = luaL_newstate();
	}

	~LuaStateHolder()
	{
		Close();
	}

	void Close()
	{
		if (m_state)
		{
			lua_close(m_state);

			m_state = nullptr;
		}
	}

	operator lua_State*()
	{
		return m_state;
	}

	inline lua_State* Get()
	{
		return m_state;
	}
};

struct PointerFieldEntry
{
	bool empty;
	uintptr_t value;

	PointerFieldEntry()
	{
		empty = true;
	}
};

struct PointerField
{
	PointerFieldEntry data[64];
};

class LuaScriptRuntime : public OMClass<LuaScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime, IScriptRefRuntime, IScriptMemInfoRuntime, IScriptStackWalkingRuntime, IScriptDebugRuntime>
{
private:
	typedef std::function<void(const char*, const char*, size_t, const char*)> TEventRoutine;

	typedef std::function<void(int32_t, const char*, size_t, char**, size_t*)> TCallRefRoutine;

	typedef std::function<int32_t(int32_t)> TDuplicateRefRoutine;

	typedef std::function<void(int32_t)> TDeleteRefRoutine;

	typedef std::function<void(void*, void*, char**, size_t*)> TStackTraceRoutine;

private:
	LuaStateHolder m_state;

	IScriptHost* m_scriptHost;

	IScriptHostWithResourceData* m_resourceHost;

	IScriptHostWithManifest* m_manifestHost;

	OMPtr<IDebugEventListener> m_debugListener;

	std::function<void()> m_tickRoutine;

	TEventRoutine m_eventRoutine;

	TCallRefRoutine m_callRefRoutine;

	TDuplicateRefRoutine m_duplicateRefRoutine;

	TDeleteRefRoutine m_deleteRefRoutine;

	TStackTraceRoutine m_stackTraceRoutine;

	void* m_parentObject;

	PointerField m_pointerFields[3];

	int m_instanceId;

	std::string m_nativesDir;

	std::unordered_map<std::string, int> m_scriptIds;

public:
	inline LuaScriptRuntime()
	{
		m_instanceId = rand();
	}

	virtual ~LuaScriptRuntime() override;

	static const OMPtr<LuaScriptRuntime>& GetCurrent();

	void SetTickRoutine(const std::function<void()>& tickRoutine);

	void SetEventRoutine(const TEventRoutine& eventRoutine);

	inline void SetCallRefRoutine(const TCallRefRoutine& routine)
	{
		if (!m_callRefRoutine)
		{
			m_callRefRoutine = routine;
		}
	}

	inline void SetDuplicateRefRoutine(const TDuplicateRefRoutine& routine)
	{
		if (!m_duplicateRefRoutine)
		{
			m_duplicateRefRoutine = routine;
		}
	}

	inline void SetDeleteRefRoutine(const TDeleteRefRoutine& routine)
	{
		if (!m_deleteRefRoutine)
		{
			m_deleteRefRoutine = routine;
		}
	}

	inline void SetStackTraceRoutine(const TStackTraceRoutine& routine)
	{
		if (!m_stackTraceRoutine)
		{
			m_stackTraceRoutine = routine;
		}
	}

	inline IScriptHost* GetScriptHost()
	{
		return m_scriptHost;
	}

	inline IScriptHostWithResourceData* GetScriptHost2()
	{
		return m_resourceHost;
	}

	inline PointerField* GetPointerFields()
	{
		return m_pointerFields;
	}

	inline const char* GetResourceName()
	{
		char* resourceName = "";
		m_resourceHost->GetResourceName(&resourceName);

		return resourceName;
	}

	inline std::string GetNativesDir()
	{
		return m_nativesDir;
	}

private:
	result_t LoadFileInternal(OMPtr<fxIStream> stream, char* scriptFile);

	result_t LoadHostFileInternal(char* scriptFile);

	result_t LoadSystemFileInternal(char* scriptFile);

	result_t RunFileInternal(char* scriptFile, std::function<result_t(char*)> loadFunction);

	result_t LoadSystemFile(char* scriptFile);

	result_t LoadNativesBuild(const std::string& nativeBuild);

public:
	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;

	NS_DECL_ISCRIPTTICKRUNTIME;

	NS_DECL_ISCRIPTEVENTRUNTIME;

	NS_DECL_ISCRIPTREFRUNTIME;

	NS_DECL_ISCRIPTMEMINFORUNTIME;

	NS_DECL_ISCRIPTSTACKWALKINGRUNTIME;

	NS_DECL_ISCRIPTDEBUGRUNTIME;
};

static OMPtr<LuaScriptRuntime> g_currentLuaRuntime;

static IScriptHost* g_lastScriptHost;

class LuaPushEnvironment
{
private:
	fx::PushEnvironment m_pushEnvironment;

	OMPtr<LuaScriptRuntime> m_lastLuaRuntime;

public:
	inline LuaPushEnvironment(LuaScriptRuntime* runtime)
		: m_pushEnvironment(runtime)
	{
		g_lastScriptHost = runtime->GetScriptHost();

		m_lastLuaRuntime = g_currentLuaRuntime;
		g_currentLuaRuntime = runtime;
	}

	inline ~LuaPushEnvironment()
	{
		g_currentLuaRuntime = m_lastLuaRuntime;
	}
};

LuaScriptRuntime::~LuaScriptRuntime()
{
	
}

static int lua_error_handler(lua_State* L);

lua_CFunction Lua_GetNative(lua_State* L, const char* name);

const OMPtr<LuaScriptRuntime>& LuaScriptRuntime::GetCurrent()
{
#if _DEBUG
	LuaScriptRuntime* luaRuntime;
	OMPtr<IScriptRuntime> runtime;

	assert(FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)));
	assert(luaRuntime = dynamic_cast<LuaScriptRuntime*>(runtime.GetRef()));

	assert(luaRuntime == g_currentLuaRuntime.GetRef());
#endif

	return g_currentLuaRuntime;
}

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

// luaL_openlibs version without io/os libs
static const luaL_Reg lualibs[] =
{
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
	{ NULL, NULL }
};

LUALIB_API void safe_openlibs(lua_State *L)
{
	const luaL_Reg *lib = lualibs;
	for (; lib->func; lib++)
	{
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);
	}
}

static int Lua_SetTickRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the tick callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();
	
	luaRuntime->SetTickRoutine([=] ()
	{
		// set the error handler
		//lua_pushcfunction(L, lua_error_handler);

		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_replace(L, -2);

		int eh = lua_gettop(L);

		// get the referenced function
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// invoke the tick routine
		if (lua_pcall(L, 0, 0, eh) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			ScriptTrace("Error running system tick function for resource %s: %s\n", luaRuntime->GetResourceName(), err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

void LuaScriptRuntime::SetTickRoutine(const std::function<void()>& tickRoutine)
{
	if (!m_tickRoutine)
	{
		m_tickRoutine = tickRoutine;
	}
}

struct LuaBoundary
{
	int hint;
	lua_State* thread;
};

static int Lua_SetStackTraceRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the tick callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetStackTraceRoutine([=](void* start, void* end, char** blob, size_t* size)
	{
		// static array for retval output (sadly)
		static std::vector<char> retvalArray(32768);

		// set the error handler
		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_replace(L, -2);

		int eh = lua_gettop(L);

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

	luaRuntime->SetEventRoutine([=] (const char* eventName, const char* eventPayload, size_t payloadSize, const char* eventSource)
	{
		// set the error handler
		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_replace(L, -2);

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

void LuaScriptRuntime::SetEventRoutine(const TEventRoutine& eventRoutine)
{
	if (!m_eventRoutine)
	{
		m_eventRoutine = eventRoutine;
	}
}

static int Lua_SetCallRefRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetCallRefRoutine([=] (int32_t refId, const char* argsSerialized, size_t argsSize, char** retval, size_t* retvalLength)
	{
		// static array for retval output (sadly)
		static std::vector<char> retvalArray(32768);

		// set the error handler
		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_replace(L, -2);

		int eh = lua_gettop(L);

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

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetDeleteRefRoutine([=] (int32_t refId)
	{
		// set the error handler
		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_replace(L, -2);

		int eh = lua_gettop(L);

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

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent().GetRef();

	luaRuntime->SetDuplicateRefRoutine([=] (int32_t refId)
	{
		// set the error handler
		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_replace(L, -2);

		int eh = lua_gettop(L);

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
	scriptHost->InvokeNative(context);

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

	auto val = lua_tointeger(L, 1);
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

	auto val = lua_tointeger(L, 1);
	lua_State* thread = lua_tothread(L, 2);

	LuaBoundary b;
	b.hint = val;
	b.thread = thread;

	scriptHost->SubmitBoundaryEnd((char*)& b, sizeof(b));

	return 0;
}

int Lua_Trace(lua_State* L)
{
	ScriptTrace("%s", luaL_checkstring(L, 1));

	return 0;
}

enum class LuaMetaFields
{
	PointerValueInt,
	PointerValueFloat,
	PointerValueVector,
	ReturnResultAnyway,
	ResultAsInteger,
	ResultAsLong,
	ResultAsFloat,
	ResultAsString,
	ResultAsVector,
	ResultAsObject,
	Max
};

static uint8_t g_metaFields[(int)LuaMetaFields::Max];

struct scrObject
{
	const char* data;
	uintptr_t length;
};

// padded vector struct
struct scrVector
{
    float x;

private:
    uint32_t pad0;

public:
    float y;

private:
    uint32_t pad1;

public:
    float z;

private:
    uint32_t pad2;
};

int Lua_InvokeNative(lua_State* L)
{
	// get the hash
	uint64_t hash = lua_tointeger(L, 1);

#ifdef GTA_FIVE
	// hacky super fast path for 323 GET_HASH_KEY in GTA
	if (hash == 0xD24D37CC275948CC)
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
	auto& luaRuntime = LuaScriptRuntime::GetCurrent();
	auto scriptHost = luaRuntime->GetScriptHost();

	auto pointerFields = luaRuntime->GetPointerFields();

	// variables to hold state
	fxNativeContext context = { 0 };

	// return values and their types
	int numReturnValues = 0;
	uintptr_t retvals[16];
	LuaMetaFields rettypes[16];

	// coercion for the result value
	LuaMetaFields returnValueCoercion = LuaMetaFields::Max;

	// flag to return a result even if a pointer return value is passed
	bool returnResultAnyway = false;

	// get argument count for the loop
	int numArgs = lua_gettop(L);

	context.nativeIdentifier = hash;

	// pushing function
	auto push = [&] (const auto& value)
	{
		using TVal = std::decay_t<decltype(value)>;

		if constexpr (sizeof(TVal) < sizeof(uintptr_t))
		{
			*reinterpret_cast<uintptr_t*>(&context.arguments[context.numArguments]) = 0;
		}

		*reinterpret_cast<TVal*>(&context.arguments[context.numArguments]) = value;
		context.numArguments++;
	};

	// the big argument loop
	for (int arg = 2; arg <= numArgs; arg++)
	{
		// get the type and decide what to do based on it
		const auto value = lua_getvalue(L, arg);
		int type = lua_valuetype(L, value);

		// nil: add '0'
		switch (type)
		{
		// nil
		case LUA_TNIL:
		{
			push(0);
			break;
		}
		// integer/float
		case LUA_TNUMBER:
		{
			if (lua_valueisinteger(L, value))
			{
				push(lua_valuetointeger(L, value));
			}
			else if (lua_valueisfloat(L, value))
			{
				push(static_cast<float>(lua_valuetonumber(L, value)));
			}
			break;
		}
		// boolean
		case LUA_TBOOLEAN:
			push(lua_valuetoboolean(L, value));
			break;
			// table (high-level class with __data field)
		case LUA_TTABLE:
		{
			lua_pushstring(L, "__data");
			lua_rawget(L, arg);

			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				push(lua_tointeger(L, -1));
				lua_pop(L, 1);
			}
			else
			{
				lua_pushstring(L, "Invalid Lua type in __data");
				lua_error(L);
			}

			break;
		}
		// string
		case LUA_TSTRING:
		{
			push(lua_valuetostring(L, value));
			break;
		}
		// vectors
		case LUA_TVECTOR2:
		{
			auto f4 = lua_valuetofloat4(L, value);

			push(f4.x);
			push(f4.y);

			break;
		}
		case LUA_TVECTOR3:
		{
			auto f4 = lua_valuetofloat4(L, value);

			push(f4.x);
			push(f4.y);
			push(f4.z);

			break;
		}
		case LUA_TVECTOR4:
		case LUA_TQUAT:
		{
			auto f4 = lua_valuetofloat4(L, value);

			push(f4.x);
			push(f4.y);
			push(f4.z);
			push(f4.w);

			break;
		}
		// metafield
		case LUA_TLIGHTUSERDATA:
		{
			auto pushPtr = [&](LuaMetaFields metaField)
			{
				if (numReturnValues >= _countof(retvals))
				{
					lua_pushstring(L, "too many return value arguments");
					lua_error(L);
				}

				// push the offset and set the type
				push(&retvals[numReturnValues]);
				rettypes[numReturnValues] = metaField;

				// increment the counter
				if (metaField == LuaMetaFields::PointerValueVector)
				{
					numReturnValues += 3;
				}
				else
				{
					numReturnValues += 1;
				}
			};

			uint8_t* ptr = reinterpret_cast<uint8_t*>(lua_valuetouserdata(L, value));

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
					retvals[numReturnValues] = 0;

					if (metaField == LuaMetaFields::PointerValueVector)
					{
						retvals[numReturnValues + 1] = 0;
						retvals[numReturnValues + 2] = 0;
					}

					pushPtr(metaField);

					break;
				}
				case LuaMetaFields::ReturnResultAnyway:
					returnResultAnyway = true;
					break;
				case LuaMetaFields::ResultAsInteger:
				case LuaMetaFields::ResultAsLong:
				case LuaMetaFields::ResultAsString:
				case LuaMetaFields::ResultAsFloat:
				case LuaMetaFields::ResultAsVector:
				case LuaMetaFields::ResultAsObject:
					returnValueCoercion = metaField;
					break;
				}
			}
			// or if the pointer is a runtime pointer field
			else if (ptr >= reinterpret_cast<uint8_t*>(pointerFields) && ptr < (reinterpret_cast<uint8_t*>(pointerFields) + (sizeof(PointerField) * 2)))
			{
				// guess the type based on the pointer field type
				intptr_t ptrField = ptr - reinterpret_cast<uint8_t*>(pointerFields);
				LuaMetaFields metaField = static_cast<LuaMetaFields>(ptrField / sizeof(PointerField));

				if (metaField == LuaMetaFields::PointerValueInt || metaField == LuaMetaFields::PointerValueFloat)
				{
					auto ptrFieldEntry = reinterpret_cast<PointerFieldEntry*>(ptr);

					retvals[numReturnValues] = ptrFieldEntry->value;
					ptrFieldEntry->empty = true;

					pushPtr(metaField);
				}
			}
			else
			{
				push(ptr);
			}

			break;
		}
		default:
		{
			lua_pushstring(L, va("Invalid Lua type: %s", lua_typename(L, type)));
			lua_error(L);

			break;
		}
		}
	}

	// invoke the native on the script host
	if (!FX_SUCCEEDED(scriptHost->InvokeNative(context)))
	{
		lua_pushstring(L, va("Execution of native %016x in script host failed.", hash));
		lua_error(L);
	}

	// number of Lua results
	int numResults = 0;

	// if no other result was requested, or we need to return the result anyway, push the result
	if (numReturnValues == 0 || returnResultAnyway)
	{
		// increment the result count
		numResults++;

		// handle the type coercion
		switch (returnValueCoercion)
		{
			case LuaMetaFields::ResultAsString:
			{
				const char* str = *reinterpret_cast<const char**>(&context.arguments[0]);
				
				if (str)
				{
					lua_pushstring(L, str);
				}
				else
				{
					lua_pushnil(L);
				}

				break;
			}
			case LuaMetaFields::ResultAsFloat:
				lua_pushnumber(L, *reinterpret_cast<float*>(&context.arguments[0]));
				break;
			case LuaMetaFields::ResultAsVector:
			{
				scrVector vector = *reinterpret_cast<scrVector*>(&context.arguments[0]);
				lua_pushvector3(L, vector.x, vector.y, vector.z);

				break;
			}
			case LuaMetaFields::ResultAsObject:
			{
				scrObject object = *reinterpret_cast<scrObject*>(&context.arguments[0]);
				lua_pushlstring(L, object.data, object.length);

				break;
			}
			case LuaMetaFields::ResultAsInteger:
				lua_pushinteger(L, *reinterpret_cast<int32_t*>(&context.arguments[0]));
				break;
			case LuaMetaFields::ResultAsLong:
				lua_pushinteger(L, *reinterpret_cast<int64_t*>(&context.arguments[0]));
				break;
			default:
			{
				int32_t integer = *reinterpret_cast<int32_t*>(&context.arguments[0]);

				if ((integer & 0xFFFFFFFF) == 0)
				{
					lua_pushboolean(L, false);
				}
				else
				{
					lua_pushinteger(L, integer);
				}
			}
		}
	}

	// loop over the return value pointers
	{
		int i = 0;

		while (i < numReturnValues)
		{
			switch (rettypes[i])
			{
				case LuaMetaFields::PointerValueInt:
					lua_pushinteger(L, *reinterpret_cast<int32_t*>(&retvals[i]));
					i++;
					break;

				case LuaMetaFields::PointerValueFloat:
					lua_pushnumber(L, *reinterpret_cast<float*>(&retvals[i]));
					i++;
					break;

				case LuaMetaFields::PointerValueVector:
				{
					scrVector vector = *reinterpret_cast<scrVector*>(&retvals[i]);
					lua_pushvector3(L, vector.x, vector.y, vector.z);

					i += 3;
					break;
				}
			}

			numResults++;
		}
	}

	// and return with the 'desired' amount of results
	return numResults;
}

int Lua_LoadNative(lua_State* L)
{
	const char* fn = luaL_checkstring(L, 1);

	auto& runtime = LuaScriptRuntime::GetCurrent();
	
	int isCfxv2 = 0;
	runtime->GetScriptHost2()->GetNumResourceMetaData("is_cfxv2", &isCfxv2);

	// TODO/TEMPORARY: fxv2 oal is disabled by default for now
	if (isCfxv2)
	{
		runtime->GetScriptHost2()->GetNumResourceMetaData("use_fxv2_oal", &isCfxv2);
	}

	if (isCfxv2)
	{
		auto nativeImpl = Lua_GetNative(L, fn);

		if (nativeImpl)
		{
			lua_pushcfunction(L, nativeImpl);
			return 1;
		}
	}

	OMPtr<fxIStream> stream;

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

template<LuaMetaFields metaField>
int Lua_GetMetaField(lua_State* L)
{
	lua_pushlightuserdata(L, &g_metaFields[(int)metaField]);

	return 1;
}

template<LuaMetaFields MetaField>
int Lua_GetPointerField(lua_State* L)
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
			if (lua_isnil(L, 1) || lua_islightuserdata(L, 1) || lua_isuserdata(L, 1))
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

static const struct luaL_Reg g_citizenLib[] =
{
	{ "SetTickRoutine", Lua_SetTickRoutine },
	{ "SetEventRoutine", Lua_SetEventRoutine },
	{ "Trace", Lua_Trace },
	{ "InvokeNative", Lua_InvokeNative },
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

static int Lua_Print(lua_State* L)
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i = 1; i <= n; i++) {
		const char *s;
		size_t l;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tolstring(L, -1, &l);  /* get result */
		if (s == NULL)
			return luaL_error(L, "'tostring' must return a string to 'print'");
		if (i > 1) ScriptTrace("%s", std::string("\t", 1));
		ScriptTrace("%s", std::string(s, l));
		lua_pop(L, 1);  /* pop result */
	}
	ScriptTrace("\n");
	return 0;
}

result_t LuaScriptRuntime::Create(IScriptHost *scriptHost)
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

			if (FX_SUCCEEDED(m_manifestHost->IsManifestVersionBetween(std::get<ManifestVersion>(versionPair).guid, guid_t{0}, &isGreater)) && isGreater)
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
#else
				"natives_server.lua"
#endif
				;
		}
	}

	safe_openlibs(m_state);

	// register the 'Citizen' library
	lua_newtable(m_state);
	luaL_setfuncs(m_state, g_citizenLib, 0);
	lua_setglobal(m_state, "Citizen");

	// load the system scheduler script
	result_t hr;

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/json.lua")))
	{
		return hr;
	}

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/MessagePack.lua")))
	{
		return hr;
	}

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

	return FX_S_OK;
}

result_t LuaScriptRuntime::LoadNativesBuild(const std::string& nativesBuild)
{
	result_t hr = FX_S_OK;

	bool useLazyNatives = false;

#ifndef IS_FXSERVER
	useLazyNatives = true;

	int32_t numFields = 0;

	if (FX_SUCCEEDED(hr = m_resourceHost->GetNumResourceMetaData("disable_lazy_natives", &numFields)))
	{
		if (numFields > 0)
		{
			useLazyNatives = false;
		}
	}
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
				auto j = json::array();

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

static int lua_error_handler(lua_State* L)
{
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");

	lua_pop(L, -2);

	lua_pushvalue(L, 1);
	lua_pushinteger(L, 2);

	lua_call(L, 2, 1);

	// TODO: proper log channels for this purpose
	ScriptTrace("Lua error: %s\n", lua_tostring(L, -1));

	return 1;
}

result_t LuaScriptRuntime::RunFileInternal(char* scriptName, std::function<result_t(char*)> loadFunction)
{
	LuaPushEnvironment pushed(this);

	//lua_pushcfunction(m_state, lua_error_handler);

	lua_getglobal(m_state, "debug");
	lua_getfield(m_state, -1, "traceback");
	lua_replace(m_state, -2);

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

int32_t LuaScriptRuntime::HandlesFile(char* fileName)
{
	return strstr(fileName, ".lua") != 0;
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
	*memoryUsage = (lua_gc(m_state, LUA_GCCOUNT, 0) * 1024) + lua_gc(m_state, LUA_GCCOUNTB, 0);

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

using Lua_NativeMap = std::map<std::string, lua_CFunction, std::less<>>;

#ifdef IS_FXSERVER
struct LuaNativeWrapper
{
	inline LuaNativeWrapper(uint64_t)
	{
	}
};

struct LuaNativeContext : public fxNativeContext
{
	inline LuaNativeContext(void*, int numArguments)
	{
		numArguments = numArguments;
		numResults = 0;
	}

	inline void Invoke(lua_State* L, uint64_t hash)
	{
		nativeIdentifier = hash;

		if (FX_FAILED(g_lastScriptHost->InvokeNative(*this)))
		{
			lua_pushstring(L, "Native invocation failed.");
			lua_error(L);
		}
	}

	template<typename TVal>
	inline TVal GetResult()
	{
		return *(TVal*)(&arguments[0]);
	}

	template<typename TVal>
	inline void SetArgument(size_t offset, const TVal& val)
	{
		if constexpr (sizeof(TVal) < 4)
		{
			*reinterpret_cast<uintptr_t*>(&arguments[offset]) = 0;
		}

		*reinterpret_cast<TVal*>(&arguments[offset]) = val;
	}

	template<typename TVal>
	inline void Push(const TVal& val)
	{
		if constexpr (sizeof(TVal) < 4)
		{
			*reinterpret_cast<uintptr_t*>(&arguments[numArguments]) = 0;
		}

		*reinterpret_cast<TVal*>(&arguments[numArguments]) = val;

		if constexpr (sizeof(TVal) == sizeof(scrVector))
		{
			numArguments += 3;
		}
		else
		{
			numArguments++;
		}
	}
};

#define LUA_EXC_WRAP_START(hash)

#define LUA_EXC_WRAP_END(hash)

#define ASSERT_LUA_ARGS(count) \
	if (!lua_asserttop(L, count)) return 0;
#else
struct LuaNativeWrapper
{
	rage::scrEngine::NativeHandler handler;

	inline LuaNativeWrapper(uint64_t hash)
	{
		handler = rage::scrEngine::GetNativeHandler(hash);
	}
};

struct LuaNativeContext
{
	NativeContextRaw rawCxt;

	int numArguments;
	uintptr_t arguments[32];

	LuaNativeWrapper* nw;

	__forceinline inline LuaNativeContext(LuaNativeWrapper* nw, int numArguments)
		: rawCxt(arguments, numArguments), numArguments(numArguments), nw(nw)
	{
		
	}

	__forceinline inline void Invoke(lua_State* L, uint64_t hash)
	{
		nw->handler(&rawCxt);
	}

	template<typename TVal>
	__forceinline inline TVal GetResult()
	{
		return *(TVal*)(&arguments[0]);
	}

	template<typename TVal>
	__forceinline inline void SetArgument(size_t offset, const TVal& val)
	{
		if constexpr (sizeof(TVal) < 4)
		{
			*reinterpret_cast<uintptr_t*>(&arguments[offset]) = 0;
		}

		*reinterpret_cast<TVal*>(&arguments[offset]) = val;
	}

	template<typename TVal>
	inline void Push(const TVal& val)
	{
		if constexpr (sizeof(TVal) < 4)
		{
			*reinterpret_cast<uintptr_t*>(&arguments[numArguments]) = 0;
		}

		*reinterpret_cast<TVal*>(&arguments[numArguments]) = val;

		if constexpr (sizeof(TVal) == sizeof(scrVector))
		{
			numArguments += 3;
		}
		else
		{
			numArguments++;
		}
	}
};

#define LUA_EXC_WRAP_START(hash) \
	try \
	{

#define LUA_EXC_WRAP_END(hash) \
	} \
	catch (std::exception& e) \
	{ \
		lua_pushstring(L, e.what()); \
		lua_error(L); \
	} \
	catch (...) \
	{ \
		lua_pushstring(L, va("Error executing native %016llx.", hash)); \
		lua_error(L); \
	}

#define ASSERT_LUA_ARGS(count) \
	if (!lua_asserttop(L, count)) return 0;
#endif

inline const char* Lua_ToFuncRef(lua_State* L, int idx)
{
	// TODO: maybe?
	return lua_tostring(L, idx);
}

inline uint32_t Lua_ToHash(lua_State* L, int idx)
{
	const auto value = lua_getvalue(L, idx);

	if (lua_valuetype(L, value) == LUA_TSTRING)
	{
		return HashString(lua_valuetostring(L, value));
	}
	
	return lua_valuetointeger(L, value);
}

struct scrVectorLua
{
	float x;
	uint32_t pad;
	float y;
	uint32_t pad2;
	float z;
	uint32_t pad3;

	inline scrVectorLua()
	{

	}

	inline scrVectorLua(float x, float y, float z)
		: x(x), y(y), z(z)
	{

	}
};

inline scrVectorLua Lua_ToScrVector(lua_State* L, int idx)
{
	auto f4 = lua_valuetofloat4(L, lua_getvalue(L, idx));

	return scrVectorLua{ f4.x, f4.y, f4.z };
}

inline void Lua_PushScrVector(lua_State* L, const scrVectorLua& val)
{
	lua_pushvector3(L, val.x, val.y, val.z);
}

inline void Lua_PushScrObject(lua_State* L, const scrObject& val)
{
	lua_pushlstring(L, val.data, val.length);

	lua_getglobal(L, "msgpack");
	lua_pushstring(L, "unpack");
	lua_gettable(L, -2);

	lua_pop(L, -2);

	lua_call(L, 1, 1);
}

#ifndef IS_FXSERVER
#include "Natives.h"
#else
#include "NativesServer.h"
#endif

lua_CFunction Lua_GetNative(lua_State* L, const char* name)
{
	auto it = natives.find(name);

	return (it != natives.end()) ? it->second : nullptr;
}

// {A7242855-0350-4CB5-A0FE-61021E7EAFAA}
FX_DEFINE_GUID(CLSID_LuaScriptRuntime,
			0xa7242855, 0x350, 0x4cb5, 0xa0, 0xfe, 0x61, 0x2, 0x1e, 0x7e, 0xaf, 0xaa);

FX_NEW_FACTORY(LuaScriptRuntime);

FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptFileHandlingRuntime);
}
