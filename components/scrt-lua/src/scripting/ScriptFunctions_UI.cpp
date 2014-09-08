#include "StdInc.h"
#include "ResourceScripting.h"
#include "ResourceUI.h"
//#include "NetLibrary.h"

extern bool g_mainUIFlag;

struct ResUIResultHolder
{
	ResUIResultCallback cb;
};

static int CompleteCallback(lua_State* L)
{
	// get the result callback pointer
	auto holder = (ResUIResultHolder*)lua_topointer(L, lua_upvalueindex(1));
	auto function = holder->cb;

	STACK_BASE;

	delete holder; // yes, really.

	// serialize return value
	luaS_serializeArgsJSON(L, 1, 1);

	// to string and remove array wrapping thing
	size_t len;
	const char* string = lua_tolstring(L, -1, &len);

	fwString retValue = fwString(&string[1], len - 2);

	lua_pop(L, 1);

	// and call the callback
	function(retValue);

	STACK_CHECK;

	return 0;
}

LUA_FUNCTION(AddUIHandler)
{
	fwString messageType = luaL_checkstring(L, 1);

	lua_pushvalue(L, 2);
	int luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

	LuaScriptEnvironment* environment = GetCurrentLuaEnvironment();

	ResourceUI::GetForResource(GetCurrentLuaEnvironment()->GetResource())->AddCallback(messageType, [=] (fwString inData, ResUIResultCallback resultCB)
	{
		environment->EnqueueTask([=] ()
		{
			lua_State* m_luaState = environment->GetLua();
			STACK_BASE;

			// stack: 0
			lua_pushcfunction(m_luaState, lua_error_handler);
			int eh = lua_gettop(m_luaState);

			// stack: 1
			int length;
			int table = luaS_deserializeArgsJSON(m_luaState, &length, "[" + inData + "]");

			// stack: 3? (length (=1) + 1)

			// now replace the original table with the callback
			lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, luaRef);
			lua_replace(m_luaState, table);

			auto resultCBHolder = new ResUIResultHolder;
			resultCBHolder->cb = resultCB;

			lua_pushlightuserdata(m_luaState, resultCBHolder);
			lua_pushcclosure(m_luaState, CompleteCallback, 1);

			// call the method
			if (lua_pcall(m_luaState, length + 1, 0, eh) != 0)
			{
				std::string err = luaL_checkstring(m_luaState, -1);
				lua_pop(m_luaState, 1);

				GlobalError("Error during UI call handler: %s\nsee console for details", err.c_str());
			}

			lua_pop(m_luaState, 1);

			STACK_CHECK;
		});
	});

	return 0;
}

LUA_FUNCTION(PollUI)
{
	auto ui = ResourceUI::GetForResource(GetCurrentLuaEnvironment()->GetResource());

	if (ui.GetRef())
	{
		ui->SignalPoll();
	}

	return 0;
}

LUA_FUNCTION(SetUIFocus)
{
	nui::GiveFocus(lua_toboolean(L, 1));

	return 0;
}

LUA_FUNCTION(ShutdownNetworkCit)
{
	//g_netLibrary->Disconnect(luaL_checkstring(L, 1));

	//g_mainUIFlag = true;
	nui::SetMainUI(true);

	nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");

	return 0;
}