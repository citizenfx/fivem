#pragma once

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ComponentExport.h"

namespace fx
{
	namespace ServerDecorators
	{
		class ArrayUpdatePacketHandler
		{
		public:
			ArrayUpdatePacketHandler(fx::ServerInstanceBase* instance)
			{
			}

			void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& buffer);

			static constexpr const char* GetPacketId()
			{
				return "msgArrayUpdate";
			}
		};
	}
}
