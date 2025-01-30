#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <StateBagComponent.h>

#include "ByteReader.h"
#include "ClientRegistry.h"
#include "EventReassemblyComponent.h"
#include "ResourceEventComponent.h"
#include "ResourceManagerInstance.h"
#include "ServerInstance.h"
#include "ServerInstanceBase.h"
#include "TestUtils.h"
#include "state/RlMessageBuffer.h"

class TestEventReassemblySink : public fx::EventReassemblySink
{
public:
	struct PacketQueueV2Entry
	{
		uint64_t eventId;
		uint16_t packetIdx;
		uint16_t totalPackets;
		std::vector<uint8_t> data;
	};

	std::vector<std::pair<int, std::string>> packetQueue;
	std::vector<std::pair<int, PacketQueueV2Entry>> packetQueueV2;

	void SendPacket(int target, std::string_view data) override
	{
		packetQueue.emplace_back(target, data);
	}

	void SendPacketV2(int target, net::packet::ReassembledEventV2Packet& packet) override
	{
		packetQueueV2.emplace_back(target, PacketQueueV2Entry{
			                           packet.data.eventId, packet.data.packetIdx, packet.data.totalPackets,
			                           {packet.data.data.GetValue().begin(), packet.data.data.GetValue().end()}
		                           });
	}

	bool LimitEvent(int source) override
	{
		// only used for rate limiting, can be always false for tests
		return false;
	}
};

TEST_CASE("Reassembly client to server event test")
{
	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
	serverInstance->SetComponent(new fx::ClientRegistry());

	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	client->SetNetId(1);

	auto reassemblyComponentServer = fx::EventReassemblyComponent::Create();
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::EventReassemblyComponent>(reassemblyComponentServer);
	
	std::string testEventName = fx::TestUtils::asciiRandom(10);
	std::string testEventData = fx::TestUtils::asciiRandom(50);
	
	TestEventReassemblySink reassemblySinkServer;
	reassemblyComponentServer->SetSink(&reassemblySinkServer);
	// register client net id 1 target
	reassemblyComponentServer->RegisterTarget(client->GetNetId(), 0xFF);

	auto reassemblyComponentClient = fx::EventReassemblyComponent::Create();
	TestEventReassemblySink reassemblySinkClient;
	reassemblyComponentClient->SetSink(&reassemblySinkClient);
	// register server target
	reassemblyComponentClient->RegisterTarget(0, 0xFF);

	int serverTarget = 0;
	int bytesPerSecond = 50000;
	reassemblyComponentClient->TriggerEvent(serverTarget, std::string_view{ testEventName.c_str(), testEventName.size() + 1 }, testEventData, bytesPerSecond);

	// should be empty when network tick does not run
	REQUIRE(reassemblySinkClient.packetQueue.empty() == true);

	reassemblyComponentClient->NetworkTick();

	REQUIRE(reassemblySinkClient.packetQueue.size() == 1);

	REQUIRE(std::get<0>(reassemblySinkClient.packetQueue[0]) == 0);

	std::string packet = std::get<1>(reassemblySinkClient.packetQueue[0]);

	std::string lastEventName;
	std::string lastEventData;
	std::string lastEventSource;
	bool event = false;
	serverInstance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>()->
						OnQueueEvent.Connect([&event, &lastEventName, &lastEventData, &lastEventSource](const std::string& eventName, const std::string& eventData,
												const std::string& eventSource)
						{
							event = true;
							lastEventName = eventName;
							lastEventData = eventData;
							lastEventSource = eventSource;
						});

	reassemblyComponentServer->HandlePacket(client->GetNetId(), packet);

	REQUIRE(event == true);
	REQUIRE(lastEventName == testEventName);
	REQUIRE(lastEventData == lastEventData);
	REQUIRE(lastEventSource == "net:" + std::to_string(client->GetNetId()));
}

TEST_CASE("Reassembly client packet v2 to server event test")
{
	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
	serverInstance->SetComponent(new fx::ClientRegistry());

	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	client->SetNetId(1);

	std::string testEventName = fx::TestUtils::asciiRandom(10);
	std::string testEventData = fx::TestUtils::asciiRandom(50);

	auto reassemblyComponentServer = fx::EventReassemblyComponent::Create();
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::EventReassemblyComponent>(reassemblyComponentServer);
	TestEventReassemblySink reassemblySinkServer;
	reassemblyComponentServer->SetSink(&reassemblySinkServer);
	// register client net id 1 target
	reassemblyComponentServer->RegisterTarget(client->GetNetId(), 0xFF);

	auto reassemblyComponentClient = fx::EventReassemblyComponent::Create();
	TestEventReassemblySink reassemblySinkClient;
	reassemblyComponentClient->SetSink(&reassemblySinkClient);
	// register server target
	reassemblyComponentClient->RegisterTarget(0, 0xFF);

	int serverTarget = 0;
	int bytesPerSecond = 50000;
	reassemblyComponentClient->TriggerEventV2(serverTarget, testEventName, testEventData, bytesPerSecond);

	uint32_t ticks = 5000;
	while (reassemblySinkClient.packetQueueV2.empty() && --ticks)
	{
		reassemblyComponentClient->NetworkTick();
	}

	REQUIRE(reassemblySinkClient.packetQueueV2.size() == 1);

	REQUIRE(std::get<0>(reassemblySinkClient.packetQueueV2[0]) == 0);

	auto packet = std::get<1>(reassemblySinkClient.packetQueueV2[0]);

	REQUIRE(packet.data.size() == testEventData.size() + testEventName.size() + 2);

	net::ByteReader reader {packet.data.data(), packet.data.size()};
	uint16_t nameLength;
	REQUIRE(reader.Field(nameLength) == true);
	std::string_view name;
	REQUIRE(reader.Field(name, nameLength) == true);
	std::string_view data;
	REQUIRE(reader.Field(data, reader.GetRemaining()) == true);

	REQUIRE(nameLength == testEventName.size());
	REQUIRE(name == testEventName);
	REQUIRE(data == testEventData);

	net::packet::ReassembledEventV2 reassembledEvent {};
	reassembledEvent.eventId = packet.eventId;
	reassembledEvent.packetIdx = packet.packetIdx;
	reassembledEvent.totalPackets = packet.totalPackets;
	reassembledEvent.data.SetValue({packet.data.data(), packet.data.size()});

	std::string lastEventName;
	std::string lastEventData;
	std::string lastEventSource;
	bool event = false;
	serverInstance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>()->
						OnQueueEvent.Connect([&event, &lastEventName, &lastEventData, &lastEventSource](const std::string& eventName, const std::string& eventData,
												const std::string& eventSource)
						{
							event = true;
							lastEventName = eventName;
							lastEventData = eventData;
							lastEventSource = eventSource;
						});
	
	reassemblyComponentServer->HandlePacketV2(client->GetNetId(), reassembledEvent);

	REQUIRE(event == true);
	REQUIRE(lastEventName == testEventName);
	REQUIRE(lastEventData == lastEventData);
	REQUIRE(lastEventSource == "net:" + std::to_string(client->GetNetId()));
}
