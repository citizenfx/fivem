#pragma once
#include "NetEvent.h"
#include "PacketHandler.h"
#include "ResourceEventComponent.h"
#include "ResourceManager.h"

namespace fx
{
class NetEventPacketHandler : public net::PacketHandler<net::packet::ServerClientEvent, HashRageString("msgNetEvent")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerClientEvent& serverClientEvent)
		{
			// convert the source net ID to a string
			std::string source = "net:" + std::to_string(serverClientEvent.sourceNetId);

			// get the resource manager and eventing component
			static fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
			static fwRefContainer<fx::ResourceEventManagerComponent> eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

			// and queue the event
			eventManager->QueueEvent(std::string(reinterpret_cast<const char*>(serverClientEvent.eventName.GetValue().data()), serverClientEvent.eventName.GetValue().size() - 1), std::string(reinterpret_cast<const char*>(serverClientEvent.eventData.GetValue().data()), serverClientEvent.eventData.GetValue().size()), source);
		});
	}
};
}
