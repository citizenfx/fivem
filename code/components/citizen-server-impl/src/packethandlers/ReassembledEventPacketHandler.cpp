#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include <ByteReader.h>

#include "packethandlers/ReassembledEventPacketHandler.h"

#include "EventReassemblyComponent.h"
#include "ReassembledEvent.h"
#include "ResourceManager.h"

fx::ServerDecorators::ReassembledEventPacketHandler::ReassembledEventPacketHandler(fx::ServerInstanceBase* instance): m_enableNetEventReassemblyConVar(instance->AddVariable<bool>("sv_enableNetEventReassembly", ConVar_None, true)), m_rac(instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::EventReassemblyComponent>())
{
}

bool fx::ServerDecorators::ReassembledEventPacketHandler::Process(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet)
{
	if (!m_enableNetEventReassemblyConVar->GetValue())
	{
		return false;
	}

	return ProcessPacket(reader, [](net::packet::ReassembledEvent& reassembledEvent, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, fwRefContainer<fx::EventReassemblyComponent>& rac, ENetPacketPtr& packet)
	{
		gscomms_execute_callback_on_main_thread([rac, reassembledEvent, client, packet]()
		{
			rac->HandlePacket(client->GetNetId(), std::string_view{ reinterpret_cast<const char*>(reassembledEvent.data.GetValue().data()), reassembledEvent.data.GetValue().size() });
			// packet needs to be moved to prevent packet memory from being freed
			(void)packet;
		});
	}, instance, client, m_rac, packet);
}
