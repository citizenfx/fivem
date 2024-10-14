#pragma once

#include "SerializableComponent.h"
#include "SerializableOptional.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ClientHeHost : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> allegedNewId;
	SerializableProperty<uint32_t> baseNum;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			allegedNewId,
			baseNum
		);
	}
};

/// <summary>
/// Request from the client to the server for voting for a specific client to become the new host.
/// Is never send when OneSync is enabled.
/// </summary>
class ClientHeHostPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ net::force_consteval<uint32_t, HashRageString("msgHeHost")> };
	ClientHeHost data;

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
