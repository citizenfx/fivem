#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ServerRpcNative : public SerializableComponent
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
/// Rpc native send from the server to the client
/// </summary>
class ServerRpcNativePacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ net::force_consteval<uint32_t, HashRageString("msgRpcNative")> };
	ServerRpcNative event;

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
