#include <StdInc.h>

#include "packethandlers/StateBagPacketHandler.h"

#include <charconv>

#include "ByteReader.h"
#include "GameServer.h"
#include "KeyedRateLimiter.h"
#include "ResourceManager.h"
#include "StateBagComponent.h"

#include <state/ServerGameStatePublic.h>

bool StateBagPacketHandler::Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
                                   net::ByteReader& reader, fx::ENetPacketPtr& packet)
{
	const uint64_t offset = reader.GetOffset();
	gscomms_execute_callback_on_sync_thread([instance, client, offset, packet]
	{
		net::ByteReader movedReader (packet->data, packet->dataLength);
		movedReader.Seek(offset);
		HandleStateBagMessage(instance, client, movedReader);
		(void) packet;
	});

	return true;
}

void StateBagPacketHandler::HandleStateBagMessage(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader)
{
	static fx::RateLimiterStore<uint32_t, false> stateBagRateLimiterStore{
		instance->GetComponent<console::Context>().GetRef()
	};

	constexpr double kStateBagRateLimit = 75.f;
	constexpr double kStateBagRateLimitBurst = 125.f;
	static auto stateBagRateLimiter = stateBagRateLimiterStore.GetRateLimiter(
		"stateBag", fx::RateLimiterDefaults{kStateBagRateLimit, kStateBagRateLimitBurst});

	constexpr double kStateBagRateFloodLimit = 150.f;
	constexpr double kStateBagRateFloodLimitBurst = 175.f;
	static auto stateBagRateFloodLimiter = stateBagRateLimiterStore.GetRateLimiter(
		"stateBagFlood", fx::RateLimiterDefaults{kStateBagRateFloodLimit, kStateBagRateFloodLimitBurst});

	constexpr double kStateBagSizeLimit = 128 * 1024.0;
	constexpr double kStateBagSizeLimitBurst = 256 * 1024.0;
	static auto stateBagSizeRateLimiter = stateBagRateLimiterStore.GetRateLimiter(
		"stateBagSize", fx::RateLimiterDefaults{kStateBagSizeLimit, kStateBagSizeLimitBurst});

	static fx::KeyedRateLimiter<uint32_t, true> logLimiter{1.0, 1.0};

	auto resourceManager = instance->GetComponent<fx::ResourceManager>();

	auto stateBagComponent = resourceManager->GetComponent<fx::StateBagComponent>();

	const uint32_t netId = client->GetNetId();

	const bool hitRateLimit = !stateBagRateLimiter->Consume(netId);
	const bool hitFloodRateLimit = !stateBagRateFloodLimiter->Consume(netId);

	if (hitRateLimit)
	{
		const std::string& clientName = client->GetName();
		auto printStateWarning = [&clientName, netId](const std::string& logChannel, const std::string_view logReason,
		                                              const std::string_view rateLimiter, double rateLimit,
		                                              double burstRateLimit)
		{
			console::Printf(logChannel, logReason, clientName, netId);
			console::Printf(
				logChannel,
				"If you believe this to be a mistake please increase your rateLimiter_%s_rate and rateLimiter_%s_burst. ",
				rateLimiter, rateLimiter);
			console::Printf(
				logChannel,
				"You can do this with `set rateLimiter_%s_rate [new value]`. The default rate limit is %0.0f and burst limit is %0.0f\n",
				rateLimiter, rateLimit, burstRateLimit);
			console::Printf(
				logChannel,
				"You can disable this warning with `con_addChannelFilter %s drop` if you think you have this properly set up.\n",
				logChannel);
		};

		if (hitFloodRateLimit)
		{
			if (!client->IsDropping())
			{
				printStateWarning("sbag-client-flood",
				                  "Client %s %d got dropped for sending too many state bag value updates.\n",
				                  "stateBagFlood",
				                  kStateBagRateFloodLimit, kStateBagRateFloodLimitBurst);
			}

			instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverDropResourceName, fx::ClientDropReason::STATE_BAG_RATE_LIMIT, "Reliable state bag packet overflow.");
			return;
		}

		// only log here if we didn't log before, logging should only happen once in every 15 seconds.
		if (logLimiter.Consume(netId))
		{
			printStateWarning("sbag-update-dropped",
			                  "Client %s %d sent too many state bag updates and had their updates dropped.\n",
			                  "stateBag",
			                  kStateBagRateLimit, kStateBagRateLimitBurst);
		}

		return;
	}

	uint32_t dataLength = reader.GetRemaining();
	if (!stateBagSizeRateLimiter->Consume(netId, double(dataLength)))
	{
		if (!client->IsDropping())
		{
			const std::string& logChannel = "sbag-size-kick";
			console::Printf(logChannel, "Client %s %d got dropped for sending too large of a state bag update.\n",
			                client->GetName(), netId);
			console::Printf(
				logChannel, "If you believe this to be a mistake please increase your rateLimiter_stateBagSize_rate. ");
			console::Printf(
				logChannel,
				"You can do this with `set rateLimiter_stateBagSize_rate [new value]`. The default size rate limit is %0.0f.\n",
				kStateBagSizeLimit);
			console::Printf(
				logChannel,
				"You can disable this warning with `con_addChannelFilter %s drop` if you think you have this properly set up.\n",
				logChannel);
		}

		instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverDropResourceName, fx::ClientDropReason::STATE_BAG_RATE_LIMIT, "Reliable state bag packet overflow.");
		return;
	}

	net::packet::StateBag clientStateBag;

	if (!clientStateBag.Process(reader))
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	uint32_t slotId = client->GetSlotId();
	if (slotId != -1)
	{
		std::string bagNameOnFailure;

		stateBagComponent->HandlePacket(slotId, clientStateBag.data, &bagNameOnFailure);

		// state bag isn't present, apply conditions for automatic creation
		if (!bagNameOnFailure.empty())
		{
			// only allow clients to create entity state bags
			if (bagNameOnFailure.rfind("entity:", 0) == 0)
			{
				int entityID;
				auto result = std::from_chars(bagNameOnFailure.data() + 7,
				                              bagNameOnFailure.data() + bagNameOnFailure.size(), entityID);

				if (result.ec == std::errc()) // success
				{
					auto gameState = instance->GetComponent<fx::ServerGameStatePublic>();
					if (gameState->SetEntityStateBag(0, entityID, [stateBagComponent, bagNameOnFailure]
					{
						return stateBagComponent->RegisterStateBag(bagNameOnFailure);
					}))
					{
						// second attempt, should go through now
						stateBagComponent->HandlePacket(slotId, clientStateBag.data);
					}
				}
			}
		}
	}
}

