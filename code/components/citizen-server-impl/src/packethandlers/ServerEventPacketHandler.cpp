#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "GameServer.h"
#include "KeyedRateLimiter.h"
#include "ResourceEventComponent.h"
#include "ResourceManager.h"

#include "packethandlers/ServerEventPacketHandler.h"

void ServerEventPacketHandler::Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
                                      net::Buffer& buffer)
{
	static fx::RateLimiterStore<uint32_t, false> netEventRateLimiterStore{
		instance->GetComponent<console::Context>().GetRef()
	};
	static auto netEventRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netEvent", fx::RateLimiterDefaults{50.f, 200.f});
	static auto netFloodRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netEventFlood", fx::RateLimiterDefaults{75.f, 300.f});
	static auto netEventSizeRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netEventSize", fx::RateLimiterDefaults{128 * 1024.0, 384 * 1024.0});

	const uint32_t netId = client->GetNetId();

	if (!netEventRateLimiter->Consume(netId))
	{
		if (!netFloodRateLimiter->Consume(netId))
		{
			instance->GetComponent<fx::GameServer>()->DropClient(client, "Reliable network event overflow.");
		}

		return;
	}

	const uint16_t eventNameLength = buffer.Read<uint16_t>();

	// validate input, eventNameLength 1 would be zero, because we remove last byte
	if (eventNameLength <= 1)
	{
		return;
	}

	const std::string_view eventNameView = buffer.Read<std::string_view>(eventNameLength - 1);

	// read 0, maybe we can drop the uint8 in the future with a new net version update
	buffer.Read<uint8_t>();

	const uint32_t dataLength = static_cast<uint32_t>(buffer.GetRemainingBytes());

	if (!netEventSizeRateLimiter->Consume(netId, static_cast<double>(eventNameView.size() + dataLength)))
	{
		const std::string eventName(eventNameView);
		// if this happens, try increasing rateLimiter_netEventSize_rate and rateLimiter_netEventSize_burst
		// preferably, fix client scripts to not have this large a set of events with high frequency
		instance->GetComponent<fx::GameServer>()->DropClient(client, "Reliable network event size overflow: %s",
		                                                     eventName);
		return;
	}

	const std::string_view dataView = buffer.Read<std::string_view>(dataLength);

	const fwRefContainer<fx::ResourceManager> resourceManager = instance->GetComponent<fx::ResourceManager>();
	const fwRefContainer<fx::ResourceEventManagerComponent> eventManager = resourceManager->GetComponent<
		fx::ResourceEventManagerComponent>();

	eventManager->QueueEvent(
		std::string(eventNameView),
		std::string(dataView),
		"net:" + std::to_string(netId)
	);
}
