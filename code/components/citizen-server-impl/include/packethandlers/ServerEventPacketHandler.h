#pragma once

#include <ServerInstanceBase.h>
#include <Client.h>
#include <NetBuffer.h>

class ServerEventPacketHandler
{
public:
	static void Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
	                   net::Buffer& buffer);

	static constexpr const char* GetPacketId()
	{
		return "msgServerEvent";
	}
};
