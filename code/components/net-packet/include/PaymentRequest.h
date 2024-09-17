#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ServerPaymentRequest : public SerializableComponent
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
/// Send from the server to the client to trigger the payment request.
/// </summary>
class ServerPaymentRequestPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgPaymentRequest") };
	ServerPaymentRequest event;

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
