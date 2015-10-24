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

class LuaScriptRuntime : public OMClass<LuaScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime>
{
private:
	LuaStateHolder m_state;

	IScriptHost* m_scriptHost;

	std::function<void()> m_tickRoutine;

public:
	static OMPtr<LuaScriptRuntime> GetCurrent();

	void SetTickRoutine(const std::function<void()>& tickRoutine);

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
	{ "Trace", Lua_Trace },
	{ "InvokeNative", Lua_InvokeNative },
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

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/scheduler.lua")))
	{
		return hr;
	}

	scriptHost->InvokeNative(fxNativeContext{});

	OMPtr<fxIStream> stream;
	
	if (FX_SUCCEEDED(scriptHost->OpenHostFile("__resource.lua", stream.GetAddressOf())))
	{
		char buffer[8192];
		uint32_t didRead;

		if (FX_SUCCEEDED(stream->Read(buffer, sizeof(buffer), &didRead)))
		{
			buffer[didRead] = '\0';

			trace("read:\n%s\n", buffer);
		}
	}

	return FX_S_OK;
}

result_t LuaScriptRuntime::Destroy()
{
	return FX_S_OK;
}

int32_t LuaScriptRuntime::GetInstanceId()
{
	// TODO: handle properly
	return 435;
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

	if (luaL_loadbuffer(m_state, &fileData[0], length, chunkName.c_str()) != 0)
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

// {A7242855-0350-4CB5-A0FE-61021E7EAFAA}
FX_DEFINE_GUID(CLSID_LuaScriptRuntime,
			0xa7242855, 0x350, 0x4cb5, 0xa0, 0xfe, 0x61, 0x2, 0x1e, 0x7e, 0xaf, 0xaa);

FX_NEW_FACTORY(LuaScriptRuntime);

FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptFileHandlingRuntime);
}