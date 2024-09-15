#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"

namespace net::packet
{
class ServerFrame : public SerializableComponent
{
public:
	// todo: in future net version remove, its always 0
	SerializableProperty<uint32_t> idx{0};
	SerializableProperty<uint8_t> lockdownMode;
	SerializableProperty<uint8_t> syncStyle;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			idx,
			lockdownMode,
			syncStyle
		);
	}
};

/// <summary>
/// Send from the server to the client when the strict mode or sync style changes
/// </summary>
class ServerFramePacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgFrame") };
	ServerFrame data;

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
