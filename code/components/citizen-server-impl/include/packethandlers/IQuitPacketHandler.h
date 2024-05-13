#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <iostream>

#include "GameServer.h"

namespace fx
{
	namespace ServerDecorators
	{
		//  Used from the client to signal to the server that the client wants to quit with the given reason.
		class IQuitPacketHandler
		{
		public:
			static void Handle(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet)
			{
				const std::string reason = std::string(
					packet.Read<std::string_view>(std::min(packet.GetRemainingBytes(), static_cast<size_t>(1024))));
				instance->GetComponent<fx::GameServer>()->DropClientv(client, reason);
			}

			static constexpr const char* GetPacketId()
			{
				return "msgIQuit";
			}
		};
	}
}
