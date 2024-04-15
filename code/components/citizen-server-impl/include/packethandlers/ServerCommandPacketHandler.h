#pragma once

#include <ServerInstanceBase.h>
#include <Client.h>
#include <NetBuffer.h>

#include "ComponentExport.h"

class ServerCommandPacketHandler
{
	std::string rawCommand;

public:
	COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) ServerCommandPacketHandler(fx::ServerInstanceBase* instance);

	void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
	            net::Buffer& buffer);

	static constexpr const char* GetPacketId()
	{
		return "msgServerCommand";
	}
};
