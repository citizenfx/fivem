#pragma once
#include <ENetPacketUniquePtr.h>

namespace fx
{
class ENetPacketInstance
{
	struct ENetPacketDeleter
	{
		void operator()(const ENetPacket* data)
		{
			delete data;
		}
	};

public:
	static ENetPacketPtr Create(void* data, size_t dataLength)
	{
		ENetPacket* packet = new ENetPacket();
		packet->data = static_cast<enet_uint8*>(data);
		packet->dataLength = dataLength;
		return { packet, ENetPacketDeleter() };
	}
};
}
