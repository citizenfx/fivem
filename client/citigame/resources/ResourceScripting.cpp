#include "StdInc.h"
#include <mmsystem.h>
#include "ResourceManager.h"
#include "ResourceScripting.h"
#include "CrossLibraryInterfaces.h"

CRITICAL_SECTION g_scriptCritSec;
ScriptEnvironment* g_currentEnvironment;
std::stack<ScriptEnvironment*> g_environmentStack;
bool g_errorOccurredThisFrame;
static ScriptThread* g_currentThread;

class PushEnvironment
{
private:
	ScriptEnvironment* m_oldEnvironment;

public:
	PushEnvironment(ScriptEnvironment* environment)
	{
		EnterCriticalSection(&g_scriptCritSec);

		m_oldEnvironment = g_currentEnvironment;
		g_currentEnvironment = environment;

		g_environmentStack.push(m_oldEnvironment);
	}

	~PushEnvironment()
	{
		g_currentEnvironment = m_oldEnvironment;

		g_environmentStack.pop();

		LeaveCriticalSection(&g_scriptCritSec);
	}
};

int lua_error_handler (lua_State *l);

ScriptThread::ScriptThread(ScriptEnvironment* environment, lua_State* luaThread, int luaRef, int numInitialArguments)
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

	g_currentEnvironment->AddEventHandler(std::string(eventName), luaRef);

	return 0;
}

LUA_FUNCTION(CreateThread)
{
	if (!g_currentEnvironment)
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
	auto thread = std::make_shared<ScriptThread>(g_currentEnvironment, luaThread, luaRef, nargs - 1);

	g_currentEnvironment->AddThread(thread);

	return 0;
}

LUA_FUNCTION(Wait)
{
	// set wake timer
	int msec = luaL_checkinteger(L, 1);

	g_currentThread->SetWait(msec);

	// yield the lua state
	return lua_yield(L, 0);
}

void ScriptEnvironment::AddEventHandler(std::string& eventName, ScriptFunctionRef ref)
{
	m_eventHandlers[eventName].push_back(ref);
}

void ScriptEnvironment::Tick()
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

void ScriptEnvironment::AddThread(std::shared_ptr<ScriptThread> thread)
{
	m_threads.push_back(thread);
}

void ScriptEnvironment::TriggerEvent(std::string& eventName, std::string& argsSerialized, int source)
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

std::string ScriptEnvironment::CallExportInternal(ScriptFunctionRef ref, std::string& argsSerialized, int(*deserializeCB)(lua_State*, int*, std::string&))
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
	}

	// serialize return value
	luaS_serializeArgs(m_luaState, lua_gettop(m_luaState), 1);

	size_t len;
	const char* string = lua_tolstring(m_luaState, -1, &len);

	std::string retValue = std::string(string, len);

	lua_pop(m_luaState, 3);

	STACK_CHECK;

	return retValue;
}

std::string ScriptEnvironment::CallExport(ScriptFunctionRef ref, std::string& argsSerialized)
{
	return CallExportInternal(ref, argsSerialized, luaS_deserializeArgs);
}

ResourceRef ScriptEnvironment::GetRef(int luaRef)
{
	ResourceRef retval;
	retval.reference = luaRef;
	retval.resource = m_resource;

	return retval;
}

int ScriptEnvironment::DuplicateRef(int luaRef)
{
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, luaRef);
	return luaL_ref(m_luaState, LUA_REGISTRYINDEX);
}

void ScriptEnvironment::RemoveRef(int luaRef)
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

ScriptEnvironment::ScriptEnvironment(Resource* resource)
	: m_resource(resource)
{
	// create a lua state; don't do anything with it yet
	m_luaState = luaL_newstate();
	safe_openlibs(m_luaState);
	luaopen_cjson(m_luaState);

	// init critsec if needed
	if (!g_scriptCritSec.DebugInfo)
	{
		InitializeCriticalSection(&g_scriptCritSec);
	}

	InitializeCriticalSection(&m_taskCritSec);

	m_missionCleanup = std::make_shared<CMissionCleanup>();
}

