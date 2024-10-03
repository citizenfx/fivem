#pragma once

#include <ServerInstanceBase.h>
#include <Client.h>

#include "ByteReader.h"
#include "ComponentExport.h"
#include "NetEvent.h"
#include "PacketHandler.h"
#include "ENetPacketUniquePtr.h"

class ServerEventPacketHandler : public net::PacketHandler<net::packet::ClientServerEvent, HashRageString("msgServerEvent")>
{
public:
	ServerEventPacketHandler(fx::ServerInstanceBase* instance)
	{
	}

	bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
	                   net::ByteReader& buffer, fx::ENetPacketPtr& packet);
};
