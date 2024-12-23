#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include <ByteReader.h>

#include "packethandlers/ReassembledEventPacketHandler.h"

#include "EventReassemblyComponent.h"
#include "ReassembledEventPacket.h"
#include "ResourceManager.h"

namespace fx
{
std::shared_ptr<ConVar<bool>> g_enableNetEventReassemblyConVar;
}

fx::ServerDecorators::ReassembledEventPacketHandler::ReassembledEventPacketHandler(fx::ServerInstanceBase* instance): m_rac(instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::EventReassemblyComponent>())
{
}

bool fx::ServerDecorators::ReassembledEventPacketHandler::Process(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet)
{
	if (!g_enableNetEventReassemblyConVar->GetValue())
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

fx::ServerDecorators::ReassembledEventV2PacketHandler::ReassembledEventV2PacketHandler(fx::ServerInstanceBase* instance): m_rac(instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::EventReassemblyComponent>())
{
}

bool fx::ServerDecorators::ReassembledEventV2PacketHandler::Process(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet)
{
	if (!g_enableNetEventReassemblyConVar->GetValue())
	{
		return false;
	}

	return ProcessPacket(reader, [](net::packet::ReassembledEventV2& reassembledEvent, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, fwRefContainer<fx::EventReassemblyComponent>& rac, ENetPacketPtr& packet)
	{
		gscomms_execute_callback_on_main_thread([rac, reassembledEvent, client, packet]()
		{
			rac->HandlePacketV2(client->GetNetId(), reassembledEvent);
			// packet needs to be moved to prevent packet memory from being freed
			(void)packet;
		});
	}, instance, client, m_rac, packet);
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		fx::g_enableNetEventReassemblyConVar = instance->AddVariable<bool>("sv_enableNetEventReassembly", ConVar_None, true);
	});
});
