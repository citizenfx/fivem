/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fxScripting.h"

#include <lua.hpp>

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
		lua_close(m_state);
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

class LuaScriptRuntime : public OMClass<LuaScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime, IScriptRefRuntime>
{
private:
	typedef std::function<void(const char*, const char*, size_t, const char*)> TEventRoutine;

	typedef std::function<void(int32_t, const char*, size_t, char**, size_t*)> TCallRefRoutine;

	typedef std::function<int32_t(int32_t)> TDuplicateRefRoutine;

	typedef std::function<void(int32_t)> TDeleteRefRoutine;

private:
	LuaStateHolder m_state;

	IScriptHost* m_scriptHost;

	std::function<void()> m_tickRoutine;

	TEventRoutine m_eventRoutine;

	TCallRefRoutine m_callRefRoutine;

	TDuplicateRefRoutine m_duplicateRefRoutine;

	TDeleteRefRoutine m_deleteRefRoutine;

	void* m_parentObject;

	int m_instanceId;

public:
	inline LuaScriptRuntime()
	{
		m_instanceId = rand();
	}

	static OMPtr<LuaScriptRuntime> GetCurrent();

	void SetTickRoutine(const std::function<void()>& tickRoutine);

	void SetEventRoutine(const TEventRoutine& eventRoutine);

	inline void SetCallRefRoutine(const TCallRefRoutine& routine)
	{
		m_callRefRoutine = routine;
	}

	inline void SetDuplicateRefRoutine(const TDuplicateRefRoutine& routine)
	{
		m_duplicateRefRoutine = routine;
	}

	inline void SetDeleteRefRoutine(const TDeleteRefRoutine& routine)
	{
		m_deleteRefRoutine = routine;
	}

	inline IScriptHost* GetScriptHost()
	{
		return m_scriptHost;
	}

private:
	result_t LoadFileInternal(OMPtr<fxIStream> stream, char* scriptFile);

	result_t LoadHostFileInternal(char* scriptFile);

	result_t LoadSystemFileInternal(char* scriptFile);

	result_t RunFileInternal(char* scriptFile, std::function<result_t(char*)> loadFunction);

	result_t LoadSystemFile(char* scriptFile);

public:
	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;

	NS_DECL_ISCRIPTTICKRUNTIME;

	NS_DECL_ISCRIPTEVENTRUNTIME;

	NS_DECL_ISCRIPTREFRUNTIME;
};

static int lua_error_handler(lua_State* L);

