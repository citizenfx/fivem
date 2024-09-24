#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ServerArrayUpdate : public SerializableComponent
{
public:
	SerializableProperty<uint8_t> handler;
	SerializableProperty<uint16_t> ownerNetId;
	// todo: in future net version change to uint16_t like packet from client to server
	SerializableProperty<uint32_t> index;
	// todo: in future net version use ConstrainedStreamTail
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedBigBytesArray<0, 128>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			handler,
			ownerNetId,
			index,
			data
		);
	}
};

/// <summary>
/// Announcement from the server to the client for updating the array handler
/// </summary>
class ServerArrayUpdatePacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgArrayUpdate") };
	ServerArrayUpdate data;

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

class ClientArrayUpdate : public SerializableComponent
{
public:
	SerializableProperty<uint8_t> handler;
	SerializableProperty<uint16_t> index;
	// todo: in future net version use ConstrainedStreamTail
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedBytesArray<0, 128>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			handler,
			index,
			data
		);
	}
};

/// <summary>
/// Request from the client to the server for updating the array handler
/// </summary>
class ClientArrayUpdatePacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgArrayUpdate") };
	ClientArrayUpdate data;

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
