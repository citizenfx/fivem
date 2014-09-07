#pragma once

#include "BaseResourceScripting.h"
#include <stack>
#include <functional>
#include <memory>
#include <map>
#include <lua.hpp>
#include <MissionCleanup.h>

class Resource;
class LuaScriptEnvironment;

struct ResourceRef
{
	int reference;
	Resource* resource;
	uint32_t instance;
};

class ScriptThread
{
private:
	LuaScriptEnvironment* m_environment;

	uint32_t m_lastWake;
	uint32_t m_sleepTime;

	lua_State* m_luaThread;

	int m_luaRef;

	int m_nInitialArguments;

public:
	ScriptThread(LuaScriptEnvironment* environment, lua_State* luaThread, int luaRef, int initialArguments = 0);

	bool Run();

	bool Tick();

	void SetWait(int msec);
};

class LuaScriptEnvironment : public BaseScriptEnvironment
{
private:
	Resource* m_resource;

	lua_State* m_luaState;

	std::vector<std::shared_ptr<ScriptThread>> m_threads;

	std::map<fwString, std::vector<ScriptFunctionRef>> m_eventHandlers;

	std::vector<std::function<void()>> m_taskQueue;

	CRITICAL_SECTION m_taskCritSec;

	std::shared_ptr<CMissionCleanup> m_missionCleanup;

	int m_initHandler;

	uint32_t m_instanceId;

private:
	fwString CallExportInternal(ScriptFunctionRef ref, fwString& argsSerialized, int(*deserializeCB)(lua_State*, int*, fwString&));

public:
	LuaScriptEnvironment(Resource* resource);

	virtual ~LuaScriptEnvironment();

	virtual bool Create() override;

	virtual bool DoInitFile(bool preParse) override;

	bool DoFile(fwString& scriptName, fwString& path);

	bool LoadFile(fwString& scriptName, fwString& path);

	bool LoadScripts();

	virtual void Destroy() override;

	virtual void Tick() override;

	virtual fwString CallExport(ScriptFunctionRef ref, fwString& argsSerialized) override;

	virtual void TriggerEvent(fwString& eventName, fwString& argsSerialized, int source) override;

	ScriptFunctionRef GetExport(fwString& name);

	void AddThread(std::shared_ptr<ScriptThread> thread);

	void AddEventHandler(fwString& eventName, ScriptFunctionRef ref);
	
	void EnqueueTask(std::function<void()> task);

	ResourceRef GetRef(int luaRef);

	virtual void RemoveRef(int luaRef) override;

	virtual int DuplicateRef(int luaRef) override;

	virtual const char* GetEnvironmentName() override;

	static fwEvent<lua_State*> SignalResourceScriptInit;
	static fwEvent<> SignalScriptReset;

public:
	inline void SetInitHandler(int luaRef) { m_initHandler = luaRef; }

	inline lua_State* GetLua() { return m_luaState; }

	inline Resource* GetResource() { return m_resource; }

	inline CMissionCleanup* GetMissionCleanup() { return m_missionCleanup.get(); }

	virtual uint32_t GetInstanceId();

	fwString GetName();
};

// lua function management
class LuaFunction;

extern LuaFunction* luaFunctionList;

class LuaFunction
{
private:
	const char* m_name;

	lua_CFunction m_function;

	LuaFunction* m_next;

public:
	LuaFunction(const char* name, lua_CFunction function)
		: m_name(name), m_function(function)
	{
		m_next = luaFunctionList;

		luaFunctionList = this;
	}

	inline const char* GetName() { return m_name; }
	inline lua_CFunction GetFunction() { return m_function; }
	inline LuaFunction* GetNext() { return m_next; }

	static void RegisterAll(lua_State* L);
};

int lua_error_handler(lua_State *l);

#undef NDEBUG
#include <assert.h>
#define NDEBUG

#define LUA_FUNCTION(name) int luaFunc_##name(lua_State* L); LuaFunction _luaFunc##name(#name, luaFunc_##name); int luaFunc_##name(lua_State* L)

#ifdef NO_LUA_STACK_CHECKS
#define STACK_BASE_(name)
#define STACK_CHECK_N_(name,N)
#else
#define STACK_BASE_(name) int name = lua_gettop(L)
#define STACK_CHECK_N_(name,N) assert(lua_gettop(L)==name+N)
#endif

#define STACK_BASE STACK_BASE_(____stack)
#define STACK_CHECK_N(N) STACK_CHECK_N_(____stack,N)
#define STACK_CHECK_(name) STACK_CHECK_N_(name,0)
#define STACK_CHECK STACK_CHECK_N(0)

void luaS_serializeArgs(lua_State* L, int firstArg, int numArgs);
void luaS_serializeArgsJSON(lua_State* L, int firstArg, int numArgs);
int luaS_deserializeArgs(lua_State* L, int* numArgs, fwString& argsSerialized);
int luaS_deserializeArgsJSON(lua_State* L, int* numArgs, fwString& argsSerialized);

extern bool g_errorOccurredThisFrame;

inline BaseScriptEnvironment* GetInvokingEnvironment()
{
	auto invokingEnvironment = BaseScriptEnvironment::GetInvokingEnvironment();

	return invokingEnvironment ? invokingEnvironment : BaseScriptEnvironment::GetCurrentEnvironment();
}

inline LuaScriptEnvironment* GetCurrentLuaEnvironment()
{
	return dynamic_cast<LuaScriptEnvironment*>(BaseScriptEnvironment::GetCurrentEnvironment());
}