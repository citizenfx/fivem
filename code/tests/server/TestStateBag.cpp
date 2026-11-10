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

struct SentKey
{
	int peer;
	std::string key;
	std::string value;

	bool operator==(const SentKey& other) const
	{
		return peer == other.peer && key == other.key && value == other.value;
	}
};

// records every key a server state bag sends, and to which peer
class RecordingInterface : public fx::StateBagGameInterface
{
public:
	std::vector<SentKey> sent;

	void SendPacket(int peer, net::packet::StateBagPacket& packet) override
	{
		const std::string_view data = packet.data.data.GetValue();
		rl::MessageBuffer buffer{ reinterpret_cast<const uint8_t*>(data.data()), data.size() };

		auto readStr = [&buffer]()
		{
			uint16_t length = 0;
			buffer.Read<uint16_t>(16, &length);

			std::string str(length - 1, '\0');
			buffer.ReadBits(str.data(), str.size() * 8);
			buffer.Read<uint8_t>(8);

			return str;
		};

		readStr(); // bag name
		auto key = readStr();

		std::string value(data.size() - (buffer.GetCurrentBit() / 8), '\0');
		buffer.ReadBits(value.data(), value.size() * 8);

		sent.push_back({ peer, std::move(key), std::move(value) });
	}

	void SendPacket(int peer, net::packet::StateBagV2Packet& packet) override
	{
	}
};

TEST_CASE("State Bag private keys")
{
	const std::string nil{ "\xc0" };

	auto stateBagComponent = fx::StateBagComponent::Create(fx::StateBagRole::Server);
	RecordingInterface testInterface;
	stateBagComponent->SetGameInterface(&testInterface);

	// peer 1 owns the state bag, peer 2 only has it in scope
	auto stateBag = stateBagComponent->RegisterStateBag("player:1");
	stateBag->SetOwningPeer(1);
	stateBag->AddRoutingTarget(1);
	stateBag->AddRoutingTarget(2);

	stateBag->SetKeyPrivate("secret", true);

	SECTION("a private key is only sent to the owning peer")
	{
		stateBag->SetKey(0, "secret", "one");
		stateBag->SetKey(0, "public", "two");

		REQUIRE(testInterface.sent == std::vector<SentKey>{ { 1, "secret", "one" }, { 1, "public", "two" }, { 2, "public", "two" } });
	}

	SECTION("a write by the owning peer is not relayed to other peers")
	{
		stateBag->SetKey(1, "secret", "one");

		REQUIRE(testInterface.sent == std::vector<SentKey>{ { 1, "secret", "one" } });
	}

	SECTION("a new routing target does not receive private keys")
	{
		stateBag->SetKey(0, "secret", "one");
		stateBag->SetKey(0, "public", "two");
		testInterface.sent.clear();

		stateBag->AddRoutingTarget(3);

		REQUIRE(testInterface.sent == std::vector<SentKey>{ { 3, "public", "two" } });
	}

	SECTION("private keys follow the owning peer")
	{
		stateBag->SetKey(0, "secret", "one");
		testInterface.sent.clear();

		stateBag->SetOwningPeer(2);

		REQUIRE(testInterface.sent == std::vector<SentKey>{ { 1, "secret", nil }, { 2, "secret", "one" } });
	}

	SECTION("changing the mode of a key with a value hides or reveals it")
	{
		stateBag->SetKey(0, "public", "two");
		testInterface.sent.clear();

		stateBag->SetKeyPrivate("public", true);
		REQUIRE(testInterface.sent == std::vector<SentKey>{ { 2, "public", nil } });

		testInterface.sent.clear();

		stateBag->SetKeyPrivate("public", false);
		REQUIRE(testInterface.sent == std::vector<SentKey>{ { 2, "public", "two" } });
	}
}
