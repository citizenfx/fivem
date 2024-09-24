#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"

namespace net::packet
{
class ServerConVars : public SerializableComponent
{
public:
	// msg pack buffer
	// todo: in future net version use custom format using SerializableComponent
	SerializableProperty<net::Span<uint8_t>, storage_type::StreamTail> data;

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
/// Send from the server to the client to tell the client about all con vars that should be synced initially.
/// </summary>
class ServerConVarsPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgConVars") };
	ServerConVars data;

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
