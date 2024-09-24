#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ServerWorldGrid : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> base;
	// todo: in future net version use stream tail
	SerializableProperty<Span<uint8_t>, storage_type::BigBytesArray> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			base,
			data
		);
	}
};

/// <summary>
/// Announcement from the server to the client about an update to the world grid
/// </summary>
class ServerWorldGridPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgWorldGrid3") };
	ServerWorldGrid data;

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
