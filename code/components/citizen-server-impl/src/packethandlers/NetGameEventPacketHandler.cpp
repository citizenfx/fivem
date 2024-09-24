#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include <NetGameEventPacket.h>
#include <SerializableComponent.h>

#include <ByteReader.h>
#include <ByteWriter.h>
#include <RoundToType.h>

#include <EASTL/bitset.h>

#include "packethandlers/NetGameEventPacketHandler.h"

void NetGameEventPacketHandlerV2::RouteEvent(const fwRefContainer<fx::ServerGameStatePublic>& sgs, uint32_t bucket, const std::vector<uint16_t>& targetPlayers, const fwRefContainer<fx::ClientRegistry>& clientRegistry, const net::Buffer& data)
{
	for (uint16_t player : targetPlayers)
	{
		auto targetClient = clientRegistry->GetClientByNetID(player);

		if (!targetClient)
		{
			continue;
		}

		targetClient->SendPacket(1, data, NetPacketType_Reliable);
	}
}

void NetGameEventPacketHandlerV2::Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& buffer)
{
	static size_t kClientMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ClientNetGameEventV2>();
	static size_t kServerMaxReplySize = net::SerializableComponent::GetSize<net::packet::ServerNetGameEventV2Packet>();

	if (buffer.GetLength() > kClientMaxPacketSize)
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	net::packet::ClientNetGameEventV2 clientNetEvent;

	net::ByteReader reader{ buffer.GetRemainingBytesPtr(), buffer.GetRemainingBytes() };
	if (!clientNetEvent.Process(reader))
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();

	net::Span<uint16_t>& targetPlayersSpan = clientNetEvent.targetPlayers.GetValue();

	// todo: generate vector and filter duplicates and none relevant entities

	net::packet::ServerNetGameEventV2Packet serverNetEventPacket;
	auto& serverNetEvent = serverNetEventPacket.event;
	serverNetEvent.clientNetId = static_cast<uint16_t>(client->GetNetId());
	serverNetEvent.eventNameHash = clientNetEvent.eventNameHash;
	serverNetEvent.eventId = clientNetEvent.eventId;
	serverNetEvent.isReply = clientNetEvent.isReply;
	serverNetEvent.data = clientNetEvent.data;

	net::Buffer routingBuffer(kServerMaxReplySize);
	net::ByteWriter routingWriter{ const_cast<uint8_t*>(routingBuffer.GetBuffer()), kServerMaxReplySize };

	if (!serverNetEventPacket.Process(routingWriter))
	{
		trace("Serialization of the server net event failed. Please report this error at https://github.com/citizenfx/fivem.\n");
		return;
	}

	// adjust net::Buffer to the offset of the net::ByteWriter
	routingBuffer.Seek(routingWriter.GetOffset());

	auto bucket = sgs->GetClientRoutingBucket(client);

	auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

	// todo: use std::vector<fx::ClientSharedPtr> targetPlayers; when msgNetGameEvent is removed
	std::vector<uint16_t> targetPlayers;
	targetPlayers.reserve(targetPlayersSpan.size());

	thread_local eastl::bitset<net::roundToType<size_t>(MAX_CLIENTS)> processed;
	processed.reset();

	for (const uint16_t player : targetPlayersSpan)
	{
		// de-duplicate targetPlayers, preventing the sending of a large number of events to a single client
		if (processed.test(player))
		{
			continue;
		}

		// note that the player already got processed
		processed.set(player);

		auto targetClient = clientRegistry->GetClientByNetID(player);

		if (!targetClient)
		{
			continue;
		}

		if (const uint32_t targetClientRoutingBucket = sgs->GetClientRoutingBucket(targetClient);
			targetClientRoutingBucket != bucket)
		{
			continue;
		}

		if (!sgs->IsClientRelevantEntity(client, targetClient->GetSlotId()))
		{
			// the client that send the event can't see this target
			continue;
		}

		targetPlayers.push_back(player);
	}

	if (auto eventHandler = sgs->GetGameEventHandlerWithEvent(client, targetPlayers, clientNetEvent))
	{
		gscomms_execute_callback_on_main_thread(
		[sgs, bucket, targetPlayers = std::move(targetPlayers), clientRegistry, eventHandler, routingBuffer = std::move(routingBuffer)]()
		{
			if (eventHandler())
			{
				RouteEvent(sgs, bucket, targetPlayers, clientRegistry, routingBuffer);
			}
		});
	}
	else
	{
		// routing buffer is written on the same thread and don't need a extra copy
		RouteEvent(sgs, bucket, targetPlayers, clientRegistry, routingBuffer);
	}
}
