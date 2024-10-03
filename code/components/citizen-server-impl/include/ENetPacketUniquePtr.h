#pragma once
#include <enet/enet.h>

namespace fx
{
struct ENetPacketDeleter
{
	void operator()(ENetPacket* data)
	{
		enet_packet_destroy(data);
	}
};

using ENetPacketPtr = std::shared_ptr<ENetPacket>;
}
