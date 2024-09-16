#pragma once
#include "IHost.h"
#include "NetLibrary.h"
#include "PacketHandler.h"

namespace fx
{
class IHostPacketHandler : public net::PacketHandler<net::packet::ServerIHost, HashRageString("msgIHost")>
{
	NetLibrary* m_netLibrary;

public:
	IHostPacketHandler(NetLibrary* netLibrary): m_netLibrary(netLibrary)
	{
	}

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerIHost& serverIHostPacket, NetLibrary* netLibrary)
		{
			uint16_t hostNetID = serverIHostPacket.netId;
			uint32_t hostBase = serverIHostPacket.baseNum;
			netLibrary->SetHost(hostNetID, hostBase);
		}, m_netLibrary);
	}
};
}
