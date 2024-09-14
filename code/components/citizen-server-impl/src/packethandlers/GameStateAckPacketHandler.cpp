#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "packethandlers/GameStateAckPacketHandler.h"

#include "ArrayUpdate.h"
#include "ByteReader.h"

void fx::ServerDecorators::GameStateAckPacketHandler::Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& buffer)
{
	static size_t kClientMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ClientGameStateAck>();

	if (buffer.GetRemainingBytes() > kClientMaxPacketSize)
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	net::packet::ClientGameStateAck clientGameStateAck;

	net::ByteReader reader{ buffer.GetRemainingBytesPtr(), buffer.GetRemainingBytes() };
	if (!clientGameStateAck.Process(reader))
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	gscomms_execute_callback_on_sync_thread([clientGameStateAck, client, instance, buffer = std::move(buffer)]() mutable
	{
		auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();
		sgs->HandleGameStateAck(instance, client, clientGameStateAck);
		// buffer needs to be moved to prevent packet memory from being freed
		(void)buffer;
	});
}
