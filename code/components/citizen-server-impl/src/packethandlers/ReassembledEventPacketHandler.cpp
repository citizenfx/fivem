#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <SerializableComponent.h>

#include <ByteReader.h>
#include <ByteWriter.h>

#include "packethandlers/ReassembledEventPacketHandler.h"

#include "EventReassemblyComponent.h"
#include "ReassembledEvent.h"
#include "ResourceManager.h"

fx::ServerDecorators::ReassembledEventPacketHandler::ReassembledEventPacketHandler(fx::ServerInstanceBase* instance): m_enableNetEventReassemblyConVar(instance->AddVariable<bool>("sv_enableNetEventReassembly", ConVar_None, true)), m_rac(instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::EventReassemblyComponent>())
{
}

void fx::ServerDecorators::ReassembledEventPacketHandler::Handle(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& buffer)
{
	if (!m_enableNetEventReassemblyConVar->GetValue())
	{
		return;
	}

	static size_t kMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ReassembledEvent>();

	if (buffer.GetRemainingBytes() > kMaxPacketSize)
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	net::packet::ReassembledEvent reassembledEvent;

	net::ByteReader reader{ buffer.GetRemainingBytesPtr(), buffer.GetRemainingBytes() };
	if (!reassembledEvent.Process(reader))
	{
		// this only happens when a malicious client sends packets not created from our client code
		return;
	}

	m_rac->HandlePacket(client->GetNetId(), std::string_view{ reinterpret_cast<const char*>(reassembledEvent.data.GetValue().data()), reassembledEvent.data.GetValue().size() });
}
