#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "packethandlers/ArrayUpdatePacketHandler.h"

#include "ArrayUpdate.h"
#include "ByteReader.h"
#include "KeyedRateLimiter.h"

bool fx::ServerDecorators::ArrayUpdatePacketHandler::Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet)
{
	static RateLimiterStore<uint32_t, false> arrayHandlerLimiterStore{ instance->GetComponent<console::Context>().GetRef() };
	static auto arrayUpdateRateLimiter = arrayHandlerLimiterStore.GetRateLimiter(
		"arrayUpdate", fx::RateLimiterDefaults{ 75.f, 125.f }
	);

	if (!arrayUpdateRateLimiter->Consume(client->GetNetId()))
	{
		return false;
	}

	return ProcessPacket(reader, [](net::packet::ClientArrayUpdate& clientArrayUpdate, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, ENetPacketPtr& packet)
	{
		gscomms_execute_callback_on_sync_thread([clientArrayUpdate, instance, client, packet]() mutable
		{
			auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();
			sgs->HandleArrayUpdate(client, clientArrayUpdate);
			// packet needs to be moved to prevent packet memory from being freed
			(void)packet;
		});
	}, instance, client, packet);
}
