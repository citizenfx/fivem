#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ComponentExport.h"

namespace fx
{
namespace ServerDecorators
{
	class GameStateNAckPacketHandler
	{
	public:
		GameStateNAckPacketHandler(fx::ServerInstanceBase* instance)
		{
		}

		void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet);

		static constexpr const char* GetPacketId()
		{
			return "gameStateNAck";
		}
	};
}
}
