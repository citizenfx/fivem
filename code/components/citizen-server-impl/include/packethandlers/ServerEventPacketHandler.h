#pragma once

#include <ServerInstanceBase.h>
#include <Client.h>
#include <NetBuffer.h>

#include "ComponentExport.h"

class ServerEventPacketHandler
{
public:
	static COMPONENT_EXPORT(citizen_server_impl) void Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
	                   net::Buffer& buffer);

	static constexpr const char* GetPacketId()
	{
		return "msgServerEvent";
	}
};
