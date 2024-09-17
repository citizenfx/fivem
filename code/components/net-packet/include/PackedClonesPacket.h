#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ServerPackedClones : public SerializableComponent
{
public:
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedStreamTail<1, UINT32_MAX>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
		stream,
		data
		);
	}
};

/// <summary>
/// PackedClones OneSync packet from server to the client
/// </summary>
class ServerPackedClonesPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgPackedClones") };
	ServerPackedClones event;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
		stream,
		type,
		event
		);
	}
};
}
