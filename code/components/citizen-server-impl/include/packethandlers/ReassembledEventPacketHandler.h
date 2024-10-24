#pragma once

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ComponentExport.h"
#include "ENetPacketUniquePtr.h"
#include "EventReassemblyComponent.h"
#include "PacketHandler.h"
#include "ReassembledEvent.h"

namespace fx
{
namespace ServerDecorators
{
	class ReassembledEventPacketHandler : public net::PacketHandler<net::packet::ReassembledEvent, HashRageString("msgReassembledEvent")>
	{
		std::shared_ptr<ConVar<bool>> m_enableNetEventReassemblyConVar;
		fwRefContainer<fx::EventReassemblyComponent> m_rac;
	public:
		COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) ReassembledEventPacketHandler(fx::ServerInstanceBase* instance);

		bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet);
	};
}
}
