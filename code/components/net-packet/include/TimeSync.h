#pragma once

#include "SerializableComponent.h"
#include "SerializableOptional.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
/// <summary>
/// Request from the client to the server for requesting the server time
/// </summary>
class TimeSyncRequest : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> requestTime;
	SerializableProperty<uint32_t> requestSequence;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			requestTime,
			requestSequence
		);
	}
};

class TimeSyncRequestPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ net::force_consteval<uint32_t, HashRageString("msgTimeSyncReq")> };
	TimeSyncRequest data;

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
/// Response from the server to the client after the client requested the server time
/// </summary>
class TimeSyncResponse : public SerializableComponent
{
public:
	TimeSyncRequest request;
	SerializableProperty<uint32_t> serverTimeMillis;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			request,
			serverTimeMillis
		);
	}
};

class TimeSyncResponsePacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ net::force_consteval<uint32_t, HashRageString("msgTimeSync")> };
	TimeSyncResponse data;

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