ScriptEnvironment::~ScriptEnvironment()
{
	lua_close(m_luaState);
}

std::string ScriptEnvironment::GetName()
{
	return m_resource->GetName();
}

void ScriptEnvironment::EnqueueTask(std::function<void()> task)
{
	EnterCriticalSection(&m_taskCritSec);

	m_taskQueue.push_back(task);

	LeaveCriticalSection(&m_taskCritSec);
}

extern GtaThread* TheScriptManager;

void ScriptEnvironment::Destroy()
{
	// don't do anything yet
	TheScriptManager->RemoveCleanupFlag();

	m_missionCleanup->CleanUp(TheScriptManager);
}

bool ScriptEnvironment::LoadFile(std::string& scriptName, std::string& path)
{
	// open the file
	fiDevice* device = fiDevice::GetDevice(path.c_str(), true);

	int handle = device->open(path.c_str(), true);

	if (handle == -1)
	{
		trace("Could not find script %s in resource %s.\n", scriptName.c_str(), m_resource->GetName().c_str());

		return false;
	}

	// read file data
	int length = device->fileLength(handle);
	char* fileData = new char[length + 1];

	device->read(handle, fileData, length);
	device->close(handle);

	fileData[length] = '\0';

	// create a chunk name prefixed with @ (suppresses '[string "..."]' formatting)
	std::string chunkName("@");
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

bool ScriptEnvironment::DoFile(std::string& scriptName, std::string& path)
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

bool ScriptEnvironment::DoInitFile(bool preParse)
{
	PushEnvironment env(this);

	lua_pushcfunction(m_luaState, lua_error_handler);
	int eh = lua_gettop(m_luaState);

	// function to call
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_initHandler);

	// actual script
	std::string scriptName = "__resource.lua";
	std::string scriptPath = m_resource->GetPath() + "/__resource.lua";

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

bool ScriptEnvironment::Create()
{
	// register game-specific Lua functions
	LuaFunction::RegisterAll(m_luaState);

	SignalResourceScriptInit.emit(m_luaState);

	// set environment
	PushEnvironment env(this);

	bool result;

	result = DoFile(std::string("natives.lua"), std::string("citizen:/natives.lua"));

	if (!result)
	{
		return result;
	}

	result = DoFile(std::string("resource_init.lua"), std::string("citizen:/resource_init.lua"));

	if (!result)
	{
		return result;
	}

	result = DoFile(std::string("natives_hl.lua"), std::string("citizen:/natives_hl.lua"));

	if (!result)
	{
		return result;
	}

	result = DoFile(std::string("natives_classes.lua"), std::string("citizen:/natives_classes.lua"));

	if (!result)
	{
		return result;
	}

	result = DoFile(std::string("luajit-msgpack-pure.lua"), std::string("citizen:/luajit-msgpack-pure.lua"));

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

bool ScriptEnvironment::LoadScripts()
{
	bool result;

	PushEnvironment env(this);

	// load scripts from the resource
	for (auto& scriptName : m_resource->GetScripts())
	{
		std::string path = m_resource->GetPath();
		path.append("/");
		path.append(scriptName);

		result = DoFile(scriptName, path);

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

sigslot::signal0<> ScriptEnvironment::SignalScriptReset;
sigslot::signal1<lua_State*> ScriptEnvironment::SignalResourceScriptInit;

int lua_error_handler(lua_State* L)
{
	luaL_traceback(L, L, lua_tostring(L, -1), 1);
	lua_remove(L, -2);

	trace("Lua error: %s\n", lua_tostring(L, -1));

	return 1;
}

static InitFunction initFunction([] ()
{
	g_hooksDLL->SetHookCallback(StringHash("mCleanup"), [] (void* missionCleanupInstance)
	{
		CMissionCleanup** instance = (CMissionCleanup**)missionCleanupInstance;

		if (g_currentEnvironment != nullptr)
		{
			*instance = g_currentEnvironment->GetMissionCleanup();
		}
	});
});