#include "StdInc.h"
#include "ResourceScripting.h"
#include "ResourceManager.h"
#include "NetLibrary.h"

#include <CPlayerInfo.h>
#include <scrEngine.h>

LUA_FUNCTION(TriggerEvent)
{
	STACK_BASE;

	// get event name
	const char* eventName = luaL_checkstring(L, 1);

	// serialize arguments
	int nargs = lua_gettop(L);
	luaS_serializeArgs(L, 2, nargs - 1);

	size_t len;
	const char* jsonString = lua_tolstring(L, -1, &len);

	// call into resources
	TheResources.TriggerEvent(std::string(eventName), std::string(jsonString, len));

	// remove serialized string
	lua_pop(L, 1);

	STACK_CHECK;

	return 0;
}

static void SendNetEvent(std::string eventName, std::string jsonString, int i)
{
	const char* cmdType = "msgNetEvent";

	if (i >= 0)
	{
		auto info = CPlayerInfo::GetPlayer(i);

		if (!info)
		{
			return;
		}

		auto netID = info->address.inaOnline.s_addr;

		if (netID == g_netLibrary->GetServerNetID())
		{
			TheResources.QueueEvent(eventName, jsonString, i);

			return;
		}

		i = netID;
	}
	else if (i == -1)
	{
		i = UINT16_MAX;
	}
	else if (i == -2)
	{
		cmdType = "msgServerEvent";
	}

	size_t eventNameLength = eventName.length();

	NetBuffer buffer(100000);

	if (i >= 0)
	{
		buffer.Write<uint16_t>(i);
	}

	buffer.Write<uint16_t>(eventNameLength + 1);
	buffer.Write(eventName.c_str(), eventNameLength + 1);

	buffer.Write(jsonString.c_str(), jsonString.size());
	g_netLibrary->SendReliableCommand(cmdType, buffer.GetBuffer(), buffer.GetCurLength());
}

LUA_FUNCTION(TriggerRemoteEvent)
{
	STACK_BASE;

	// get event name
	const char* eventName = luaL_checkstring(L, 1);

	// get target client ids
	int targetClient = luaL_checkinteger(L, 2);

	// serialize arguments
	int nargs = lua_gettop(L);
	luaS_serializeArgs(L, 3, nargs - 2);

	size_t len;
	const char* jsonString = lua_tolstring(L, -1, &len);

	lua_pop(L, 1);

	// make event object
	SendNetEvent(eventName, std::string(jsonString, len), targetClient);

	STACK_CHECK;

	// done
	return 0;
}

LUA_FUNCTION(TriggerServerEvent)
{
	STACK_BASE;

	// get event name
	const char* eventName = luaL_checkstring(L, 1);

	// serialize arguments
	int nargs = lua_gettop(L);
	luaS_serializeArgs(L, 2, nargs - 1);

	size_t len;
	const char* jsonString = lua_tolstring(L, -1, &len);

	lua_pop(L, 1);

	// make event object
	SendNetEvent(eventName, std::string(jsonString, len), -2);

	STACK_CHECK;

	// done
	return 0;
}

static InitFunction initFunction([]()
{
	g_netLibrary->AddReliableHandler("msgNetEvent", [] (const char* buf, size_t len)
	{
		NetBuffer buffer(buf, len);

		// get the source net ID
		uint16_t sourceNetID = buffer.Read<uint16_t>();

		// get length of event name and read the event name
		static char eventName[65536];

		uint16_t nameLength = buffer.Read<uint16_t>();
		buffer.Read(eventName, nameLength);

		// read the data
		size_t dataLen = len - nameLength - (sizeof(uint16_t) * 2);
		char* eventData = new char[dataLen];

		buffer.Read(eventData, dataLen);

		// get the source player ID from the net ID
		uint16_t playerID = -1;

		for (int i = 0; i < 32; i++)
		{
			if (NativeInvoke::Invoke<0x4E237943, int>(i))
			{
				auto info = CPlayerInfo::GetPlayer(i);

				auto netID = info->address.inaOnline.s_addr;

				if (netID == sourceNetID)
				{
					playerID = i;
					break;
				}
			}
		}

		// probably a message from a since-disconnected-from-game's-vision player
		if (playerID == -1)
		{
			//return;
		}

		TheResources.QueueEvent(std::string(eventName), std::string(eventData, dataLen), playerID);
	});
});