OMPtr<LuaScriptRuntime> LuaScriptRuntime::GetCurrent()
{
	OMPtr<IScriptRuntime> runtime;
	LuaScriptRuntime* luaRuntime;
	
	assert(FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)));
	assert(luaRuntime = dynamic_cast<LuaScriptRuntime*>(runtime.GetRef()));

	return OMPtr<LuaScriptRuntime>(luaRuntime);
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
	auto luaRuntime = LuaScriptRuntime::GetCurrent();
	
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

			trace("Error running system tick function for resource %s: %s\n", "TODO", err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

void LuaScriptRuntime::SetTickRoutine(const std::function<void()>& tickRoutine)
{
	m_tickRoutine = tickRoutine;
}

static int Lua_SetEventRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent();

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

			trace("Error running system event handling function for resource %s: %s\n", "TODO", err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

void LuaScriptRuntime::SetEventRoutine(const TEventRoutine& eventRoutine)
{
	m_eventRoutine = eventRoutine;
}

static int Lua_SetCallRefRoutine(lua_State* L)
{
	// push the routine to reference and add a reference
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// set the event callback in the current routine
	auto luaRuntime = LuaScriptRuntime::GetCurrent();

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

			trace("Error running call reference function for resource %s: %s\n", "TODO", err.c_str());

			*retval = nullptr;
			*retvalLength = 0;
		}
		else
		{
			const char* retvalString = lua_tolstring(L, -1, retvalLength);
			strncpy(&retvalArray[0], retvalString, min(retvalArray.size(), *retvalLength));

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
	auto luaRuntime = LuaScriptRuntime::GetCurrent();

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

			trace("Error running system ref deletion function for resource %s: %s\n", "TODO", err.c_str());
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
	auto luaRuntime = LuaScriptRuntime::GetCurrent();

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

			trace("Error running system ref duplication function for resource %s: %s\n", "TODO", err.c_str());
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
	auto luaRuntime = LuaScriptRuntime::GetCurrent();
	
	char* refString;
	result_t hr = luaRuntime->GetScriptHost()->CanonicalizeRef(luaL_checkinteger(L, 1), luaRuntime->GetInstanceId(), &refString);

	lua_pushstring(L, refString);
	fwFree(refString);

	return 1;
}

static int Lua_InvokeFunctionReference(lua_State* L)
{
	// get required entries
	OMPtr<LuaScriptRuntime> luaRuntime = LuaScriptRuntime::GetCurrent();
	OMPtr<IScriptHost> scriptHost = luaRuntime->GetScriptHost();

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

int Lua_Trace(lua_State* L)
{
	trace("%s", luaL_checkstring(L, 1));

	return 0;
}

enum class LuaMetaFields
{
	PointerValueInt,
	PointerValueFloat,
	PointerValueVector,
	ReturnResultAnyway,
	ResultAsInteger,
	ResultAsFloat,
	ResultAsString,
	ResultAsVector,
	Max
};

static uint8_t g_metaFields[(int)LuaMetaFields::Max];

int Lua_InvokeNative(lua_State* L)
{
	// get required entries
	OMPtr<LuaScriptRuntime> luaRuntime = LuaScriptRuntime::GetCurrent();
	OMPtr<IScriptHost> scriptHost = luaRuntime->GetScriptHost();

	// variables to hold state
	fxNativeContext context = { 0 };

	// return values and their types
	int numReturnValues = 0;
	uintptr_t retvals[16] = { 0 };
	LuaMetaFields rettypes[16];

	// coercion for the result value
	LuaMetaFields returnValueCoercion = LuaMetaFields::Max;

	// flag to return a result even if a pointer return value is passed
	bool returnResultAnyway = false;

	// get argument count for the loop
	int numArgs = lua_gettop(L);

	// get the hash
	uint64_t hash = lua_tointeger(L, 1);

	context.nativeIdentifier = hash;

	// pushing function
	auto push = [&] (const auto& value)
	{
		*reinterpret_cast<uintptr_t*>(&context.arguments[context.numArguments]) = 0;
		*reinterpret_cast<std::decay_t<decltype(value)>*>(&context.arguments[context.numArguments]) = value;
		context.numArguments++;
	};

	// the big argument loop
	for (int i = 2; i <= numArgs; i++)
	{
		// get the type and decide what to do based on it
		int type = lua_type(L, i);

		// nil: add '0'
		if (type == LUA_TNIL)
		{
			push(0);
		}
		// number/integer
		else if (type == LUA_TNUMBER)
		{
			if (lua_isinteger(L, i))
			{
				push(lua_tointeger(L, i));
			}
			else
			{
				push(static_cast<float>(lua_tonumber(L, i)));
			}
		}
		// boolean
		else if (type == LUA_TBOOLEAN)
		{
			push(lua_toboolean(L, i));
		}
		// table (high-level class with __data field)
		else if (type == LUA_TTABLE)
		{
			lua_pushstring(L, "__data");
			lua_rawget(L, i);

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
		}
		// string
		else if (type == LUA_TSTRING)
		{
			push(lua_tostring(L, i));
		}
		// vector3
		else if (type == LUA_TVECTOR2 || type == LUA_TVECTOR3 || type == LUA_TVECTOR4 || type == LUA_TQUAT)
		{
			float x, y, z, w;

			if (type == LUA_TVECTOR2)
			{
				lua_checkvector2(L, i, &x, &y);
			}
			else if (type == LUA_TVECTOR3)
			{
				lua_checkvector3(L, i, &x, &y, &z);
			}
			else if (type == LUA_TVECTOR4)
			{
				lua_checkvector4(L, i, &x, &y, &z, &w);
			}
			else if (type == LUA_TQUAT)
			{
				lua_checkquat(L, i, &x, &y, &z, &w);
			}

			push(x);
			push(y);

			if (type == LUA_TVECTOR3 || type == LUA_TVECTOR4 || type == LUA_TQUAT)
			{
				push(z);

				if (type == LUA_TVECTOR4 || type == LUA_TQUAT)
				{
					push(w);
				}
			}
		}
		// metafield
		else if (type == LUA_TLIGHTUSERDATA)
		{
			uint8_t* ptr = reinterpret_cast<uint8_t*>(lua_touserdata(L, i));

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

						break;
					}
					case LuaMetaFields::ReturnResultAnyway:
						returnResultAnyway = true;
						break;
					case LuaMetaFields::ResultAsInteger:
					case LuaMetaFields::ResultAsString:
					case LuaMetaFields::ResultAsFloat:
					case LuaMetaFields::ResultAsVector:
						returnValueCoercion = metaField;
						break;
				}
			}
			else
			{
				push(ptr);
			}
		}
		else
		{
			lua_pushstring(L, va("Invalid Lua type: %s", lua_typename(L, type)));
			lua_error(L);
		}
	}

	// invoke the native on the script host
	if (!FX_SUCCEEDED(scriptHost->InvokeNative(context)))
	{
		lua_pushstring(L, va("Execution of native %016x in script host failed.", hash));
		lua_error(L);
	}

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
				lua_pushstring(L, *reinterpret_cast<const char**>(&context.arguments[0]));
				break;
			case LuaMetaFields::ResultAsFloat:
				lua_pushnumber(L, *reinterpret_cast<float*>(&context.arguments[0]));
				break;
			case LuaMetaFields::ResultAsVector:
			{
				scrVector vector = *reinterpret_cast<scrVector*>(&context.arguments[0]);
				lua_pushvector3(L, vector.x, vector.y, vector.z);

				break;
			}
			case LuaMetaFields::ResultAsInteger:
				lua_pushinteger(L, *reinterpret_cast<int32_t*>(&context.arguments[0]));
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
					lua_pushinteger(L, retvals[i]);
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

template<LuaMetaFields metaField>
int Lua_GetMetaField(lua_State* L)
{
	lua_pushlightuserdata(L, &g_metaFields[(int)metaField]);

	return 1;
}

static const struct luaL_Reg g_citizenLib[] =
{
	{ "SetTickRoutine", Lua_SetTickRoutine },
	{ "SetEventRoutine", Lua_SetEventRoutine },
	{ "Trace", Lua_Trace },
	{ "InvokeNative", Lua_InvokeNative },
	// ref things
	{ "SetCallRefRoutine", Lua_SetCallRefRoutine },
	{ "SetDeleteRefRoutine", Lua_SetDeleteRefRoutine },
	{ "SetDuplicateRefRoutine", Lua_SetDuplicateRefRoutine },
	{ "CanonicalizeRef", Lua_CanonicalizeRef },
	{ "InvokeFunctionReference", Lua_InvokeFunctionReference },
	// metafields
	{ "PointerValueInt", Lua_GetMetaField<LuaMetaFields::PointerValueInt> },
	{ "PointerValueFloat", Lua_GetMetaField<LuaMetaFields::PointerValueFloat> },
	{ "PointerValueVector", Lua_GetMetaField<LuaMetaFields::PointerValueVector> },
	{ "ReturnResultAnyway", Lua_GetMetaField<LuaMetaFields::ReturnResultAnyway> },
	{ "ResultAsInteger", Lua_GetMetaField<LuaMetaFields::ResultAsInteger> },
	{ "ResultAsFloat", Lua_GetMetaField<LuaMetaFields::ResultAsFloat> },
	{ "ResultAsString", Lua_GetMetaField<LuaMetaFields::ResultAsString> },
	{ "ResultAsVector", Lua_GetMetaField<LuaMetaFields::ResultAsVector> },
	{ nullptr, nullptr }
};

result_t LuaScriptRuntime::Create(IScriptHost *scriptHost)
{
	m_scriptHost = scriptHost;

	safe_openlibs(m_state);

	// register the 'Citizen' library
	lua_newtable(m_state);
	luaL_setfuncs(m_state, g_citizenLib, 0);
	lua_setglobal(m_state, "Citizen");

	// load the system scheduler script
	result_t hr;

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/natives.lua")))
	{
		return hr;
	}

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/MessagePack.lua")))
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

	return FX_S_OK;
}

result_t LuaScriptRuntime::Destroy()
{
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

		trace("Error parsing script %s in resource %s: %s\n", scriptFile, "TODO", err.c_str());

		// TODO: change?
		return FX_E_INVALIDARG;
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

	return LoadFileInternal(stream, scriptFile);
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
	trace("Lua error: %s\n", lua_tostring(L, -1));

	return 1;
}

result_t LuaScriptRuntime::RunFileInternal(char* scriptName, std::function<result_t(char*)> loadFunction)
{
	fx::PushEnvironment pushed(this);

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

		trace("Error loading script %s in resource %s: %s\n", scriptName, "TODO", err.c_str());

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
		fx::PushEnvironment pushed(this);

		m_tickRoutine();
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::TriggerEvent(char* eventName, char* eventPayload, uint32_t payloadSize, char* eventSource)
{
	if (m_eventRoutine)
	{
		fx::PushEnvironment pushed(this);

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
		*outRefIdx = m_duplicateRefRoutine(refIdx);
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::RemoveRef(int32_t refIdx)
{
	if (m_deleteRefRoutine)
	{
		m_deleteRefRoutine(refIdx);
	}

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

// {A7242855-0350-4CB5-A0FE-61021E7EAFAA}
FX_DEFINE_GUID(CLSID_LuaScriptRuntime,
			0xa7242855, 0x350, 0x4cb5, 0xa0, 0xfe, 0x61, 0x2, 0x1e, 0x7e, 0xaf, 0xaa);

FX_NEW_FACTORY(LuaScriptRuntime);

FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptFileHandlingRuntime);
}