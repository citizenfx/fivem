/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <NetLibrary.h>
#include <ResourceManager.h>

#include <msgpack.hpp>

static NetLibrary* g_netLibrary;
static std::unordered_map<int, std::string> g_netIdToNames;

// redefined here as game_ny migration still isn't complete
typedef struct
{
	IN_ADDR ina;
	IN_ADDR inaOnline;
	WORD wPortOnline;
	BYTE abEnet[6];
	BYTE abOnline[20];
} XNADDR;

class CPlayerInfo
{
public:
	XNADDR address; // 36 bytes long
};

static const char* __fastcall CPlayerInfo__GetName(CPlayerInfo* playerInfo)
{
	int netId = playerInfo->address.inaOnline.s_addr;

	return g_netIdToNames[netId].c_str();
}

static void InlineGetName(const char** outNamePtr, char* playerInfoOffset)
{
	CPlayerInfo* playerInfo = reinterpret_cast<CPlayerInfo*>(playerInfoOffset - 0x4C);

	*outNamePtr = CPlayerInfo__GetName(playerInfo);
}

static void __declspec(naked) InlineDrawNameHook()
{
	__asm
	{
		// preserve edx
		push edx

		// pass eax and a ptr to the value of eax on the stack to the subroutine that mingles with the stack
		push eax

		lea eax, [esp + 8]
		push eax

		call InlineGetName

		add esp, 8h

		pop edx

		// push '%s' anyway
		push 0E875ACh

		// and return to sender
		push 04799B4h
		retn
	}
}

static HookFunction hookFunction([] ()
{
	// CPlayerInfo non-inlined function
	hook::jump(0x75D2E0, CPlayerInfo__GetName);

	// network player names, inline sneaky asm hook
	hook::jump(0x4799AF, InlineDrawNameHook);
});

static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* netLibrary)
	{
		g_netLibrary = netLibrary;
	});

	ResourceManager::OnTriggerEvent.Connect([] (const fwString& eventName, const fwString& argsSerialized, int source)
	{
		// if this is the event 'we' handle...
		if (eventName == "onPlayerJoining")
		{
			// deserialize the arguments
			msgpack::unpacked msg;
			msgpack::unpack(msg, argsSerialized.c_str(), argsSerialized.size());

			msgpack::object obj = msg.get();

			// get the netid/name pair

			// convert to an array
			std::vector<msgpack::object> arguments;
			obj.convert(arguments);

			// get the fields from the dictionary, if existent
			if (arguments.size() >= 2)
			{
				// convert to the concrete types
				int netId = arguments[0].as<int>();
				std::string name = arguments[1].as<std::string>();

				// and add to the list
				g_netIdToNames[netId] = name;
			}
		}
	});
});