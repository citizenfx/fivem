#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>
#include <iostream>

#include <StateBagComponent.h>

#include "ByteReader.h"
#include "TestUtils.h"

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

	bool HasPacket()
	{
		return lastPacket.has_value();
	}

	std::tuple<int, std::string> GetPacket()
	{
		return lastPacket.value();
	}
};

TEST_CASE("State Bag handle")
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
		fx::StateBagMessage message;
		net::ByteReader reader(reinterpret_cast<const uint8_t*>(std::get<1>(newPacket).data()), std::get<1>(newPacket).size());
		message.Process(reader);
		stateBagComponentHandleNewData->HandlePacketV2(std::get<0>(newPacket), message);
	};
}
