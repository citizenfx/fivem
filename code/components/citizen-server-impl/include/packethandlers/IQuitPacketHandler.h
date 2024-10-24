#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "GameServer.h"
#include "IQuit.h"

#include "PacketHandler.h"

namespace fx
{
	namespace ServerDecorators
	{
		//  Used from the client to signal to the server that the client wants to quit with the given reason.
		class IQuitPacketHandler : public net::PacketHandler<net::packet::ClientIQuit, HashRageString("msgIQuit")>
		{
		public:
			IQuitPacketHandler(fx::ServerInstanceBase* instance)
			{
			}

			bool Process(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr& packet)
			{
				if (reader.GetRemaining() < 1)
				{
					instance->GetComponent<fx::GameServer>()->DropClientv(client, clientDropResourceName, fx::ClientDropReason::CLIENT, "");
					return false;
				}

				const bool result = ProcessPacket(reader, [](net::packet::ClientIQuit& clientIQuit, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client)
				{
					instance->GetComponent<fx::GameServer>()->DropClientv(client, clientDropResourceName, fx::ClientDropReason::CLIENT, std::string(std::string_view(clientIQuit.reason.GetValue().data(), clientIQuit.reason.GetValue().size() - 1)));
				}, instance, client);

				if (!result)
				{
					instance->GetComponent<fx::GameServer>()->DropClientv(client, clientDropResourceName, fx::ClientDropReason::CLIENT, "");
				}

				return result;
			}
		};
	}
}
