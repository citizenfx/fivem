#include <StdInc.h>

#include "ArrayUpdate.h"
#include "ByteReader.h"
#include "ByteWriter.h"
#include "catch_amalgamated.hpp"
#include "NetBuffer.h"
#include "SerializableComponent.h"
#include "TestUtils.h"

TEST_CASE("array update tests")
{
	std::vector<uint8_t> serverEventData(net::SerializableComponent::GetSize<net::packet::ServerArrayUpdatePacket>());
	net::ByteWriter serverEventDataWriter(serverEventData.data(), serverEventData.size());

	net::packet::ServerArrayUpdatePacket serverArrayUpdatePacket;
	serverArrayUpdatePacket.data.handler = fx::TestUtils::u64Random(UINT8_MAX);
	serverArrayUpdatePacket.data.index = fx::TestUtils::u64Random(UINT16_MAX);
	serverArrayUpdatePacket.data.ownerNetId = fx::TestUtils::u64Random(UINT16_MAX);

	// 128 is the max size of the array sync data
	std::vector<uint8_t> arrayElementData(128);
	fx::TestUtils::fillVectorU8Random(arrayElementData);

	serverArrayUpdatePacket.data.data.SetValue({ arrayElementData.data(), arrayElementData.size() });

	REQUIRE(serverArrayUpdatePacket.Process(serverEventDataWriter) == true);
	
	static thread_local std::vector<uint8_t> data(1024);
	memset(data.data(), 0, data.size());
	static std::vector<size_t> hashes(UINT16_MAX + 1);

	static std::vector<uint8_t> sendBuffer(net::SerializableComponent::GetSize<net::packet::ClientArrayUpdatePacket>());

	BENCHMARK("old array update deserialization")
	{
		net::Buffer buf(reinterpret_cast<const uint8_t*>(serverEventData.data()), serverEventData.size());
		buf.Seek(serverEventDataWriter.GetOffset());
		
		auto arrayIndex = buf.Read<uint8_t>();
		auto player = buf.Read<uint16_t>();
		auto element = buf.Read<uint32_t>();
		auto length = buf.Read<uint32_t>();

		std::vector<uint8_t> bufData(length);
		buf.Read(bufData.data(), bufData.size());

		{
			// write element
			std::string sv{ reinterpret_cast<char*>(arrayElementData.data()), arrayElementData.size() };
			auto thisHash = std::hash<std::string>()(sv);
			hashes[element] = thisHash;
		}
	};
	
	BENCHMARK("new array update deserialization")
	{
		net::packet::ServerArrayUpdatePacket readServerArrayUpdatePacket;
		net::ByteReader serverEventDataReader(serverEventData.data(), serverEventData.size());
		readServerArrayUpdatePacket.Process(serverEventDataReader);

		uint8_t arrayIndex = readServerArrayUpdatePacket.data.handler;
		uint16_t player = readServerArrayUpdatePacket.data.ownerNetId;
		uint32_t element = readServerArrayUpdatePacket.data.index;
		size_t length = readServerArrayUpdatePacket.data.data.GetValue().size();

		{
			// write element
			std::string_view sv{ reinterpret_cast<char*>(arrayElementData.data()), arrayElementData.size() };
			auto thisHash = std::hash<std::string_view>()(sv);
			hashes[element] = thisHash;
		}
	};

	BENCHMARK("old array update serialization")
	{
		static net::Buffer outBuffer;
		std::string sv{ reinterpret_cast<char*>(arrayElementData.data()), arrayElementData.size() };
		auto thisHash = std::hash<std::string>()(sv);

		// write the array update
		outBuffer.Reset();

		outBuffer.Write<uint8_t>(serverArrayUpdatePacket.data.handler);
		outBuffer.Write<uint16_t>(serverArrayUpdatePacket.data.index);
		outBuffer.Write<uint16_t>(sv.size());
		outBuffer.Write(sv.data(), sv.size());

		{
			net::Buffer msg;
			msg.Write(HashRageString("msgArrayUpdate"));
			msg.Write((const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());
		}

		hashes[serverArrayUpdatePacket.data.index] = thisHash;
	};

	BENCHMARK("new array update serialization")
	{
		std::string_view sv = { reinterpret_cast<char*>(arrayElementData.data()), arrayElementData.size() };
		auto thisHash = std::hash<std::string_view>()(sv);
		net::ByteWriter writer(sendBuffer.data(), sendBuffer.size());
		net::packet::ClientArrayUpdatePacket clientArrayUpdatePacket;
		clientArrayUpdatePacket.data.handler = serverArrayUpdatePacket.data.handler;
		clientArrayUpdatePacket.data.index = serverArrayUpdatePacket.data.index;
		clientArrayUpdatePacket.data.data = serverArrayUpdatePacket.data.data;
		clientArrayUpdatePacket.Process(writer);
		hashes[serverArrayUpdatePacket.data.index] = thisHash;
	};
}
