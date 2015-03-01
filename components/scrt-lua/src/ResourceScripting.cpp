/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <mmsystem.h>
#include "ResourceManager.h"
#include "ResourceScripting.h"
//#include "CrossLibraryInterfaces.h"

bool g_errorOccurredThisFrame;
static ScriptThread* g_currentThread;

int lua_error_handler (lua_State *l);

ScriptThread::ScriptThread(LuaScriptEnvironment* environment, lua_State* luaThread, int luaRef, int numInitialArguments)
	: m_environment(environment), m_luaThread(luaThread), m_lastWake(0), m_sleepTime(0), m_luaRef(luaRef), m_nInitialArguments(numInitialArguments)
{
	
}

bool ScriptThread::Tick()
{
	// check if we should execute
	if (m_sleepTime > 0)
	{
		if ((timeGetTime() - m_lastWake) < m_sleepTime)
		{
			return true;
		}
	}

	g_currentThread = this;

	// restart the lua thread
	int result = lua_resume(m_luaThread, m_nInitialArguments);
	m_nInitialArguments = 0;

	if (result != LUA_YIELD)
	{
		if (result != 0)
		{
			lua_error_handler(m_luaThread);

			std::string err = std::string(luaL_checkstring(m_luaThread, -1));
			lua_pop(m_luaThread, 1);

			GlobalError("Error running of script in resource %s: %s\nsee console for details", m_environment->GetName().c_str(), err.c_str());

			g_errorOccurredThisFrame = true;
		}

		// free the lua state
		luaL_unref(m_environment->GetLua(), LUA_REGISTRYINDEX, m_luaRef);

		g_currentThread = nullptr;

		return false;
	}

	m_lastWake = timeGetTime();
	g_currentThread = nullptr;

	return true;
}

void ScriptThread::SetWait(int msec)
{
	m_sleepTime = msec;
}

LUA_FUNCTION(AddEventHandler)
{
	const char* eventName = luaL_checkstring(L, 1);

	lua_pushvalue(L, 2);
	int luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

	GetCurrentLuaEnvironment()->AddEventHandler(fwString(eventName), luaRef);

	return 0;
}

LUA_FUNCTION(CreateThread)
{
	if (!GetCurrentLuaEnvironment())
	{
		lua_pushstring(L, "no environment when creating thread");
		lua_error(L);
	}

	lua_State* luaThread = lua_newthread(L);
	lua_pushvalue(L, 1);

	int nargs = lua_gettop(L);

	for (int i = 2; i <= nargs; i++)
	{
		lua_pushvalue(L, i);
	}

	lua_xmove(L, luaThread, nargs);

	// reference the coroutine
	int luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

	// create the thread object and add it
	auto thread = std::make_shared<ScriptThread>(GetCurrentLuaEnvironment(), luaThread, luaRef, nargs - 1);

	GetCurrentLuaEnvironment()->AddThread(thread);

	return 0;
}

LUA_FUNCTION(Wait)
{
	// set wake timer
	int msec = luaL_checkinteger(L, 1);

	// if not on a thread
	if (!g_currentThread)
	{
		lua_pushstring(L, "attempted to Wait outside of a thread context");
		lua_error(L);
	}

	g_currentThread->SetWait(msec);

	// yield the lua state
	return lua_yield(L, 0);
}

void LuaScriptEnvironment::AddEventHandler(fwString& eventName, ScriptFunctionRef ref)
{
	m_eventHandlers[eventName].push_back(ref);
}

void LuaScriptEnvironment::Tick()
{
	// store the environment someplace
	//g_currentEnvironment = this;
	PushEnvironment env(this);

	EnterCriticalSection(&m_taskCritSec);

	// invoke any tasks that are waiting
	for (auto& task : m_taskQueue)
	{
		task();
	}

	m_taskQueue.clear();

	LeaveCriticalSection(&m_taskCritSec);

	// loop through all threads and see which we can remove
	for (int i = m_threads.size(); i > 0; i--)
	{
		auto thread = m_threads[i - 1];

		if (!thread->Tick())
		{
			m_threads.erase(m_threads.begin() + (i - 1));
		}
	}
}

void LuaScriptEnvironment::AddThread(std::shared_ptr<ScriptThread> thread)
{
	m_threads.push_back(thread);
}

