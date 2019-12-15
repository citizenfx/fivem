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
#include <ResourceEventComponent.h>

#include <msgpack.hpp>

#include <CoreNetworking.h>

static NetLibrary* g_netLibrary;
static std::unordered_map<int, std::string> g_netIdToNames;
static std::unordered_map<int, int> g_netIdToSlots;

static const char* GetPlayerNameFromGamerInfo(rlGamerInfo* addr)
{
	if (addr == (void*)0x20)
	{
		return "** Invalid **";
	}

	if ((addr->peerAddress.localAddr.ip.addr & 0xFFFF0000) != 0xC0A80000)
	{
		return addr->name;
	}

	int netId = (addr->peerAddress.localAddr.ip.addr & 0xFFFF) ^ 0xFEED;
	auto it = g_netIdToNames.find(netId);

	if (it == g_netIdToNames.end())
	{
		return va("player %d", netId);
	}

	return it->second.c_str();
}

static HookFunction hookFunction([]()
{
	// function that (hopefully, again!) is only used for getting names from SC data blocks
	void* playerNameGetter = hook::pattern("49 8B CC FF 50 60 48 8B C8 E8").count(1).get(0).get<void>(9);

	hook::jump(hook::get_call(playerNameGetter), GetPlayerNameFromGamerInfo);
});

// TODO: share code
static InitFunction initFunction([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		g_netLibrary = netLibrary;
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		fwRefContainer<fx::ResourceEventManagerComponent> eventComponent = manager->GetComponent<fx::ResourceEventManagerComponent>();

		if (eventComponent.GetRef())
		{
			eventComponent->OnTriggerEvent.Connect([](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
			{
				// if this is the event 'we' handle...
				if (eventName == "onPlayerJoining" || eventName == "onPlayerDropped")
				{
					try
					{
						// deserialize the arguments
						msgpack::unpacked msg;
						msgpack::unpack(msg, eventPayload.c_str(), eventPayload.size());

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

							if (eventName == "onPlayerJoining")
							{
								// and add to the list
								g_netIdToNames[netId] = name;
							}

							if (arguments.size() >= 3)
							{
								uint32_t slotId = arguments[2].as<uint32_t>();

								if (slotId != -1)
								{
									g_netIdToSlots[netId] = slotId;

									// trickle the event down the stack
									NetLibraryClientInfo clientInfo;
									clientInfo.netId = netId;
									clientInfo.name = name;
									clientInfo.slotId = slotId;

									if (eventName == "onPlayerJoining")
									{
										g_netLibrary->OnClientInfoReceived(clientInfo);
									}
									else if (eventName == "onPlayerDropped")
									{
										g_netLibrary->OnClientInfoDropped(clientInfo);
									}
								}
							}
						}
					}
					catch (std::runtime_error & e)
					{
						trace("Failed to unpack onPlayerJoining event: %s\n", e.what());
					}
				}
			});
		}
	}, 500);
});
