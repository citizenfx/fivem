#pragma once

#include "SerializableComponent.h"
#include "SerializableOptional.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
/// <summary>
/// Request from the server to the client to start the resource
/// </summary>
class ServerResourceStart : public SerializableComponent
{
public:
	SerializableProperty<std::string_view, storage_type::StreamTail> resourceName;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			resourceName
		);
	}
};

class ServerResourceStartPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgResStart") };
	ServerResourceStart data;

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

/// <summary>
/// Request from the server to the client to stop the resource
/// </summary>
class ServerResourceStop : public SerializableComponent
{
public:
	SerializableProperty<std::string_view, storage_type::StreamTail> resourceName;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			resourceName
		);
	}
};

class ServerResourceStopPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgResStop") };
	ServerResourceStop data;

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