void LuaScriptEnvironment::TriggerEvent(fwString& eventName, fwString& argsSerialized, int source)
{
	auto eventHandlers = m_eventHandlers.find(eventName);

	if (eventHandlers == m_eventHandlers.end())
	{
		return;
	}

	lua_State* L = m_luaState;
	STACK_BASE;

	PushEnvironment env(this);

	lua_pushnumber(m_luaState, source);
	lua_setglobal(m_luaState, "source");

	// stack: 0
	lua_pushcfunction(m_luaState, lua_error_handler);
	int eh = lua_gettop(m_luaState);

	int length;
	int table = luaS_deserializeArgs(m_luaState, &length, argsSerialized);

	int k = 0;

	for (auto& eventHandler : eventHandlers->second)
	{
		// now replace the original table with the callback
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, eventHandler);
		lua_replace(m_luaState, table);

		// push stuff we love
		lua_pushvalue(m_luaState, table);

		for (int i = 1; i <= length; i++)
		{
			lua_pushvalue(m_luaState, table + i);
		}

		// TODO: support return value
		if (lua_pcall(m_luaState, length, 0, eh) != 0)
		{
			std::string err = luaL_checkstring(m_luaState, -1);
			lua_pop(m_luaState, 1);

			g_errorOccurredThisFrame = true;

			GlobalError("Error during event handler for %s: %s\nsee console for details", eventName.c_str(), err.c_str());

			m_eventHandlers.clear();

			break;
		}

		k++;
	}

	lua_pop(m_luaState, length + 1);
	lua_pop(m_luaState, 1);

	STACK_CHECK;
}

fwString LuaScriptEnvironment::CallExportInternal(ScriptFunctionRef ref, fwString& argsSerialized, int(*deserializeCB)(lua_State*, int*, fwString&))
{
	PushEnvironment env(this);

	lua_State* L = m_luaState;
	STACK_BASE;

	// stack: 0
	lua_pushcfunction(m_luaState, lua_error_handler);
	int eh = lua_gettop(m_luaState);

	int length;
	int table = deserializeCB(m_luaState, &length, argsSerialized);

	// now replace the original table with the callback
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, ref);
	lua_replace(m_luaState, table);

	// call the method
	if (lua_pcall(m_luaState, length, 1, eh) != 0)
	{
		std::string err = luaL_checkstring(m_luaState, -1);
		lua_pop(m_luaState, 1);

		GlobalError("Error during export call handler: %s\nsee console for details", err.c_str());

		g_errorOccurredThisFrame = true;

		return "";
	}

	// serialize return value
	luaS_serializeArgs(m_luaState, lua_gettop(m_luaState), 1);

	size_t len;
	const char* string = lua_tolstring(m_luaState, -1, &len);

	fwString retValue = fwString(string, len);

	lua_pop(m_luaState, 3);

	STACK_CHECK;

	return retValue;
}

fwString LuaScriptEnvironment::CallExport(ScriptFunctionRef ref, fwString& argsSerialized)
{
	return CallExportInternal(ref, argsSerialized, luaS_deserializeArgs);
}

ResourceRef LuaScriptEnvironment::GetRef(int luaRef)
{
	ResourceRef retval;
	retval.reference = luaRef;
	retval.resource = m_resource;
	retval.instance = m_instanceId;

	return retval;
}

int LuaScriptEnvironment::DuplicateRef(int luaRef)
{
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, luaRef);
	return luaL_ref(m_luaState, LUA_REGISTRYINDEX);
}

void LuaScriptEnvironment::RemoveRef(int luaRef)
{
	luaL_unref(m_luaState, LUA_REGISTRYINDEX, luaRef);
}

// luaL_openlibs version without io/os libs
static const luaL_Reg lualibs[] =
{
	{"", luaopen_base},
	{LUA_LOADLIBNAME, luaopen_package},
	{LUA_TABLIBNAME, luaopen_table},
	{LUA_STRLIBNAME, luaopen_string},
	{LUA_MATHLIBNAME, luaopen_math},
	{LUA_DBLIBNAME, luaopen_debug},
	{LUA_FFILIBNAME, luaopen_ffi},
	{LUA_BITLIBNAME, luaopen_bit},
	{NULL, NULL}
};

