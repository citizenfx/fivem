#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ComponentExport.h"

class StateBagPacketHandler
{
public:
	StateBagPacketHandler(fx::ServerInstanceBase* instance)
	{
	}

	void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet);

	static void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) HandleStateBagMessage(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, const net::Buffer& packet);

	static constexpr const char* GetPacketId()
	{
		return "msgStateBag";
	}
};

class StateBagPacketHandlerV2
{
public:
	StateBagPacketHandlerV2(fx::ServerInstanceBase* instance)
	{
	}

	void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet);

	static void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) HandleStateBagMessage(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, const net::Buffer& buffer);

	static constexpr const char* GetPacketId()
	{
		return "msgStateBagV2";
	}
};
