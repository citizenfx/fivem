#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"

namespace net::packet
{
class ServerIHost : public SerializableComponent
{
public:
	SerializableProperty<uint16_t> netId;
	SerializableProperty<uint32_t> baseNum;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			netId,
			baseNum
		);
	}
};

/// <summary>
/// Announcement from the server to the client to notify about a new host.
/// Is never send when OneSync is enabled.
/// </summary>
class ServerIHostPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgIHost") };
	ServerIHost data;

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

class ClientIHost : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> baseNum;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			baseNum
		);
	}
};

/// <summary>
/// Request from the client to the server for becoming the new host.
/// Is never send when OneSync is enabled.
/// </summary>
class ClientIHostPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgIHost") };
	ClientIHost data;

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