LUALIB_API void safe_openlibs (lua_State *L)
{
	const luaL_Reg *lib = lualibs;
	for (; lib->func; lib++)
	{
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}
}

extern "C" int luaopen_cjson(lua_State *l);

LuaScriptEnvironment::LuaScriptEnvironment(Resource* resource)
	: m_resource(resource)
{
	// create a lua state; don't do anything with it yet
	m_luaState = luaL_newstate();
	safe_openlibs(m_luaState);
	luaopen_cjson(m_luaState);

	// get an instance ID
	m_instanceId = rand();

	// init critsec if needed
	InitializeCriticalSection(&m_taskCritSec);

	m_missionCleanup = std::make_shared<CMissionCleanup>();
}

LuaScriptEnvironment::~LuaScriptEnvironment()
{
	lua_close(m_luaState);
}

fwString LuaScriptEnvironment::GetName()
{
	return m_resource->GetName();
}

void LuaScriptEnvironment::EnqueueTask(std::function<void()> task)
{
	EnterCriticalSection(&m_taskCritSec);

	m_taskQueue.push_back(task);

	LeaveCriticalSection(&m_taskCritSec);
}

//extern GtaThread* TheScriptManager;

void LuaScriptEnvironment::Destroy()
{
	TheScriptManager->RemoveCleanupFlag();

	m_missionCleanup->CleanUp(TheScriptManager);
}

bool LuaScriptEnvironment::LoadFile(fwString& scriptName, fwString& path)
{
	// open the file
	fiDevice* device = fiDevice::GetDevice(path.c_str(), true);

	int handle = device->Open(path.c_str(), true);

	if (handle == -1)
	{
		trace("Could not find script %s in resource %s.\n", scriptName.c_str(), m_resource->GetName().c_str());

		return false;
	}

	// read file data
	int length = device->GetFileLength(handle);
	char* fileData = new char[length + 1];

	device->Read(handle, fileData, length);
	device->Close(handle);

	fileData[length] = '\0';

	// create a chunk name prefixed with @ (suppresses '[string "..."]' formatting)
	fwString chunkName("@");
	chunkName.append(path);

	if (luaL_loadbuffer(m_luaState, fileData, length, chunkName.c_str()) != 0)
	{
		std::string err = luaL_checkstring(m_luaState, -1);
		lua_pop(m_luaState, 1);

		trace("Error parsing script %s in resource %s: %s\n", scriptName.c_str(), m_resource->GetName().c_str(), err.c_str());

		delete[] fileData;

		return false;
	}

	// delete the buffer and run the script's global scope
	delete[] fileData;

	return true;

}

bool LuaScriptEnvironment::DoFile(fwString& scriptName, fwString& path)
{
	lua_pushcfunction(m_luaState, lua_error_handler);
	int eh = lua_gettop(m_luaState);

	if (!LoadFile(scriptName, path))
	{
		return false;
	}

	if (lua_pcall(m_luaState, 0, 0, eh) != 0)
	{
		std::string err = luaL_checkstring(m_luaState, -1);
		lua_pop(m_luaState, 1);

		trace("Error loading script %s in resource %s: %s\n", scriptName.c_str(), m_resource->GetName().c_str(), err.c_str());

		return false;
	}

	lua_pop(m_luaState, 1);

	return true;
}

bool LuaScriptEnvironment::DoInitFile(bool preParse)
{
	PushEnvironment env(this);

	lua_pushcfunction(m_luaState, lua_error_handler);
	int eh = lua_gettop(m_luaState);

	// function to call
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_initHandler);

	// actual script
	fwString scriptName = "__resource.lua";
	fwString scriptPath = va("%s/__resource.lua", m_resource->GetPath().c_str());

	if (!LoadFile(scriptName, scriptPath))
	{
		return false;
	}

	// boolean argument
	lua_pushboolean(m_luaState, preParse);

	// call the init handler
	if (lua_pcall(m_luaState, 2, 0, eh) != 0)
	{
		std::string err = luaL_checkstring(m_luaState, -1);
		lua_pop(m_luaState, 1);

		trace("Error loading init script in resource %s: %s\n", m_resource->GetName().c_str(), err.c_str());

		return false;
	}

	lua_pop(m_luaState, 1);

	return true;
}

