#pragma once

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ComponentExport.h"
#include "EventReassemblyComponent.h"

namespace fx
{
namespace ServerDecorators
{
	class ReassembledEventPacketHandler
	{
		std::shared_ptr<ConVar<bool>> m_enableNetEventReassemblyConVar;
		fwRefContainer<fx::EventReassemblyComponent> m_rac;
	public:
		COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) ReassembledEventPacketHandler(fx::ServerInstanceBase* instance);

		void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Handle(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& buffer);

		static constexpr const char* GetPacketId()
		{
			return "msgReassembledEvent";
		}
	};
}
}
