#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "GameServer.h"
#include "IHost.h"

#include "PacketHandler.h"

namespace fx
{
	namespace ServerDecorators
	{
		// Used from the client to announce itself as a new lobby host when the old host has disconnected.
		class IHostPacketHandler : public net::PacketHandler<net::packet::ClientIHost, HashRageString("msgIHost")>
		{
		public:
			IHostPacketHandler(fx::ServerInstanceBase* instance)
			{
			}

			bool Process(ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
			                          net::ByteReader& reader, fx::ENetPacketPtr& packet)
			{
				if (IsOneSync())
				{
					return false;
				}

				return ProcessPacket(reader, [](net::packet::ClientIHost& clientIHost, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client)
				{
					auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
					auto gameServer = instance->GetComponent<fx::GameServer>();

					auto currentHost = clientRegistry->GetHost();

					if (!currentHost || currentHost->IsDead())
					{
						client->SetNetBase(clientIHost.baseNum);
						clientRegistry->SetHost(client);

						net::Buffer hostBroadcast(net::SerializableComponent::GetMaxSize<net::packet::ServerIHostPacket>());
						net::packet::ServerIHostPacket serverIHostPacket;
						serverIHostPacket.data.netId = client->GetNetId();
						serverIHostPacket.data.baseNum = client->GetNetBase();
						net::ByteWriter writer(hostBroadcast.GetBuffer(), hostBroadcast.GetLength());
						serverIHostPacket.Process(writer);
						hostBroadcast.Seek(writer.GetOffset());
						gameServer->Broadcast(hostBroadcast);
						//client->SendPacket(1, hostBroadcast, NetPacketType_Reliable);
					}
				}, instance, client);
			}
		};
	}
}
