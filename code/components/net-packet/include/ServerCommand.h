#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ClientServerCommand : public SerializableComponent
{
public:
	// the command string
	SerializableProperty<std::string_view, storage_type::ConstrainedBytesArray<1, 4096>> command;
	// the command string hash, unused can be removed in a future net version
	SerializableProperty<uint32_t> commandHash;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
		stream,
		command,
		commandHash
		);
	}
};

/// <summary>
/// msgServerCommand is a packet that is sent to the server to execute a command.
/// This packet is sent from the client to the server.
/// </summary>
class ClientServerCommandPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ net::force_consteval<uint32_t, HashRageString("msgServerCommand")> };
	ClientServerCommand data;

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
