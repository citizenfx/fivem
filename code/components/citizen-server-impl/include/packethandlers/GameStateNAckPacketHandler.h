#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ByteReader.h"
#include "ComponentExport.h"
#include "ENetPacketUniquePtr.h"
#include "PacketHandler.h"

namespace fx
{
namespace ServerDecorators
{
	class GameStateNAckPacketHandler : public net::PacketHandler<net::packet::ClientGameStateNAck, HashRageString("gameStateNAck")>
	{
	public:
		GameStateNAckPacketHandler(fx::ServerInstanceBase* instance)
		{
		}

		bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet);
	};
}
}
