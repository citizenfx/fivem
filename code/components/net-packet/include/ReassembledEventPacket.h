#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ReassembledEvent : public SerializableComponent
{
public:
	static constexpr uint32_t kPacketSizeBits = 17;
	static constexpr uint32_t kFragmentSize = 1024 - 1;
	static constexpr uint32_t kFragmentSizeBits = 10;
	static constexpr uint32_t kMaxPacketSize = (1 << kPacketSizeBits) * kFragmentSize;
	
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedStreamTail<1, 1088>> data;

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

class ReassembledEventV2 : public SerializableComponent
{
public:
	static constexpr uint32_t kPacketSizeBits = 16;
	static constexpr uint32_t kFragmentSizeBits = 16;
	static constexpr uint32_t kFragmentSize = 2048 - 1;
	static constexpr uint32_t kMaxPacketSize = (1 << kPacketSizeBits) * kFragmentSize;
	
	SerializableProperty<uint64_t> eventId;
	SerializableProperty<uint16_t> packetIdx;
	SerializableProperty<uint16_t> totalPackets;
	// size 0 is allowed, this means it is an ack packet
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedStreamTail<0, 2047>> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			eventId,
			packetIdx,
			totalPackets,
			data
		);
	}

	bool IsAck() const
	{
		return data.GetValue().empty();
	}
};

/// <summary>
/// New version of the request from the client<->server for transmitting an event in fragments.
/// </summary>
class ReassembledEventV2Packet : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ net::force_consteval<uint32_t, HashRageString("msgReassembledEventV2")> };
	ReassembledEventV2 data;

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
