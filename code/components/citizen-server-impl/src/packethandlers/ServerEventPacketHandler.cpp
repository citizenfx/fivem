﻿#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "GameServer.h"
#include "KeyedRateLimiter.h"
#include "ResourceEventComponent.h"
#include "ResourceManager.h"

#include "packethandlers/ServerEventPacketHandler.h"

#include "ByteReader.h"
#include "NetEvent.h"

bool ServerEventPacketHandler::Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr& packet)
{
	static fx::RateLimiterStore<uint32_t, false> netEventRateLimiterStore{
		instance->GetComponent<console::Context>().GetRef()
	};
	static auto netEventRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netEvent", fx::RateLimiterDefaults{ 50.f, 200.f });
	static auto netFloodRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netEventFlood", fx::RateLimiterDefaults{ 75.f, 300.f });
	static auto netEventSizeRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netEventSize", fx::RateLimiterDefaults{ 128 * 1024.0, 384 * 1024.0 });

	return ProcessPacket(reader, [](net::packet::ClientServerEvent& clientServerEvent, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client)
	{
		const uint32_t netId = client->GetNetId();

		if (!netEventRateLimiter->Consume(netId))
		{
			if (!netFloodRateLimiter->Consume(netId))
			{
				instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverDropResourceName, fx::ClientDropReason::NET_EVENT_RATE_LIMIT, "Reliable network event overflow.");
			}

			return;
		}

		const std::string_view eventNameView = { reinterpret_cast<const char*>(clientServerEvent.eventName.GetValue().data()), clientServerEvent.eventName.GetValue().size() - 1 };

		const uint32_t dataLength = static_cast<uint32_t>(clientServerEvent.eventData.GetValue().size());

		if (!netEventSizeRateLimiter->Consume(netId, static_cast<double>(eventNameView.size() + dataLength)))
		{
			const std::string eventName(eventNameView);
			// if this happens, try increasing rateLimiter_netEventSize_rate and rateLimiter_netEventSize_burst
			// preferably, fix client scripts to not have this large a set of events with high frequency
			instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverDropResourceName, fx::ClientDropReason::NET_EVENT_RATE_LIMIT, "Reliable network event size overflow: %s",
			eventName);
			return;
		}

		const std::string_view dataView = { reinterpret_cast<const char*>(clientServerEvent.eventData.GetValue().data()), clientServerEvent.eventData.GetValue().size() };

		const fwRefContainer<fx::ResourceManager> resourceManager = instance->GetComponent<fx::ResourceManager>();
		const fwRefContainer<fx::ResourceEventManagerComponent> eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

		eventManager->QueueEvent(
			std::string(eventNameView),
			std::string(dataView),
			"net:" + std::to_string(netId)
		);
	}, instance, client);
}
