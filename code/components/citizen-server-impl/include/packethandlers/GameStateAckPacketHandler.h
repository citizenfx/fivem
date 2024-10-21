#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ByteReader.h"
#include "ComponentExport.h"
#include "ENetPacketUniquePtr.h"
#include "PacketHandler.h"
#include "GameStateAck.h"

namespace fx
{
namespace ServerDecorators
{
	class GameStateAckPacketHandler : public net::PacketHandler<net::packet::ClientGameStateAck, HashRageString("gameStateAck")>
	{
	public:
		GameStateAckPacketHandler(fx::ServerInstanceBase* instance)
		{
		}

		bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet);
	};
}
}
