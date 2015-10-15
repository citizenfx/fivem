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

class LuaScriptRuntime : public OMClass<LuaScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime>
{
private:
	LuaStateHolder m_state;

	IScriptHost* m_scriptHost;

private:
	result_t LoadFileInternal(char* scriptFile);

public:
	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;
};

// luaL_openlibs version without io/os libs
static const luaL_Reg lualibs[] =
{
	{ "", luaopen_base },
	{ LUA_LOADLIBNAME, luaopen_package },
	{ LUA_TABLIBNAME, luaopen_table },
	{ LUA_STRLIBNAME, luaopen_string },
	{ LUA_MATHLIBNAME, luaopen_math },
	{ LUA_DBLIBNAME, luaopen_debug },
	{ NULL, NULL }
};

LUALIB_API void safe_openlibs(lua_State *L)
{
	const luaL_Reg *lib = lualibs;
	for (; lib->func; lib++)
	{
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}
}

result_t LuaScriptRuntime::Create(IScriptHost *scriptHost)
{
	safe_openlibs(m_state);

	scriptHost->InvokeNative(fxNativeContext{});

	OMPtr<fxIStream> stream;
	
	if (SUCCEEDED(scriptHost->OpenHostFile("__resource.lua", stream.GetAddressOf())))
	{
		char buffer[8192];
		uint32_t didRead;

		if (SUCCEEDED(stream->Read(buffer, sizeof(buffer), &didRead)))
		{
			buffer[didRead] = '\0';

			trace("read:\n%s\n", buffer);
		}
	}

	m_scriptHost = scriptHost;

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

result_t LuaScriptRuntime::LoadFileInternal(char* scriptFile)
{
	// open the file
	OMPtr<fxIStream> stream;

	result_t hr = m_scriptHost->OpenHostFile(scriptFile, stream.GetAddressOf());

	if (FX_FAILED(hr))
	{
		// TODO: log this?
		return hr;
	}

	// read file data
	uint64_t length;

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

result_t LuaScriptRuntime::LoadFile(char* scriptName)
{
	lua_pushcfunction(m_state, lua_error_handler);
	int eh = lua_gettop(m_state);

	if (!LoadFileInternal(scriptName))
	{
		return FX_E_INVALIDARG;
	}

	if (lua_pcall(m_state, 0, 0, eh) != 0)
	{
		std::string err = luaL_checkstring(m_state, -1);
		lua_pop(m_state, 1);

		trace("Error loading script %s in resource %s: %s\n", scriptName, "TODO", err.c_str());

		return false;
	}

	lua_pop(m_state, 1);

	return true;
}

int32_t LuaScriptRuntime::HandlesFile(char* fileName)
{
	return strstr(fileName, ".lua") != 0;
}

// {A7242855-0350-4CB5-A0FE-61021E7EAFAA}
FX_DEFINE_GUID(CLSID_LuaScriptRuntime,
			0xa7242855, 0x350, 0x4cb5, 0xa0, 0xfe, 0x61, 0x2, 0x1e, 0x7e, 0xaf, 0xaa);

FX_NEW_FACTORY(LuaScriptRuntime);

FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptFileHandlingRuntime);
}