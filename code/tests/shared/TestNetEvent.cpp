#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "NetBuffer.h"
#include "NetEvent.h"
#include "TestUtils.h"

TEST_CASE("net event serialization test")
{
	std::vector<uint8_t> eventData(512);
	fx::TestUtils::fillVectorU8Random(eventData);
	std::string eventName = fx::TestUtils::asciiRandom(128);


	std::vector<uint8_t> eventBuffer(net::SerializableComponent::GetSize<net::packet::ClientServerEvent>());
	net::ByteWriter writer {eventBuffer.data(), eventBuffer.size()};
	net::packet::ClientServerEvent clientServerEvent;
	clientServerEvent.eventName.SetValue({reinterpret_cast<uint8_t*>(eventName.data()), eventName.size()});
	clientServerEvent.eventData.SetValue({eventData.data(), eventData.size()});
	REQUIRE(clientServerEvent.Process(writer) == true);
	
	BENCHMARK("new net event deserialization") {
		net::ByteReader reader {eventBuffer.data(), eventBuffer.size()};
		net::packet::ClientServerEvent clientServerEventRead;
		clientServerEventRead.Process(writer);
	};

	BENCHMARK("old net event deserialization") {
		net::Buffer buffer(reinterpret_cast<const uint8_t*>(eventBuffer.data()), eventBuffer.size());

		// get the source net ID
		uint16_t sourceNetID = buffer.Read<uint16_t>();

		// get length of event name and read the event name
		static char eventName[65536];

		uint16_t nameLength = buffer.Read<uint16_t>();
		buffer.Read(eventName, nameLength);

		// read the data
		size_t dataLen = eventBuffer.size() - nameLength - (sizeof(uint16_t) * 2);
		std::vector<char> eventData(dataLen);

		buffer.Read(&eventData[0], dataLen);
	};
}
