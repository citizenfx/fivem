#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <StateBagComponent.h>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "ClientRegistry.h"
#include "ConsoleContextInstance.h"
#include "ResourceManagerInstance.h"
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
		val.second.resize(net::SerializableComponent::GetSize<net::packet::StateBagPacket>());
		net::ByteWriter writer(val.second.data(), val.second.size());
		packet.Process(writer);
		val.second.resize(writer.GetOffset());
	}

	void SendPacket(int peer, net::packet::StateBagV2Packet& packet) override
	{
		auto& val = lastPacket.emplace(peer, std::vector<uint8_t>{});
		val.second.resize(net::SerializableComponent::GetSize<net::packet::StateBagV2Packet>());
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
		// first 4 bytes are the packet type hash
		stateBagComponentHandleOldData->HandlePacket(std::get<0>(oldPacket), std::string_view(reinterpret_cast<const char*>(std::get<1>(oldPacket).data() + 4), std::get<1>(oldPacket).size() - 4));
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

TEST_CASE("State Bag v1 test")
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

	REQUIRE(std::get<1>(oldPacket).size() == testKey.size() + testData.size() + 11 + 4/*packet type*/);
	rl::MessageBuffer buffer{ reinterpret_cast<const uint8_t*>(std::get<1>(oldPacket).data() + 4), std::get<1>(oldPacket).size() - 4 };
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

	stateBagComponentHandleOldData->HandlePacket(std::get<0>(oldPacket), {reinterpret_cast<const char*>(oldPacket.second.data() + 4), oldPacket.second.size() - 4});
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

	testInterfaceNewData->Reset();

	auto stateBag = stateBagComponentCreateOldDataServer->RegisterStateBag("net:1");
	
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	client->SetSlotId(1);
	StateBagPacketHandlerV2 handler(serverInstance.GetRef());
	net::Buffer buffer;
	buffer.Write(std::get<1>(newPacket).data(), std::get<1>(newPacket).size());
	buffer.Reset();
	REQUIRE(buffer.Read<uint32_t>() == HashRageString("msgStateBagV2"));
	handler.Handle(serverInstance.GetRef(), client, buffer);

	REQUIRE(stateBag->HasKey(testKey) == true);
	REQUIRE(stateBag->GetKey(testKey) == testData);
}

TEST_CASE("State Bag handler v1 test")
{
	std::string testKey = fx::TestUtils::asciiRandom(10);
	std::string testData = fx::TestUtils::asciiRandom(50);

	auto stateBagComponentCreateOldData = fx::StateBagComponent::Create(fx::StateBagRole::Client);
	TestInterface* testInterfaceOldData = new TestInterface();
	stateBagComponentCreateOldData->SetGameInterface(testInterfaceOldData);
	auto stateBagOldData = stateBagComponentCreateOldData->RegisterStateBag("net:1");
	stateBagOldData->AddRoutingTarget(2);
	testInterfaceOldData->Reset();
	stateBagOldData->SetKey(1, testKey, testData);
	REQUIRE(testInterfaceOldData->HasPacket());
	auto oldPacket = testInterfaceOldData->GetPacket();
	REQUIRE(std::get<0>(oldPacket) == 2);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());

	auto stateBagComponentCreateOldDataServer = fx::StateBagComponent::Create(fx::StateBagRole::Server);
	auto resourceManager = fx::ResourceManagerInstance::Create();
	serverInstance->SetComponent<fx::ResourceManager>(resourceManager);
	resourceManager->SetComponent<fx::StateBagComponent>(stateBagComponentCreateOldDataServer);
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());

	testInterfaceOldData->Reset();

	auto stateBag = stateBagComponentCreateOldDataServer->RegisterStateBag("net:1");
	
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	client->SetSlotId(1);
	StateBagPacketHandler handler(serverInstance.GetRef());
	net::Buffer buffer;
	buffer.Write(std::get<1>(oldPacket).data(), std::get<1>(oldPacket).size());
	buffer.Reset();
	REQUIRE(buffer.Read<uint32_t>() == HashRageString("msgStateBag"));
	handler.Handle(serverInstance.GetRef(), client, buffer);

	REQUIRE(stateBag->HasKey(testKey) == true);
	REQUIRE(stateBag->GetKey(testKey) == testData);
}
