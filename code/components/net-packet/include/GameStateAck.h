#pragma once

#include "GameStateNAck.h"
#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ClientGameStateAck : public SerializableComponent
{
	SerializableProperty<uint64_t> frameIndex;
	SerializableProperty<Span<ClientGameStateNAck::IgnoreListEntry>, storage_type::SmallBytesArray> ignoreList;
	SerializableProperty<Span<uint16_t>, storage_type::SmallBytesArray> recreateList;
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder(
			stream,
			frameIndex,
			ignoreList,
			recreateList
		);
	}

	void SetFrameIndex(const uint64_t frameIndexValue)
	{
		frameIndex = frameIndexValue;
	}

	void SetIgnoreList(std::vector<ClientGameStateNAck::IgnoreListEntry>& ignoreListValue)
	{
		if (!ignoreListValue.empty())
		{
			ignoreList.SetValue({ ignoreListValue.data(), ignoreListValue.size() });
		}
	}

	void SetRecreateList(std::vector<uint16_t>& recreateListValue)
	{
		if (!recreateListValue.empty())
		{
			recreateList.SetValue({ recreateListValue.data(), recreateListValue.size() });
		}
	}

	Span<ClientGameStateNAck::IgnoreListEntry>& GetIgnoreList()
	{
		return ignoreList.GetValue();
	}

	Span<uint16_t>& GetRecreateList()
	{
		return recreateList.GetValue();
	}

	uint64_t GetFrameIndex() const
	{
		return frameIndex;
	}
};

/// <summary>
/// Information from the client to the server to acknowledge the game state packets.
/// Only used when OneSync is enabled and ARQ mode is enabled.
/// </summary>
class ClientGameStateAckPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("gameStateAck") };
	ClientGameStateAck data;

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
