#pragma once
#include "PacketHandler.h"
#include "ReassembledEvent.h"

namespace fx
{
extern ConVar<bool> g_enableEventReassembly;

class ReassembledEventPacketHandler : public net::PacketHandler<net::packet::ReassembledEvent, HashRageString("msgReassembledEvent")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ReassembledEvent& reassembleEvent)
		{
			if (!g_enableEventReassembly.GetValue())
			{
				return;
			}

			const auto reassembler = Instance<fx::ResourceManager>::Get()->GetComponent<fx::EventReassemblyComponent>();
			reassembler->HandlePacket(0, std::string_view{ reinterpret_cast<const char*>(reassembleEvent.data.GetValue().data()), reassembleEvent.data.GetValue().size() });
		});
	}
};
}
