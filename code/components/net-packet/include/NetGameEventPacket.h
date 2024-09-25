#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ClientNetGameEvent : public SerializableComponent
{
public:
	SerializableProperty<Span<uint16_t>, storage_type::SmallBytesArray> targetPlayers;
	SerializableProperty<uint16_t> eventId;
	SerializableProperty<uint8_t> isReply;
	SerializableProperty<uint16_t> eventType;
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedBytesArray<0, 1025>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
		stream,
		targetPlayers,
		eventId,
		isReply,
		eventType,
		data
		);
	}
};

/// <summary>
/// Informs the server about a net game event happened on the client.
/// This packet is sent from the client to the server.
/// </summary>
class ClientNetGameEventPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgNetGameEvent") };
	ClientNetGameEvent event;

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

class ClientNetGameEventV2 : public SerializableComponent
{
public:
	SerializableProperty<Span<uint16_t>, storage_type::SmallBytesArray> targetPlayers;
	SerializableProperty<uint32_t> eventNameHash;
	SerializableProperty<uint16_t> eventId;
	SerializableProperty<bool> isReply{ false };
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedStreamTail<0, 1025>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
		stream,
		targetPlayers,
		eventNameHash,
		eventId,
		isReply,
		data
		);
	}
};

/// <summary>
/// ClientNetGameEventV2 is the second version of the "netGameEvent" packet. It is used in the "netGameEventV2" endpoint.
/// This packet is send from the client to the server.
/// This packet provides backward compatibility for TU's by using the eventNameHash instead of the eventType.
/// Because the eventType is based on the event index which can change between TU's.
/// The other change is the data length. Before it was stored inside a uint16_t, now it is using the remaining bytes of the packet. This saves another 16 bit.
/// </summary>
class ClientNetGameEventV2Packet : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgNetGameEventV2") };
	ClientNetGameEventV2 event;

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

/// <summary>
/// ServerNetGameEventV2 is the packet that is send from the server to the client.
/// This was previously implemented inline inside the handler.
/// </summary>
class ServerNetGameEventV2 : public SerializableComponent
{
public:
	SerializableProperty<uint16_t> clientNetId;
	SerializableProperty<uint32_t> eventNameHash;
	SerializableProperty<uint16_t> eventId;
	SerializableProperty<bool> isReply{ false };
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedStreamTail<0, 1025>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
		stream,
		clientNetId,
		eventNameHash,
		eventId,
		isReply,
		data
		);
	}
};

class ServerNetGameEventV2Packet : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgNetGameEventV2") };
	ServerNetGameEventV2 event;

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

/// <summary>
/// ServerNetGameEvent is the packet that is send from the server to the client.
/// </summary>
class ServerNetGameEvent : public SerializableComponent
{
public:
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedStreamTail<0, 2048>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
		stream,
		data
		);
	}
};

class ServerNetGameEventPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgNetGameEvent") };
	ServerNetGameEvent event;

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
