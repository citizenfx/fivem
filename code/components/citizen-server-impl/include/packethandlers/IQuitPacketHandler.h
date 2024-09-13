#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "GameServer.h"
#include "IQuit.h"

namespace fx
{
	namespace ServerDecorators
	{
		//  Used from the client to signal to the server that the client wants to quit with the given reason.
		class IQuitPacketHandler
		{
		public:
			IQuitPacketHandler(fx::ServerInstanceBase* instance)
			{
			}

			void Handle(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet)
			{
				static size_t kClientMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ClientIQuit>();

				if (packet.GetRemainingBytes() < 1 || packet.GetRemainingBytes() > kClientMaxPacketSize)
				{
					instance->GetComponent<fx::GameServer>()->DropClientv(client, clientDropResourceName, fx::ClientDropReason::CLIENT, "");
					return;
				}

				net::packet::ClientIQuit clientIQuit;

				net::ByteReader reader{ packet.GetRemainingBytesPtr(), packet.GetRemainingBytes() - 1 };
				if (!clientIQuit.Process(reader))
				{
					instance->GetComponent<fx::GameServer>()->DropClientv(client, clientDropResourceName, fx::ClientDropReason::CLIENT, "");
					return;
				}

				instance->GetComponent<fx::GameServer>()->DropClientv(client, clientDropResourceName, fx::ClientDropReason::CLIENT, std::string(clientIQuit.reason.GetValue()));
			}

			static constexpr const char* GetPacketId()
			{
				return "msgIQuit";
			}
		};
	}
}
