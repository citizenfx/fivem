#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "GameServer.h"

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

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
				auto gameServer = instance->GetComponent<fx::GameServer>();

				auto baseNum = packet.Read<uint32_t>();
				auto currentHost = clientRegistry->GetHost();

				if (!currentHost || currentHost->IsDead())
				{
					client->SetNetBase(baseNum);
					clientRegistry->SetHost(client);

					net::Buffer hostBroadcast;
					hostBroadcast.Write(fx::force_consteval<uint32_t, HashRageString("msgIHost")>);
					hostBroadcast.Write<uint16_t>(client->GetNetId());
					hostBroadcast.Write(client->GetNetBase());

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
