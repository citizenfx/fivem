#pragma once
#include "SerializableComponent.h"

namespace net
{
template<typename Packet, uint32_t PacketTypeHash>
class PacketHandler
{
public:
	static constexpr uint32_t PacketType = PacketTypeHash;

	template<typename T, class HandleFunc, typename ...Args>
	static bool ProcessPacket(T& stream, const HandleFunc&& handle, Args&&... args)
	{
		static size_t kMinSize = SerializableComponent::GetMinSize<Packet>();
		static size_t kMaxSize = SerializableComponent::GetMaxSize<Packet>();

		if ((stream.GetCapacity() - stream.GetOffset()) < kMinSize)
		{
			return false;
		}

		if ((stream.GetCapacity() - stream.GetOffset()) > kMaxSize)
		{
			return false;
		}

		Packet packet;
		if (!packet.Process(stream))
		{
			return false;
		}

		handle(packet, std::forward<Args>(args)...);
		return true;
	}
};
}
