#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <StateBagComponent.h>

#include "ByteReader.h"
#include "TestUtils.h"
#include "state/RlMessageBuffer.h"

class TestInterface : public fx::StateBagGameInterface
{
	std::optional<std::tuple<int, std::string>> lastPacket;
public:
	void SendPacket(int peer, std::string_view data) override
	{
		lastPacket.emplace(peer, data);
	}

	void Reset()
	{
		lastPacket.reset();
	}

	bool HasPacket() const
	{
		return lastPacket.has_value();
	}

	std::tuple<int, std::string> GetPacket()
	{
		return lastPacket.value();
	}
};

TEST_CASE("State Bag handle benchmarks")
{
	std::string testKey = fx::TestUtils::asciiRandom(10);
	std::string testData = fx::TestUtils::asciiRandom(50);
	
	// create old data
	auto stateBagComponentCreateOldData = fx::StateBagComponent::Create(fx::StateBagRole::Server);
	TestInterface* testInterfaceOldData = new TestInterface();
	stateBagComponentCreateOldData->SetGameInterface(testInterfaceOldData);
	auto stateBagOldData = stateBagComponentCreateOldData->RegisterStateBag("net:1");
	stateBagOldData->AddRoutingTarget(2);
	testInterfaceOldData->Reset();
	stateBagOldData->SetKey(1, testKey, testData);
	REQUIRE(testInterfaceOldData->HasPacket());
	auto oldPacket = testInterfaceOldData->GetPacket();
	REQUIRE(std::get<0>(oldPacket) == 2);
	
	auto stateBagComponentHandleOldData = fx::StateBagComponent::Create(fx::StateBagRole::Client);

	BENCHMARK("stateBagHandler") {
		stateBagComponentHandleOldData->HandlePacket(std::get<0>(oldPacket), std::string_view(reinterpret_cast<const char*>(std::get<1>(oldPacket).data()), std::get<1>(oldPacket).size()));
	};

	// create new data
	auto stateBagComponentCreateNewData = fx::StateBagComponent::Create(fx::StateBagRole::ClientV2);
	TestInterface* testInterfaceNewData = new TestInterface();
	stateBagComponentCreateNewData->SetGameInterface(testInterfaceNewData);
	auto stateBagNewData = stateBagComponentCreateNewData->RegisterStateBag("net:1");
	stateBagNewData->AddRoutingTarget(2);
	testInterfaceNewData->Reset();
	stateBagNewData->SetKey(1, testKey, testData);
	REQUIRE(testInterfaceNewData->HasPacket());
	auto newPacket = testInterfaceNewData->GetPacket();
	REQUIRE(std::get<0>(newPacket) == 2);
	
	auto stateBagComponentHandleNewData = fx::StateBagComponent::Create(fx::StateBagRole::Server);

	BENCHMARK("stateBagHandlerV2") {
		net::packet::StateBagV2 stateBag;
		net::ByteReader reader(reinterpret_cast<const uint8_t*>(std::get<1>(newPacket).data()), std::get<1>(newPacket).size());
		stateBag.Process(reader);
		stateBagComponentHandleNewData->HandlePacketV2(std::get<0>(newPacket), stateBag);
	};
}

TEST_CASE("State Bag handler v2 test")
{
	std::string testKey = fx::TestUtils::asciiRandom(10);
	std::string testData = fx::TestUtils::asciiRandom(50);

	auto stateBagComponentCreateNewData = fx::StateBagComponent::Create(fx::StateBagRole::ClientV2);
	TestInterface* testInterfaceNewData = new TestInterface();
	stateBagComponentCreateNewData->SetGameInterface(testInterfaceNewData);
	auto stateBagNewData = stateBagComponentCreateNewData->RegisterStateBag("net:1");
	stateBagNewData->AddRoutingTarget(2);
	testInterfaceNewData->Reset();
	stateBagNewData->SetKey(1, testKey, testData);
	REQUIRE(testInterfaceNewData->HasPacket());
	auto newPacket = testInterfaceNewData->GetPacket();
	REQUIRE(std::get<0>(newPacket) == 2);

	auto stateBagComponentHandleNewData = fx::StateBagComponent::Create(fx::StateBagRole::Server);

	net::packet::StateBagV2 stateBag;
	net::ByteReader reader(reinterpret_cast<const uint8_t*>(std::get<1>(newPacket).data()), std::get<1>(newPacket).size());
	REQUIRE(stateBag.Process(reader) == true);
	REQUIRE(stateBag.key == testKey);
	REQUIRE(stateBag.data == testData);
	stateBagComponentHandleNewData->HandlePacketV2(std::get<0>(newPacket), stateBag);
}

TEST_CASE("State Bag handler v1 test")
{
	std::string testKey = fx::TestUtils::asciiRandom(10);
	std::string testData = fx::TestUtils::asciiRandom(50);

	auto stateBagComponentCreateOldData = fx::StateBagComponent::Create(fx::StateBagRole::Server);
	TestInterface* testInterfaceOldData = new TestInterface();
	stateBagComponentCreateOldData->SetGameInterface(testInterfaceOldData);
	auto stateBagOldData = stateBagComponentCreateOldData->RegisterStateBag("net:1");
	stateBagOldData->AddRoutingTarget(2);
	testInterfaceOldData->Reset();
	stateBagOldData->SetKey(1, testKey, testData);
	REQUIRE(testInterfaceOldData->HasPacket());
	auto oldPacket = testInterfaceOldData->GetPacket();
	REQUIRE(std::get<0>(oldPacket) == 2);
	
	auto stateBagComponentHandleOldData = fx::StateBagComponent::Create(fx::StateBagRole::Client);

	REQUIRE(std::get<1>(oldPacket).size() == testKey.size() + testData.size() + 11);
	rl::MessageBuffer buffer{ reinterpret_cast<const uint8_t*>(std::get<1>(oldPacket).data()), std::get<1>(oldPacket).size() };
	uint16_t idLength;
	buffer.Read<uint16_t>(16, &idLength);
	std::vector<char> idBuffer(idLength - 1);
	buffer.ReadBits(idBuffer.data(), idBuffer.size() * 8);
	buffer.Read<uint8_t>(8);

	REQUIRE(std::string_view(idBuffer.data(), idBuffer.size()) == "net:1");

	uint16_t keyLength;
	buffer.Read<uint16_t>(16, &keyLength);
	std::vector<char> keyBuffer(keyLength - 1);
	buffer.ReadBits(keyBuffer.data(), keyBuffer.size() * 8);
	buffer.Read<uint8_t>(8);

	REQUIRE(std::string_view(keyBuffer.data(), keyBuffer.size()) == testKey);

	size_t dataLength = (buffer.GetLength() * 8) - buffer.GetCurrentBit();
	std::vector<char> data((dataLength + 7) / 8);
	buffer.ReadBits(data.data(), dataLength);

	REQUIRE(std::string_view(data.data(), data.size()) == testData);

	stateBagComponentHandleOldData->HandlePacket(std::get<0>(oldPacket), std::get<1>(oldPacket));
}
