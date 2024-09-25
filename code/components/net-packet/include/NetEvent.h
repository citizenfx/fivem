#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
// todo: in a future net version, reuse class between server and client because, because it will be the same
/// <summary>
/// Request from the server to the client to trigger an event
/// </summary>
class ServerClientEvent : public SerializableComponent
{
public:
	// todo: remove in a future net version, because its always -1 from the server to the client
	SerializableProperty<uint16_t> sourceNetId{ static_cast<uint16_t>(-1) };
	// todo: convert event name to a std::string_view in future net version after removing null terminator
	SerializableProperty<net::Span<uint8_t>, storage_type::ConstrainedBytesArray<2, UINT16_MAX>> eventName;
	SerializableProperty<net::Span<uint8_t>, storage_type::StreamTail> eventData;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			sourceNetId,
			eventName,
			eventData
		);
	}
};

class ServerClientEventPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgNetEvent") };
	ServerClientEvent data;

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

class ClientServerEvent : public SerializableComponent
{
public:
	// todo: convert event name to a std::string_view in future net version after removing null terminator
	SerializableProperty<net::Span<uint8_t>, storage_type::ConstrainedBytesArray<2/*2, because null terminator*/, UINT16_MAX>> eventName;
	SerializableProperty<net::Span<uint8_t>, storage_type::ConstrainedStreamTail<0, 384 * 1024>> eventData;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			eventName,
			eventData
		);
	}
};

/// <summary>
/// Request from the client to the server to trigger an event
/// </summary>
class ClientServerEventPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgServerEvent") };
	ClientServerEvent data;

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
