/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Resource.h"
#include "ResourceMetaDataComponent.h"

#include "VFSManager.h"

#include <lua.hpp>

class LuaMetaDataLoader : public fx::ResourceMetaDataLoader
{
private:
	lua_State* m_luaState;

	boost::optional<std::string> m_error;

	fx::ResourceMetaDataComponent* m_component;

private:
	bool LoadFile(const std::string& filename);

	bool DoFile(const std::string& filename, int results);

	int PushExceptionHandler();

public:
	inline fx::ResourceMetaDataComponent* GetComponent()
	{
		return m_component;
	}

	virtual boost::optional<std::string> LoadMetaData(fx::ResourceMetaDataComponent* component, const std::string& resourcePath) override;
};

bool LuaMetaDataLoader::LoadFile(const std::string& filename)
{
	// load the source file
	fwRefContainer<vfs::Stream> stream = vfs::OpenRead(filename);

	if (!stream.GetRef())
	{
		m_error = "Could not open resource metadata file " + filename + ".";

		return false;
	}

	auto bytes = stream->ReadToEnd();
	stream->Close();

	// turn the byte array into a string
	std::string code(bytes.begin(), bytes.end());

	// create the chunk name as well
	std::string chunkName = "@" + filename;

	// load the buffer
	if (luaL_loadbuffer(m_luaState, code.c_str(), code.length(), chunkName.c_str()) != 0)
	{
		// seemingly, it failed...
		m_error = "Could not parse resource metadata file " + filename + ": " + luaL_checkstring(m_luaState, -1);
		lua_pop(m_luaState, 1);

		return false;
	}

	return true;
}

int LuaMetaDataLoader::PushExceptionHandler()
{
	lua_getglobal(m_luaState, "debug");
	lua_getfield(m_luaState, -1, "traceback");
	lua_replace(m_luaState, -2);

	return lua_gettop(m_luaState);
}

bool LuaMetaDataLoader::DoFile(const std::string& filename, int results)
{
	// put an error handler on the stack
	int eh = PushExceptionHandler();

	bool result = false;

	// load the file
	if (LoadFile(filename))
	{
		// call the top function
		result = true;

		if (lua_pcall(m_luaState, 0, results, eh) != 0)
		{
			m_error = "Could not execute resource metadata file " + filename  + ": " + luaL_checkstring(m_luaState, -1);
			lua_remove(m_luaState, -1);

			result = false;
		}
	}

	lua_remove(m_luaState, eh);

	return result;
}

boost::optional<std::string> LuaMetaDataLoader::LoadMetaData(fx::ResourceMetaDataComponent* component, const std::string& resourcePath)
{
	// reset any errors
	m_error.reset();

	// set the metadata component
	m_component = component;

	// create the Lua state for the metadata loader
	m_luaState = luaL_newstate();

	// validate if the Lua state exists (with LuaJIT you apparently never know)
	assert(m_luaState);

	// openlibs as well
	luaL_openlibs(m_luaState);

	// register a metadata adder for ourselves
	lua_pushlightuserdata(m_luaState, this);

	lua_pushcclosure(m_luaState, [] (lua_State* L)
	{
		LuaMetaDataLoader* loader = reinterpret_cast<LuaMetaDataLoader*>(const_cast<void*>(lua_topointer(L, lua_upvalueindex(1))));

		loader->GetComponent()->AddMetaData(luaL_checkstring(L, 1), luaL_checkstring(L, 2));

		return 0;
	}, 1);

	lua_setglobal(m_luaState, "AddMetaData");

	// push the exception handler
	int eh = PushExceptionHandler();

	// run global initialization code
	bool result = true;
	result = result && DoFile("citizen:/scripting/lua/json.lua", 0);

	result = result && DoFile("citizen:/scripting/resource_init.lua", 1);

	// remove unsafe handlers from the Lua state
	const char* unsafeGlobals[] = { "ffi", "require", "dofile", "load", "loadfile", "package", /*"AddMetaData", */"os", "io" };

	for (auto removeThat : unsafeGlobals)
	{
		lua_pushnil(m_luaState);
		lua_setglobal(m_luaState, removeThat);
	}

	// run the user file
	result = result && LoadFile(resourcePath + "/__resource.lua");

	if (result)
	{
		// invoke the init function with the resource chunk as argument
		if (lua_pcall(m_luaState, 1, 0, eh) != 0)
		{
			m_error = "Could not execute resource metadata file " + resourcePath + "/__resource.lua: " + luaL_checkstring(m_luaState, -1);
			lua_remove(m_luaState, -1);

			result = false;
		}
	}

	// remove the exception handler
	lua_remove(m_luaState, eh);

	if (!result && !m_error)
	{
		m_error = "Unknown initialization error.";
	}

	lua_close(m_luaState);
	m_luaState = nullptr;

	return m_error;
}

static LuaMetaDataLoader g_metaDataLoader;

static InitFunction initFunction([] ()
{
	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		fwRefContainer<fx::ResourceMetaDataComponent> metaDataComponent(new fx::ResourceMetaDataComponent(resource));
		metaDataComponent->SetMetaDataLoader(new LuaMetaDataLoader());

		resource->SetComponent(metaDataComponent);
	});
});