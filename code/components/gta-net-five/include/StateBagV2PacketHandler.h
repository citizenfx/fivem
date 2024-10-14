#pragma once
#include "PacketHandler.h"
#include "StateBag.h"
#include "StateBagComponent.h"

namespace fx
{
class StateBagV2PacketHandler : public net::PacketHandler<net::packet::StateBagV2, HashRageString("msgStateBagV2")>
{
	fwRefContainer<fx::StateBagComponent> m_sbac;
public:
	StateBagV2PacketHandler(const fwRefContainer<fx::StateBagComponent>& sbac): m_sbac(sbac)
	{
	}
	
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::StateBagV2& stateBag, const fwRefContainer<fx::StateBagComponent>& sbac)
		{
			sbac->HandlePacketV2(0, stateBag);
		}, m_sbac);
	}
};
}
