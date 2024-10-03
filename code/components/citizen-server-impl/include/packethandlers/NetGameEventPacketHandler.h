#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ComponentExport.h"
#include "ENetPacketUniquePtr.h"
#include "PacketHandler.h"

class NetGameEventPacketHandlerV2 : public net::PacketHandler<net::packet::ClientNetGameEventV2, HashRageString("msgNetGameEventV2")>
{
public:
	NetGameEventPacketHandlerV2(fx::ServerInstanceBase* instance)
	{
	}

	static void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) RouteEvent(const fwRefContainer<fx::ServerGameStatePublic>& sgs, uint32_t bucket, const std::vector<uint16_t>& targetPlayers, const fwRefContainer<fx::ClientRegistry>& clientRegistry, const net::Buffer& data);

	bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr& packet);
	
	static bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) ProcessNetEvent(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader);
};
