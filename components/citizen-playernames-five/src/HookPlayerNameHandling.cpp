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

// needs to be kept in sync with gta:net:five
struct ScInAddr
{
	uint64_t unkKey1;
	uint64_t unkKey2;
	uint32_t secKeyTime; // added in 393
	uint32_t ipLan;
	uint16_t portLan;
	uint32_t ipUnk;
	uint16_t portUnk;
	uint32_t ipOnline;
	uint16_t portOnline;
	uint16_t pad3;
	uint32_t newVal; // added in 372
};

static const char* GetPlayerNameFromScAddr(ScInAddr* addr)
{
	int netId = (addr->ipLan & 0xFFFF) ^ 0xFEED;
	auto it = g_netIdToNames.find(netId);

	if (it == g_netIdToNames.end())
	{
		return "** Invalid **";
	}

	return it->second.c_str();
}

static HookFunction hookFunction([] ()
{
	// function that (hopefully) is only used for getting names from SC data blocks
	void* playerNameGetter = hook::pattern("49 8B CE FF 50 30 48 8B C8 E8 ? ? ? ? 48 8D").count(1).get(0).get<void>(9);

	hook::jump(hook::get_call(playerNameGetter), GetPlayerNameFromScAddr);
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