bool StateBagPacketHandlerV2::Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
                                     net::ByteReader& reader, fx::ENetPacketPtr& packet)
{
	const uint64_t offset = reader.GetOffset();
	gscomms_execute_callback_on_sync_thread([instance, client, offset, packet]
	{
		net::ByteReader movedReader (packet->data, packet->dataLength);
		movedReader.Seek(offset);
		HandleStateBagMessage(instance, client, movedReader);
		(void) packet;
	});

	return true;
}

void StateBagPacketHandlerV2::HandleStateBagMessage(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader)
{
	static size_t kMaxPacketSize = net::SerializableComponent::GetMaxSize<net::packet::StateBagV2>();
	
	static fx::RateLimiterStore<uint32_t, false> stateBagRateLimiterStore{
		instance->GetComponent<console::Context>().GetRef()
	};

	constexpr double kStateBagRateLimit = 75.f;
	constexpr double kStateBagRateLimitBurst = 125.f;
	static auto stateBagRateLimiter = stateBagRateLimiterStore.GetRateLimiter(
		"stateBag", fx::RateLimiterDefaults{kStateBagRateLimit, kStateBagRateLimitBurst});

	constexpr double kStateBagRateFloodLimit = 150.f;
	constexpr double kStateBagRateFloodLimitBurst = 175.f;
	static auto stateBagRateFloodLimiter = stateBagRateLimiterStore.GetRateLimiter(
		"stateBagFlood", fx::RateLimiterDefaults{kStateBagRateFloodLimit, kStateBagRateFloodLimitBurst});

	constexpr double kStateBagSizeLimit = 128 * 1024.0;
	constexpr double kStateBagSizeLimitBurst = 256 * 1024.0;
	static auto stateBagSizeRateLimiter = stateBagRateLimiterStore.GetRateLimiter(
		"stateBagSize", fx::RateLimiterDefaults{kStateBagSizeLimit, kStateBagSizeLimitBurst});

	static fx::KeyedRateLimiter<uint32_t, true> logLimiter{1.0, 1.0};

	auto resourceManager = instance->GetComponent<fx::ResourceManager>();

	auto stateBagComponent = resourceManager->GetComponent<fx::StateBagComponent>();

	const uint32_t netId = client->GetNetId();

	const bool hitRateLimit = !stateBagRateLimiter->Consume(netId);
	const bool hitFloodRateLimit = !stateBagRateFloodLimiter->Consume(netId);

	if (hitRateLimit)
	{
		const std::string& clientName = client->GetName();
		auto printStateWarning = [&clientName, netId](const std::string& logChannel, const std::string_view logReason,
		                                              const std::string_view rateLimiter, double rateLimit,
		                                              double burstRateLimit)
		{
			console::Printf(logChannel, logReason, clientName, netId);
			console::Printf(
				logChannel,
				"If you believe this to be a mistake please increase your rateLimiter_%s_rate and rateLimiter_%s_burst. ",
				rateLimiter, rateLimiter);
			console::Printf(
				logChannel,
				"You can do this with `set rateLimiter_%s_rate [new value]`. The default rate limit is %0.0f and burst limit is %0.0f\n",
				rateLimiter, rateLimit, burstRateLimit);
			console::Printf(
				logChannel,
				"You can disable this warning with `con_addChannelFilter %s drop` if you think you have this properly set up.\n",
				logChannel);
		};

		if (hitFloodRateLimit)
		{
			if (!client->IsDropping())
			{
				printStateWarning("sbag-client-flood",
				                  "Client %s %d got dropped for sending too many state bag value updates.\n",
				                  "stateBagFlood",
				                  kStateBagRateFloodLimit, kStateBagRateFloodLimitBurst);
			}

			instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverDropResourceName, fx::ClientDropReason::STATE_BAG_RATE_LIMIT, "Reliable state bag packet overflow.");
			return;
		}

		// only log here if we didn't log before, logging should only happen once in every 15 seconds.
		if (logLimiter.Consume(netId))
		{
			printStateWarning("sbag-update-dropped",
			                  "Client %s %d sent too many state bag updates and had their updates dropped.\n",
			                  "stateBag",
			                  kStateBagRateLimit, kStateBagRateLimitBurst);
		}

		return;
	}

	size_t dataLength = reader.GetRemaining();
	if (!stateBagSizeRateLimiter->Consume(netId, double(dataLength)))
	{
		if (!client->IsDropping())
		{
			const std::string& logChannel = "sbag-size-kick";
			console::Printf(logChannel, "Client %s %d got dropped for sending too large of a state bag update.\n",
			                client->GetName(), netId);
			console::Printf(
				logChannel, "If you believe this to be a mistake please increase your rateLimiter_stateBagSize_rate. ");
			console::Printf(
				logChannel,
				"You can do this with `set rateLimiter_stateBagSize_rate [new value]`. The default size rate limit is %0.0f.\n",
				kStateBagSizeLimit);
			console::Printf(
				logChannel,
				"You can disable this warning with `con_addChannelFilter %s drop` if you think you have this properly set up.\n",
				logChannel);
		}

		instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverDropResourceName, fx::ClientDropReason::STATE_BAG_RATE_LIMIT, "Reliable state bag packet overflow.");
		return;
	}

	net::packet::StateBagV2 clientStateBag;
	if (!clientStateBag.Process(reader))
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	uint32_t slotId = client->GetSlotId();
	if (slotId != -1)
	{
		std::string_view bagNameOnFailure;

		stateBagComponent->HandlePacketV2(slotId, clientStateBag, &bagNameOnFailure);

		// state bag isn't present, apply conditions for automatic creation
		if (!bagNameOnFailure.empty())
		{
			// only allow clients to create entity state bags
			if (bagNameOnFailure.rfind("entity:", 0) == 0)
			{
				int entityID;
				auto result = std::from_chars(bagNameOnFailure.data() + 7,
				                              bagNameOnFailure.data() + bagNameOnFailure.size(), entityID);

				if (result.ec == std::errc()) // success
				{
					auto gameState = instance->GetComponent<fx::ServerGameStatePublic>();
					if (gameState->SetEntityStateBag(0, entityID, [stateBagComponent, bagNameOnFailure]
					{
						return stateBagComponent->RegisterStateBag(bagNameOnFailure);
					}))
					{
						// second attempt, should go through now
						stateBagComponent->HandlePacketV2(slotId, clientStateBag);
					}
				}
			}
		}
	}
}
