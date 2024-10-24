#pragma once

#include <ServerInstanceBase.h>
#include <Client.h>

#include "ByteReader.h"
#include "ComponentExport.h"
#include "ENetPacketUniquePtr.h"
#include "PacketHandler.h"
#include "ServerCommand.h"

class ServerCommandPacketHandler : public net::PacketHandler<net::packet::ClientServerCommand, HashRageString("msgServerCommand")>
{
	std::string rawCommand;
public:
	COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) ServerCommandPacketHandler(fx::ServerInstanceBase* instance);

	bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
	            net::ByteReader& reader, fx::ENetPacketPtr& packet);
};
