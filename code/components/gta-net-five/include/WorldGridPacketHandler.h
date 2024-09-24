#pragma once
#include "PacketHandler.h"
#include "WorldGrid.h"
#include "WorldGridState.h"

namespace fx
{
class WorldGridPacketHandler : public net::PacketHandler<net::packet::ServerWorldGrid, HashRageString("msgWorldGrid3")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerWorldGrid& serverWorldGrid)
		{
			const uint32_t base = serverWorldGrid.base;
			const size_t length = serverWorldGrid.data.GetValue().size();

			if (base + length > sizeof(g_worldGrid))
			{
				return;
			}

			memcpy(reinterpret_cast<uint8_t*>(&g_worldGrid) + base, serverWorldGrid.data.GetValue().data(), length);
		});
	}
};
}
