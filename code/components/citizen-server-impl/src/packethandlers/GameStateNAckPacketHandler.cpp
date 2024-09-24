#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "packethandlers/GameStateNAckPacketHandler.h"

#include "ArrayUpdate.h"
#include "ByteReader.h"

void fx::ServerDecorators::GameStateNAckPacketHandler::Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& buffer)
{
	static size_t kClientMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ClientGameStateNAck>();

	if (buffer.GetRemainingBytes() > kClientMaxPacketSize)
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	net::packet::ClientGameStateNAck clientGameStateNAck;

	net::ByteReader reader{ buffer.GetRemainingBytesPtr(), buffer.GetRemainingBytes() };
	if (!clientGameStateNAck.Process(reader))
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	gscomms_execute_callback_on_sync_thread([clientGameStateNAck, client, instance, buffer = std::move(buffer)]() mutable
	{
		auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();
		sgs->HandleGameStateNAck(instance, client, clientGameStateNAck);
		// buffer needs to be moved to prevent packet memory from being freed
		(void)buffer;
	});
}
