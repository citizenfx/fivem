#pragma once
#include "PacketHandler.h"
#include "StateBag.h"
#include "StateBagComponent.h"

namespace fx
{
class StateBagPacketHandler : public net::PacketHandler<net::packet::StateBag, HashRageString("msgStateBag")>
{
	fwRefContainer<fx::StateBagComponent> m_sbac;
public:
	StateBagPacketHandler(const fwRefContainer<fx::StateBagComponent>& sbac): m_sbac(sbac)
	{
	}
	
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::StateBag& stateBag, const fwRefContainer<fx::StateBagComponent>& sbac)
		{
			sbac->HandlePacket(0, stateBag.data);
		}, m_sbac);
	}
};
}
