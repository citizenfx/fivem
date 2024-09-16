#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "GameServer.h"
#include "IHost.h"

namespace fx
{
	namespace ServerDecorators
	{
		// Used from the client to announce itself as a new lobby host when the old host has disconnected.
		class IHostPacketHandler
		{
		public:
			IHostPacketHandler(fx::ServerInstanceBase* instance)
			{
			}

			void Handle(ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
			                          net::Buffer& packet)
			{
				if (IsOneSync())
				{
					return;
				}

				static size_t kClientMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ClientIHost>();

				if (packet.GetRemainingBytes() > kClientMaxPacketSize)
				{
					return;
				}

				net::packet::ClientIHost clientIHost;

				net::ByteReader reader{ packet.GetRemainingBytesPtr(), packet.GetRemainingBytes() };
				if (!clientIHost.Process(reader))
				{
					// this only happens when a malicious client sends packets not created from our client code
					return;
				}

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
				auto gameServer = instance->GetComponent<fx::GameServer>();

				auto currentHost = clientRegistry->GetHost();

				if (!currentHost || currentHost->IsDead())
				{
					client->SetNetBase(clientIHost.baseNum);
					clientRegistry->SetHost(client);

					net::Buffer hostBroadcast(net::SerializableComponent::GetSize<net::packet::ServerIHostPacket>());
					net::packet::ServerIHostPacket serverIHostPacket;
					serverIHostPacket.data.netId = client->GetNetId();
					serverIHostPacket.data.baseNum = client->GetNetBase();
					net::ByteWriter writer(hostBroadcast.GetBuffer(), hostBroadcast.GetLength());
					serverIHostPacket.Process(writer);
					hostBroadcast.Seek(writer.GetOffset());
					gameServer->Broadcast(hostBroadcast);
					//client->SendPacket(1, hostBroadcast, NetPacketType_Reliable);
				}
			}

			static constexpr const char* GetPacketId()
			{
				return "msgIHost";
			}
		};
	}
}
