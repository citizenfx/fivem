#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "packethandlers/ArrayUpdatePacketHandler.h"

#include "ArrayUpdate.h"
#include "ByteReader.h"
#include "KeyedRateLimiter.h"

void fx::ServerDecorators::ArrayUpdatePacketHandler::Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& buffer)
{
	static RateLimiterStore<uint32_t, false> arrayHandlerLimiterStore{ instance->GetComponent<console::Context>().GetRef() };
	static auto arrayUpdateRateLimiter = arrayHandlerLimiterStore.GetRateLimiter("arrayUpdate", fx::RateLimiterDefaults{ 75.f, 125.f });

	if (!arrayUpdateRateLimiter->Consume(client->GetNetId()))
	{
		return;
	}
	
	static size_t kClientMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ClientArrayUpdate>();

	if (buffer.GetRemainingBytes() > kClientMaxPacketSize)
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	net::packet::ClientArrayUpdate clientArrayUpdate;

	net::ByteReader reader{ buffer.GetRemainingBytesPtr(), buffer.GetRemainingBytes() };
	if (!clientArrayUpdate.Process(reader))
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();
	sgs->HandleArrayUpdate(client, clientArrayUpdate);
}
