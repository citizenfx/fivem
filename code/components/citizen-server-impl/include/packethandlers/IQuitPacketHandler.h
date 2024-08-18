#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "GameServer.h"

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
				const size_t remainingBytes = packet.GetRemainingBytes();
				if (remainingBytes < 1)
				{
					instance->GetComponent<fx::GameServer>()->DropClientv(client, clientDropResourceName, fx::ClientDropReason::CLIENT, "");
					return;
				}

				const std::string reason = std::string(
					packet.Read<std::string_view>(std::min(remainingBytes - 1, static_cast<size_t>(1024))));
				instance->GetComponent<fx::GameServer>()->DropClientv(client, clientDropResourceName, fx::ClientDropReason::CLIENT, reason);
			}

			static constexpr const char* GetPacketId()
			{
				return "msgIQuit";
			}
		};
	}
}
