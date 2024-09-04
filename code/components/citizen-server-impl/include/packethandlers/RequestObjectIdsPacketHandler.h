#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ComponentExport.h"

class RequestObjectIdsPacketHandler
{
public:
	RequestObjectIdsPacketHandler(fx::ServerInstanceBase* instance)
	{
	}

	void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet);

	// helper method to send object ids to a client
	static void SendObjectIds(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, uint8_t numIds);
	
	static constexpr const char* GetPacketId()
	{
		return "msgRequestObjectIds";
	}
};
