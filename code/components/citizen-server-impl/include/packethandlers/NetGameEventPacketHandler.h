#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ComponentExport.h"

class NetGameEventPacketHandlerV2
{
public:
	NetGameEventPacketHandlerV2(fx::ServerInstanceBase* instance)
	{
	}

	static void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) RouteEvent(const fwRefContainer<fx::ServerGameStatePublic>& sgs, uint32_t bucket, const std::vector<uint16_t>& targetPlayers, const fwRefContainer<fx::ClientRegistry>& clientRegistry, const net::Buffer& data);
	
	void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet);

	static constexpr const char* GetPacketId()
	{
		return "msgNetGameEventV2";
	}
};
