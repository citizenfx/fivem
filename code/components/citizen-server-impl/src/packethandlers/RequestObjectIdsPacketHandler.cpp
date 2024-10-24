﻿#include <StdInc.h>

#include "packethandlers/RequestObjectIdsPacketHandler.h"

#include "GameServer.h"
#include "ResourceManager.h"

#include <state/ServerGameStatePublic.h>

#include "ByteWriter.h"
#include "ObjectIds.h"

template<bool BigMode>
void SendObjectIdsToClient(const fx::ClientSharedPtr& client, const std::vector<uint16_t>& freeIds)
{
	static size_t kMaxResponseSize = net::SerializableComponent::GetMaxSize<net::packet::ServerObjectIdsPacket<BigMode>>();
	static std::vector<typename net::packet::ServerObjectIds<BigMode>::IdEntry> idContainer;

	idContainer.clear();
	net::Buffer responseBuffer(kMaxResponseSize);
	net::packet::ServerObjectIdsPacket<BigMode> packet;
	packet.data.SetIds(freeIds, idContainer);

	net::ByteWriter writer{ responseBuffer.GetBuffer(), kMaxResponseSize };
	if (!packet.Process(writer))
	{
		trace("Serialization of the server object id response failed. Please report this error at https://github.com/citizenfx/fivem.\n");
		return;
	}

	responseBuffer.Seek(writer.GetOffset());

	client->SendPacket(1, responseBuffer, NetPacketType_Reliable);
}

bool RequestObjectIdsPacketHandler::Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr& packet)
{
	gscomms_execute_callback_on_sync_thread([instance, client]
	{
		SendObjectIds(instance, client, fx::IsBigMode() ? 6 : 32);
	});

	return true;
}

void RequestObjectIdsPacketHandler::SendObjectIds(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, const uint8_t numIds)
{
	std::vector<uint16_t> freeIds;
	freeIds.reserve(numIds);
	auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();
	sgs->GetFreeObjectIds(client, numIds, freeIds);

	if (fx::IsBigMode())
	{
		SendObjectIdsToClient<true>(client, freeIds);
	}
	else
	{
		SendObjectIdsToClient<false>(client, freeIds);
	}
}
