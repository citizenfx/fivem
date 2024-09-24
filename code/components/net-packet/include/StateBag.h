#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class StateBag : public SerializableComponent
{
public:
	net::SerializableProperty<std::string_view, net::storage_type::ConstrainedStreamTail<1, 131072>> data;
	
	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			data
		);
	}
};

/// <summary>
/// Request from the client<->server for modifying a state bag.
/// </summary>
class StateBagPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgStateBag") };
	StateBag data;

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

class StateBagV2 : public SerializableComponent
{
public:
	net::SerializableProperty<std::string_view, net::storage_type::ConstrainedBytesArray<1, 128>> stateBagName;
	net::SerializableProperty<std::string_view, net::storage_type::ConstrainedBytesArray<1, 512>> key;
	net::SerializableProperty<std::string_view, net::storage_type::ConstrainedStreamTail<1, 4096>> data;

	StateBagV2() = default;

	StateBagV2(std::string_view stateBagName, std::string_view key, std::string_view data)
		: stateBagName(stateBagName), key(key), data(data)
	{
	}
	
	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			stateBagName,
			key,
			data
		);
	}
};

/// <summary>
/// Request from the client<->server for modifying a state bag.
/// </summary>
class StateBagV2Packet : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgStateBagV2") };
	StateBagV2 data;

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
