#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "ByteReader.h"
#include "Route.h"
#include "PacketHandler.h"

class RoutingPacketHandler : public net::PacketHandler<net::packet::ClientRoute, HashRageString("msgRoute")>
{
public:
	RoutingPacketHandler(fx::ServerInstanceBase* instance)
	{
	}

	bool Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr& packet)
	{
		return ProcessPacket(reader, [](net::packet::ClientRoute& clientRoute, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, fx::ENetPacketPtr& packet)
		{
			if (fx::IsOneSync())
			{
				client->SetHasRouted();

				gscomms_execute_callback_on_sync_thread([instance, client, clientRoute, packet]
				{
					instance->GetComponent<fx::ServerGameStatePublic>()->ParseGameStatePacket(client, clientRoute);
					(void)packet;
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
				outPacket.Write<uint16_t>(static_cast<uint16_t>(client->GetNetId()));
				outPacket.Write<uint16_t>(static_cast<uint16_t>(clientRoute.data.GetValue().size()));
				outPacket.Write(clientRoute.data.GetValue().data(), clientRoute.data.GetValue().size_bytes());

				targetClient->SendPacket(1, outPacket, NetPacketType_Unreliable);

				client->SetHasRouted();
			}
		}, instance, client, packet);
	}
};
