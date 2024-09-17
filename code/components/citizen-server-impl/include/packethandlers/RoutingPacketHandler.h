#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "ByteReader.h"
#include "Route.h"

class RoutingPacketHandler
{
public:
	RoutingPacketHandler(fx::ServerInstanceBase* instance)
	{
	}
	
	void Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet)
	{
		static size_t kClientMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ClientRoute>();

		if (packet.GetRemainingBytes() > kClientMaxPacketSize)
		{
			// this only happens when a malicious client sends packets not created from our client code
			return;
		}

		net::packet::ClientRoute clientRoute;

		net::ByteReader reader{ packet.GetRemainingBytesPtr(), packet.GetRemainingBytes() };
		if (!clientRoute.Process(reader))
		{
			// this only happens when a malicious client sends packets not created from our client code
			return;
		}

		if (fx::IsOneSync())
		{
			std::vector packetDataCopy = std::vector(clientRoute.data.GetValue().begin(), clientRoute.data.GetValue().end());
			client->SetHasRouted();

			gscomms_execute_callback_on_sync_thread([instance, client, packetDataCopy = std::move(packetDataCopy)]()
			{
				instance->GetComponent<fx::ServerGameStatePublic>()->ParseGameStatePacket(client, packetDataCopy);
			});

			return;
		}

		// TODO: in future net version remove targetNetId when the server is using onesync
		const uint16_t targetNetId = clientRoute.targetNetId;

		if (targetNetId == client->GetNetId())
		{
			// source can't be target
			return;
		}

		if (const auto targetClient = instance->GetComponent<fx::ClientRegistry>()->GetClientByNetID(targetNetId))
		{
			net::Buffer outPacket;
			outPacket.Write(HashRageString("msgRoute"));
			outPacket.Write<uint16_t>(client->GetNetId());
			outPacket.Write<uint16_t>(clientRoute.data.GetValue().size());
			outPacket.Write(clientRoute.data.GetValue().data(), clientRoute.data.GetValue().size_bytes());

			targetClient->SendPacket(1, outPacket, NetPacketType_Unreliable);

			client->SetHasRouted();
		}
	}

	static constexpr const char* GetPacketId()
	{
		return "msgRoute";
	}
};