bool LuaScriptEnvironment::Create()
{
	// register game-specific Lua functions
	LuaFunction::RegisterAll(m_luaState);

	SignalResourceScriptInit(m_luaState);

	// set environment
	PushEnvironment env(this);

	bool result;

	result = DoFile(fwString("natives.lua"), fwString("citizen:/natives.lua"));

	if (!result)
	{
		return result;
	}

	result = DoFile(fwString("resource_init.lua"), fwString("citizen:/resource_init.lua"));

	if (!result)
	{
		return result;
	}

	result = DoFile(fwString("natives_hl.lua"), fwString("citizen:/natives_hl.lua"));

	if (!result)
	{
		return result;
	}

	result = DoFile(fwString("natives_classes.lua"), fwString("citizen:/natives_classes.lua"));

	if (!result)
	{
		return result;
	}

	result = DoFile(fwString("luajit-msgpack-pure.lua"), fwString("citizen:/luajit-msgpack-pure.lua"));

	if (!result)
	{
		return result;
	}

	// disable unsafe functions to new callers
	const char* unsafeGlobals[] = { "ffi", "require", "dofile", "load", "loadfile", "package", "RegisterInitHandler" };

	for (auto removeThat : unsafeGlobals)
	{
		lua_pushnil(m_luaState);
		lua_setglobal(m_luaState, removeThat);
	}

	return true;
}

bool LuaScriptEnvironment::LoadScripts()
{
	bool result;

	PushEnvironment env(this);

	// load scripts from the resource
	for (auto& scriptName : m_resource->GetScripts())
	{
		fwString path = va("%s/%s", m_resource->GetPath().c_str(), scriptName.c_str());

		result = DoFile(fwString(scriptName.c_str()), path);

		if (!result)
		{
			return result;
		}
	}

	// define exported functions
	for (auto& exportDef : m_resource->GetExports())
	{
		// get the global
		lua_getglobal(m_luaState, exportDef.GetName().c_str());

		// set the reference
		int reference = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

		exportDef.SetScriptFunction(reference);
	}

	return true;
}

LuaFunction* luaFunctionList;

void LuaFunction::RegisterAll(lua_State* L)
{
	LuaFunction* func = luaFunctionList;

	do 
	{
		lua_register(L, func->GetName(), func->GetFunction());
	} while (func = func->GetNext());
}

fwEvent<> LuaScriptEnvironment::SignalScriptReset;
fwEvent<lua_State*> LuaScriptEnvironment::SignalResourceScriptInit;

int lua_error_handler(lua_State* L)
{
	luaL_traceback(L, L, lua_tostring(L, -1), 1);
	lua_remove(L, -2);

	trace("Lua error: %s\n", lua_tostring(L, -1));

	return 1;
}

uint32_t LuaScriptEnvironment::GetInstanceId()
{
	return m_instanceId;
}

const char* LuaScriptEnvironment::GetEnvironmentName()
{
	return "Lua";
}

CMissionCleanup* LuaScriptEnvironment::GetMissionCleanup()
{
	return m_missionCleanup.get();
}

static InitFunction initFunction([] ()
{
	Resource::OnCreateScriptEnvironments.Connect([] (fwRefContainer<Resource> resource)
	{
		resource->AddScriptEnvironment(new LuaScriptEnvironment(resource.GetRef()));
	});

	CMissionCleanup::OnQueryMissionCleanup.Connect([] (CMissionCleanup*& handler)
	{
		auto environment = GetCurrentLuaEnvironment();

		if (environment)
		{
			handler = environment->GetMissionCleanup();
		}
	});
	/*g_hooksDLL->SetHookCallback(StringHash("mCleanup"), [] (void* missionCleanupInstance)
	{
		CMissionCleanup** instance = (CMissionCleanup**)missionCleanupInstance;

		if (g_currentEnvironment != nullptr)
		{
			*instance = g_currentEnvironment->GetMissionCleanup();
		}
	});

	g_hooksDLL->SetHookCallback(StringHash("mCleanupT"), [] (void*)
	{
		TheResources.ForAllResources([] (std::shared_ptr<Resource> resource)
		{
			if (resource->GetState() != ResourceStateRunning)
			{
				return;
			}

			resource->GetScriptEnvironment()->GetMissionCleanup()->CheckIfCollisionHasLoadedForMissionObjects();
		});
	});*/
});