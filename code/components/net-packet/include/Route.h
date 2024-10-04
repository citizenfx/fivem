#pragma once

#include "SerializableComponent.h"
#include "SerializableOptional.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ClientRoute : public SerializableComponent
{
public:
	// TODO: in future net version remove targetNetId for OneSync route
	SerializableProperty<uint16_t> targetNetId;
	// TODO: in future net version remove packetLength
	SerializableProperty<uint16_t> unusedPacketLength;
	// TODO: in future net version limit OneSync data to 16384 bytes
	SerializableProperty<net::Span<uint8_t>, storage_type::ConstrainedStreamTail<1, UINT16_MAX/*16384*/>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			targetNetId,
			unusedPacketLength,
			data
		);
	}
};

/// <summary>
/// Request from the client to the server for routing the sync data
/// </summary>
class ClientRoutePacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ net::force_consteval<uint32_t, HashRageString("msgRoute")> };
	ClientRoute data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			type,
			data
		);
	}
};
}
