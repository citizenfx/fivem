#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
// todo: check that SerializableComponent::GetMinSize<T>() is 6
/// <summary>
/// Stream message from the client to the voice server
/// </summary>
class ClientMumbleMessage : public SerializableComponent
{
public:
	// voip-server-mumble/src/messages.h:messageType_t
	SerializableProperty<uint16_t, void, true, true> type;
	SerializableProperty<Span<uint8_t>, storage_type::ConstrainedBigBytesArray<0, 8192>, true, true> data;

	template<typename T>
	SerializableResult Process(T& stream)
	{
		return ProcessPropertiesResultInOrder<T>(
		stream,
		type,
		data
		);
	}
};
}
