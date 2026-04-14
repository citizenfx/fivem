#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <StateBagComponent.h>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "ClientRegistry.h"
#include "ConsoleContextInstance.h"
#include "ENetPacketInstance.h"
#include "ResourceManagerInstance.h"
#include "state/ServerGameStatePublic.h"
#include "ServerGameStatePublicInstance.h"
#include "ServerInstance.h"
#include "TestUtils.h"
#include "packethandlers/StateBagPacketHandler.h"
#include "state/RlMessageBuffer.h"

class TestInterface : public fx::StateBagGameInterface
{
	std::optional<std::pair<int, std::vector<uint8_t>>> lastPacket;
public:
	void SendPacket(int peer, net::packet::StateBagPacket& packet) override
	{
		auto& val = lastPacket.emplace(peer, std::vector<uint8_t>{});
		val.second.resize(net::SerializableComponent::GetMaxSize<net::packet::StateBagPacket>());
		net::ByteWriter writer(val.second.data(), val.second.size());
		packet.Process(writer);
		val.second.resize(writer.GetOffset());
	}

	void SendPacket(int peer, net::packet::StateBagV2Packet& packet) override
	{
		auto& val = lastPacket.emplace(peer, std::vector<uint8_t>{});
		val.second.resize(net::SerializableComponent::GetMaxSize<net::packet::StateBagV2Packet>());
		net::ByteWriter writer(val.second.data(), val.second.size());
		packet.Process(writer);
		val.second.resize(writer.GetOffset());
	}

	void Reset()
	{
		lastPacket.reset();
	}

	bool HasPacket() const
	{
		return lastPacket.has_value();
	}

	std::pair<int, std::vector<uint8_t>> GetPacket()
	{
		return lastPacket.value();
	}
};

TEST_CASE("State Bag handle benchmarks")
{
	std::string testKey = fx::TestUtils::asciiRandom(10);
	std::string testData = fx::TestUtils::asciiRandom(50);

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
		net::packet::StateBagV2Packet stateBag;
		net::ByteReader reader(std::get<1>(newPacket).data(), std::get<1>(newPacket).size());
		stateBag.Process(reader);
		stateBagComponentHandleNewData->HandlePacketV2(std::get<0>(newPacket), stateBag.data);
	};
}

TEST_CASE("State Bag v2 test")
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

	net::packet::StateBagV2Packet stateBag;
	net::ByteReader reader(std::get<1>(newPacket).data(), std::get<1>(newPacket).size());
	REQUIRE(stateBag.Process(reader) == true);
	REQUIRE(stateBag.type == HashRageString("msgStateBagV2"));
	REQUIRE(stateBag.data.key == testKey);
	REQUIRE(stateBag.data.data == testData);
	stateBagComponentHandleNewData->HandlePacketV2(std::get<0>(newPacket), stateBag.data);
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

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());

	auto stateBagComponentCreateOldDataServer = fx::StateBagComponent::Create(fx::StateBagRole::Server);
	auto resourceManager = fx::ResourceManagerInstance::Create();
	serverInstance->SetComponent<fx::ResourceManager>(resourceManager);
	resourceManager->SetComponent<fx::StateBagComponent>(stateBagComponentCreateOldDataServer);
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	fwRefContainer<fx::ServerGameStatePublic> serverGameState = fx::ServerGameStatePublicInstance::Create();
	serverInstance->SetComponent(serverGameState);

	testInterfaceNewData->Reset();

	auto stateBag = stateBagComponentCreateOldDataServer->RegisterStateBag("net:1");
	
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	client->SetSlotId(1);
	StateBagPacketHandlerV2 handler(serverInstance.GetRef());
	net::Buffer buffer;
	buffer.Write(std::get<1>(newPacket).data(), std::get<1>(newPacket).size());
	buffer.Reset();
	REQUIRE(buffer.Read<uint32_t>() == HashRageString("msgStateBagV2"));
	net::ByteReader handlerReader(buffer.GetBuffer(), buffer.GetLength());
	uint32_t packetType;
	handlerReader.Field(packetType);
	REQUIRE(packetType == HashRageString("msgStateBagV2"));
	fx::ENetPacketPtr packetPtr = fx::ENetPacketInstance::Create(buffer.GetBuffer(), buffer.GetLength());
	handler.Process(serverInstance.GetRef(), client, handlerReader, packetPtr);

	REQUIRE(stateBag->HasKey(testKey) == true);
	REQUIRE(stateBag->GetKey(testKey) == testData);
}
