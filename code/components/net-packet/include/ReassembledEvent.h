#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ReassembledEvent : public SerializableComponent
{
public:
	SerializableProperty<net::Span<uint8_t>, net::storage_type::ConstrainedStreamTail<1, 1088>> data;

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
/// Request from the client<->server for transmitting an event in fragments.
/// </summary>
class ReassembledEventPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ net::force_consteval<uint32_t, HashRageString("msgReassembledEvent")> };
	ReassembledEvent data;

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
