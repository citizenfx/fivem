#include "StdInc.h"
#include "ResourceScripting.h"
#include "ResourceManager.h"

#include "CrossLibraryInterfaces.h"
//#include "CefWindow.h"

LUA_FUNCTION(SafeguardDisconnect)
{
	g_hooksDLL->SetDisconnectSafeguard(lua_toboolean(L, 1));

	return 0;
}

LUA_FUNCTION(echo)
{
	trace("%s", luaL_checkstring(L, 1));

	printf(luaL_checkstring(L, 1));

	return 0;
}

#if 0
struct ControlStringTaskData
{
	lua_State* L;
	int luaRef;
	ScriptEnvironment* environment;
	int stateNumber;
	char outData[65536];
};

int lua_error_handler (lua_State *l);

static void SendControlStringRet(NPAsync<NPSendRandomStringResult>* async)
{
	ControlStringTaskData* data = (ControlStringTaskData*)async->GetUserData();

	if (data->stateNumber == TheResources.GetStateNumber())
	{
		EnterCriticalSection(&g_scriptCritSec);

		g_currentEnvironment = data->environment;

		lua_pushcfunction(data->L, lua_error_handler);
		int eh = lua_gettop(data->L);

		lua_rawgeti(data->L, LUA_REGISTRYINDEX, data->luaRef);

		lua_pushstring(data->L, data->outData);

		if (lua_pcall(data->L, 1, 0, eh) != 0)
		{
			std::string err = luaL_checkstring(data->L, -1);
			lua_pop(data->L, 1);

			GlobalError("Error during async handler: %s\nsee console for details", err.c_str());
		}

		luaL_unref(data->L, LUA_REGISTRYINDEX, data->luaRef);

		g_currentEnvironment = nullptr;

		LeaveCriticalSection(&g_scriptCritSec);
	}

	delete data;
}

LUA_FUNCTION(SendControlString)
{
	const char* str = luaL_checkstring(L, 1);
	lua_pushvalue(L, 2);

	int cb = luaL_ref(L, LUA_REGISTRYINDEX);

	ControlStringTaskData* task = new ControlStringTaskData;
	task->L = L;
	task->luaRef = cb;
	task->environment = g_currentEnvironment;
	task->stateNumber = TheResources.GetStateNumber();

	auto async = NP_SendRandomStringExt(va("citiv %s", str), task->outData, sizeof(task->outData));
	async->SetCallback(SendControlStringRet, task);	

	return 0;
}

static void SendUIStringDo(NUIWindowType type, std::string typeStr, std::string argStr)
{
	EnterV8Context(type);
	CefV8ValueList arguments;
	arguments.push_back(CefV8Value::CreateString(argStr));

	InvokeNUICallback(type, typeStr.c_str(), arguments);

	LeaveV8Context(type);
}

LUA_FUNCTION(SendUIString)
{
	const char* type = luaL_checkstring(L, 2);
	std::string typeStr = type;
	std::string str = lua_tostring(L, 3);

	CefPostTask(TID_RENDERER, NewCefRunnableFunction(&SendUIStringDo, (NUIWindowType)luaL_checkinteger(L, 1), typeStr, str));

	return 0;
}

void SetJobPos(float x, float y, float z);

LUA_FUNCTION(SetJobIndicator)
{
	SetJobPos(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));

	std::string jobType = "setJob";
	std::string jobString = luaL_checkstring(L, 4);

	CefPostTask(TID_RENDERER, NewCefRunnableFunction(&SendUIStringDo, NUIWindowJob, jobType, jobString));

	return 0;
}

void Blur_SetAmount(float amt);

LUA_FUNCTION(SetBlurAmount)
{
	float blurAmount = luaL_checknumber(L, 1);

	Blur_SetAmount(blurAmount);

	return 0;
}
#endif