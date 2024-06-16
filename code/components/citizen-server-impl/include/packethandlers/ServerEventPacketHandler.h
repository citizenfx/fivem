#pragma once

#include <ServerInstanceBase.h>
#include <Client.h>
#include <NetBuffer.h>

#include "ComponentExport.h"

class ServerEventPacketHandler
{
public:
	ServerEventPacketHandler(fx::ServerInstanceBase* instance)
	{
	}

	void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
	                   net::Buffer& buffer);

	static constexpr const char* GetPacketId()
	{
		return "msgServerEvent";
	}
};
