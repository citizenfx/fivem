#include <StdInc.h>
#include <random>
#include <thread>

#include <catch_amalgamated.hpp>

#include <StateBagComponent.h>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "ClientRegistry.h"
#include "ConsoleContextInstance.h"
#include "ENetPacketInstance.h"
#include "EventReassemblyComponent.h"
#include "LatentEventAckDiagnostics.h"
#include "GameServer.h"
#include "GameServerInstance.h"
#include "ResourceEventComponent.h"
#include "ResourceManagerInstance.h"
#include "ServerInstance.h"
#include "ServerInstanceBase.h"
#include "TestUtils.h"
#include "packethandlers/ReassembledEventPacketHandler.h"
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

TEST_CASE("Missing V2 ACK causes a retry and an accepted ACK stops it")
{
	auto reassemblyComponent = fx::EventReassemblyComponent::Create();
	TestEventReassemblySink sink;
	reassemblyComponent->SetSink(&sink);
	reassemblyComponent->RegisterTarget(1, 0xFF);
	reassemblyComponent->TriggerEventV2(1, "synthetic", "payload", 50000);

	net::packet::ReassembledEventV2 prematureAck;
	prematureAck.eventId = 0;
	prematureAck.packetIdx = 0;
	prematureAck.totalPackets = 1;
	REQUIRE(reassemblyComponent->HandlePacketV2Ack(1, prematureAck) == fx::EventReassemblyAckResult::Invalid);

	reassemblyComponent->NetworkTick();

	REQUIRE(sink.packetQueueV2.size() == 1);
	const auto sent = sink.packetQueueV2.front().second;

	std::this_thread::sleep_for(std::chrono::milliseconds(275));
	reassemblyComponent->NetworkTick();
	REQUIRE(sink.packetQueueV2.size() == 2);
	REQUIRE(sink.packetQueueV2.back().second.eventId == sent.eventId);
	REQUIRE(sink.packetQueueV2.back().second.packetIdx == sent.packetIdx);

	net::packet::ReassembledEventV2 ack;
	ack.eventId = sent.eventId;
	ack.packetIdx = sent.packetIdx;
	ack.totalPackets = sent.totalPackets;
	REQUIRE(reassemblyComponent->HandlePacketV2Ack(1, ack) == fx::EventReassemblyAckResult::Accepted);
	REQUIRE(reassemblyComponent->HandlePacketV2Ack(1, ack) == fx::EventReassemblyAckResult::Duplicate);

	std::this_thread::sleep_for(std::chrono::milliseconds(275));
	reassemblyComponent->NetworkTick();
	REQUIRE(sink.packetQueueV2.size() == 2);
}

TEST_CASE("V2 ACK diagnostics classify failures without changing acceptance", "[latent-ack]")
{
	const bool enabled = GENERATE(false, true);
	CAPTURE(enabled);
	auto component = fx::EventReassemblyComponent::Create();
	TestEventReassemblySink sink;
	component->SetSink(&sink);
	component->SetAckDiagnosticsEnabled(enabled);
	component->RegisterTarget(1, 0xFF);
	component->TriggerEventV2(1, "debug\nevent", "payload", 50000);

	fx::EventReassemblyAckInfo info;
	net::packet::ReassembledEventV2 ack;
	ack.eventId = 0;
	ack.packetIdx = 0;
	ack.totalPackets = 0;
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::InvalidEnvelope);
	ack.totalPackets = 1;
	ack.packetIdx = 1;
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::InvalidEnvelope);
	ack.packetIdx = 0;
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::UnsentFragment);
	REQUIRE(info.activeOutboundEvents == 1);
	REQUIRE(info.hasTransfer == enabled);
	if (enabled)
	{
		REQUIRE(std::string(info.eventName.data()) == "debug?event");
		REQUIRE(info.payloadBytes == 20);
		REQUIRE(info.expectedFragments == 1);
		REQUIRE(info.pendingTargets == 1);
		REQUIRE(info.bytesPerSecond == 50000);
		REQUIRE(info.observedSends == 0);
	}
	ack.totalPackets = 2;
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::FragmentCountMismatch);
	ack.totalPackets = 1;
	REQUIRE(component->HandlePacketV2Ack(2, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::UnknownTarget);
	ack.eventId = 1234;
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::UnknownEvent);
	ack.eventId = 0;

	component->NetworkTick();
	std::this_thread::sleep_for(std::chrono::milliseconds(275));
	component->NetworkTick();
	REQUIRE(sink.packetQueueV2.size() == 2);
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Accepted);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::Accepted);
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Duplicate);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::Duplicate);
	if (enabled)
	{
		REQUIRE(info.observedSends == 2);
		REQUIRE(info.observedRetries == 1);
		REQUIRE(info.acknowledgedFragments == 1);
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(275));
	component->NetworkTick();
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == (enabled ? fx::EventReassemblyAckReason::RetiredCompleted : fx::EventReassemblyAckReason::UnknownEvent));
	REQUIRE(info.activeOutboundEvents == 0);
	component->SetAckDiagnosticsEnabled(false);
	component->SetAckDiagnosticsEnabled(true);
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::UnknownEvent);
}

TEST_CASE("V2 ACK diagnostics retain disconnected transfer metadata and reset send observations", "[latent-ack]")
{
	auto component = fx::EventReassemblyComponent::Create();
	TestEventReassemblySink sink;
	component->SetSink(&sink);
	component->SetAckDiagnosticsEnabled(true);
	component->RegisterTarget(1, 0xFF);
	component->TriggerEventV2(1, std::string(100, 'n'), "payload", 50000);
	component->NetworkTick();
	component->SetAckDiagnosticsEnabled(false);
	component->SetAckDiagnosticsEnabled(true);
	fx::EventReassemblyAckInfo info;
	net::packet::ReassembledEventV2 ack;
	ack.eventId = 0;
	ack.packetIdx = 0;
	ack.totalPackets = 1;
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Accepted);
	REQUIRE(info.observedSends == 0);
	REQUIRE(std::string(info.eventName.data()) == std::string(64, 'n'));
	component->UnregisterTarget(1);
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::RetiredDisconnected);
	REQUIRE(info.hasTransfer);
	ack.totalPackets = 2;
	REQUIRE(component->HandlePacketV2Ack(1, ack, &info) == fx::EventReassemblyAckResult::Invalid);
	REQUIRE(info.reason == fx::EventReassemblyAckReason::UnknownTarget);
	REQUIRE_FALSE(info.hasTransfer);
}

TEST_CASE("V2 ACK retired history is bounded and validates identity and lifetime", "[latent-ack]")
{
	auto history = std::make_unique<fx::EventReassemblyAckHistory>();
	fx::EventReassemblyAckInfo saved;
	saved.hasTransfer = true;
	saved.expectedFragments = 3;
	saved.reason = GENERATE(fx::EventReassemblyAckReason::RetiredCompleted, fx::EventReassemblyAckReason::RetiredExpired, fx::EventReassemblyAckReason::RetiredDisconnected);
	fx::EventReassemblyAckInfo found;
	history->Remember(42, 1, 1000, saved);
	REQUIRE(history->Find(42, 1, 3, 1001, found));
	REQUIRE(found.reason == saved.reason);
	REQUIRE(found.retiredAgeMs == 1);
	REQUIRE_FALSE(history->Find(42, 2, 3, 1001, found));
	REQUIRE_FALSE(history->Find(43, 1, 3, 1001, found));
	REQUIRE_FALSE(history->Find(42, 1, 4, 1001, found));
	REQUIRE_FALSE(history->Find(42, 1, 3, 999, found));
	REQUIRE(history->Find(42, 1, 3, 120999, found));
	REQUIRE_FALSE(history->Find(42, 1, 3, 121000, found));
	constexpr size_t count = fx::EventReassemblyAckHistory::kCapacity * 20;
	for (size_t i = 0; i < count; ++i)
	{
		history->Remember(i, 1, 1000, saved);
	}
	size_t hits = 0;
	for (size_t i = 0; i < count; ++i)
	{
		hits += history->Find(i, 1, 3, 1001, found) ? 1 : 0;
	}
	REQUIRE(hits > 0);
	REQUIRE(hits <= fx::EventReassemblyAckHistory::kCapacity);
	REQUIRE(sizeof(fx::EventReassemblyAckHistory) < 256 * 1024);
}

TEST_CASE("V2 ACK diagnostic windows and logs stay bounded under a burst", "[latent-ack]")
{
	fx::LatentEventAckWindow window;
	fx::LatentEventAckLogBudget budget;
	window.Observe(1000, fx::EventReassemblyAckReason::Accepted, false, false);
	for (size_t i = 0; i < 1000000; ++i)
	{
		window.Observe(1001, fx::EventReassemblyAckReason::Duplicate, true, false);
	}
	REQUIRE(window.counts[static_cast<size_t>(fx::EventReassemblyAckReason::Accepted)] == 1);
	REQUIRE(window.counts[static_cast<size_t>(fx::EventReassemblyAckReason::Duplicate)] == 1000000);
	REQUIRE(window.softLimits == 1000000);
	REQUIRE(window.ShouldLog(1001, false));
	REQUIRE_FALSE(window.ShouldLog(1002, false));
	REQUIRE(window.ShouldLog(1002, true));
	window.Observe(6000, fx::EventReassemblyAckReason::UnknownEvent, true, true);
	REQUIRE(window.counts[static_cast<size_t>(fx::EventReassemblyAckReason::Duplicate)] == 0);
	REQUIRE(window.softLimits == 1);
	REQUIRE(window.overflows == 1);
	REQUIRE(window.lastGapMs == 4999);
	REQUIRE(window.maxGapMs == 4999);
	REQUIRE_FALSE(window.ShouldLog(6000, false));
	REQUIRE(window.ShouldLog(11001, false));
	size_t softLines = 0;
	size_t overflowLines = 0;
	for (size_t i = 0; i < 100000; ++i)
	{
		softLines += budget.Consume(1000, false) ? 1 : 0;
		overflowLines += budget.Consume(1000, true) ? 1 : 0;
	}
	REQUIRE(softLines == 4);
	REQUIRE(overflowLines == 4);
	REQUIRE_FALSE(budget.Consume(1999, true));
	REQUIRE(budget.Consume(2000, true));
}

TEST_CASE("V2 ACK overflow diagnostics preserve abuse enforcement", "[latent-ack]")
{
	const bool debug = GENERATE(false, true);
	const int mode = GENERATE(0, 1, 2);
	CAPTURE(debug, mode);
	fwRefContainer<fx::ServerInstanceBase> server = ServerInstance::Create();
	server->SetComponent(new fx::ClientRegistry());
	server->SetComponent<console::Context>(ConsoleContextInstance::Get());
	server->SetComponent<fx::GameServer>(fx::GameServerInstance::Create());
	auto resourceManager = fx::ResourceManagerInstance::Create();
	server->SetComponent<fx::ResourceManager>(resourceManager);
	auto component = fx::EventReassemblyComponent::Create();
	resourceManager->SetComponent<fx::EventReassemblyComponent>(component);
	TestEventReassemblySink sink;
	component->SetSink(&sink);
	const auto client = server->GetComponent<fx::ClientRegistry>()->MakeClient("ack-diagnostic-test");
	client->SetNetId(100 + (debug ? 10 : 0) + mode);
	component->RegisterTarget(client->GetNetId(), 0xFF);
	fx::ServerDecorators::ReassembledEventV2PacketHandler handler(server.GetRef());
	const auto variable = server->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("sv_debugLatentEventAcks");
	REQUIRE(variable->SetValue(debug ? "true" : "false"));
	component->TriggerEventV2(client->GetNetId(), "ack-diagnostic-test", "payload", 50000);
	component->NetworkTick();

	net::packet::ReassembledEventV2Packet ack;
	ack.data.eventId = mode ? 0 : 999;
	ack.data.packetIdx = 0;
	ack.data.totalPackets = 1;
	std::array<uint8_t, 64> buffer{};
	net::ByteWriter writer{ buffer.data(), buffer.size() };
	REQUIRE(ack.Process(writer));
	fx::ENetPacketPtr packet = fx::ENetPacketInstance::Create(buffer.data(), writer.GetOffset());
	if (mode == 2)
	{
		net::ByteReader reader{ buffer.data() + sizeof(uint32_t), writer.GetOffset() - sizeof(uint32_t) };
		REQUIRE(handler.Process(server.GetRef(), client, reader, packet));
		std::this_thread::sleep_for(std::chrono::milliseconds(275));
		component->NetworkTick();
		}
	fx::GameServerInstance::dropClientData.reset();
	size_t processed = 0;
	for (; processed < 10000 && !fx::GameServerInstance::dropClientData; ++processed)
	{
		net::ByteReader reader{ buffer.data() + sizeof(uint32_t), writer.GetOffset() - sizeof(uint32_t) };
		REQUIRE(handler.Process(server.GetRef(), client, reader, packet));
	}
	REQUIRE(fx::GameServerInstance::dropClientData.has_value());
	REQUIRE(fx::GameServerInstance::dropClientData->reason == "latent event ACK overflow.");
	REQUIRE(fx::GameServerInstance::dropClientData->clientDropReason == fx::ClientDropReason::LATENT_NET_EVENT_RATE_LIMIT);
	REQUIRE(processed >= 1201);
	REQUIRE(bool(client->GetData("__latentAckDiagnostics")) == debug);
	if (debug)
	{
		const auto& window = fx::AnyCast<fx::LatentEventAckWindow>(client->GetData("__latentAckDiagnostics"));
		REQUIRE(window.overflows == 1);
		const auto reason = mode == 0 ? fx::EventReassemblyAckReason::UnknownEvent :
			(mode == 1 ? fx::EventReassemblyAckReason::Duplicate : fx::EventReassemblyAckReason::RetiredCompleted);
		REQUIRE(window.counts[static_cast<size_t>(reason)] == processed - (mode == 1 ? 1 : 0));
	}
}

TEST_CASE("Valid V2 ACKs bypass the latent payload packet limiter")
{
	static constexpr uint16_t kAckCount = 400;
	const bool debug = GENERATE(false, true);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	auto resourceManager = fx::ResourceManagerInstance::Create();
	serverInstance->SetComponent<fx::ResourceManager>(resourceManager);

	auto reassemblyComponent = fx::EventReassemblyComponent::Create();
	resourceManager->SetComponent<fx::EventReassemblyComponent>(reassemblyComponent);
	TestEventReassemblySink sink;
	reassemblyComponent->SetSink(&sink);

	const auto client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("ack-test");
	client->SetNetId(debug ? 44 : 42);
	reassemblyComponent->RegisterTarget(client->GetNetId(), 0xFF);

	const size_t payloadSize = static_cast<size_t>(kAckCount) * net::packet::ReassembledEventV2::kFragmentSize - 32;
	reassemblyComponent->TriggerEventV2(client->GetNetId(), "synthetic", std::string(payloadSize, 'x'), 100000000);
	reassemblyComponent->NetworkTick();
	while (sink.packetQueueV2.size() < kAckCount)
	{
		const size_t packetsBeforeTick = sink.packetQueueV2.size();
		std::this_thread::sleep_for(std::chrono::milliseconds(110));
		reassemblyComponent->NetworkTick();
		REQUIRE(sink.packetQueueV2.size() - packetsBeforeTick <= 64);
	}
	REQUIRE(sink.packetQueueV2.size() == kAckCount);

	fx::ServerDecorators::ReassembledEventV2PacketHandler handler(serverInstance.GetRef());
	const auto debugVariable = serverInstance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("sv_debugLatentEventAcks");
	REQUIRE(debugVariable->SetValue(debug ? "true" : "false"));
	for (uint16_t packetIdx = 0; packetIdx < kAckCount; ++packetIdx)
	{
		net::packet::ReassembledEventV2Packet ackPacket;
		ackPacket.data.eventId = sink.packetQueueV2[packetIdx].second.eventId;
		ackPacket.data.packetIdx = packetIdx;
		ackPacket.data.totalPackets = kAckCount;

		std::vector<uint8_t> packetBuffer(net::SerializableComponent::GetMaxSize<net::packet::ReassembledEventV2Packet>());
		net::ByteWriter writer{ packetBuffer.data(), packetBuffer.size() };
		REQUIRE(ackPacket.Process(writer));

		net::ByteReader reader{ packetBuffer.data(), writer.GetOffset() };
		uint32_t packetType;
		REQUIRE(reader.Field(packetType));
		REQUIRE(packetType == fx::ServerDecorators::ReassembledEventV2PacketHandler::PacketType);

		fx::ENetPacketPtr packet = fx::ENetPacketInstance::Create(packetBuffer.data(), writer.GetOffset());
		REQUIRE(handler.Process(serverInstance.GetRef(), client, reader, packet));

		net::packet::ReassembledEventV2 ack;
		ack.eventId = sink.packetQueueV2[packetIdx].second.eventId;
		ack.packetIdx = packetIdx;
		ack.totalPackets = kAckCount;
		REQUIRE(reassemblyComponent->HandlePacketV2Ack(client->GetNetId(), ack) == fx::EventReassemblyAckResult::Duplicate);
	}
}

TEST_CASE("V2 payload soft rate threshold does not silently discard fragments")
{
	static constexpr size_t kPacketsPastSoftBurst = 150;

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	serverInstance->SetComponent<fx::GameServer>(fx::GameServerInstance::Create());
	auto resourceManager = fx::ResourceManagerInstance::Create();
	serverInstance->SetComponent<fx::ResourceManager>(resourceManager);

	auto reassemblyComponent = fx::EventReassemblyComponent::Create();
	resourceManager->SetComponent<fx::EventReassemblyComponent>(reassemblyComponent);
	TestEventReassemblySink sink;
	reassemblyComponent->SetSink(&sink);

	const auto client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("payload-test");
	client->SetNetId(43);
	reassemblyComponent->RegisterTarget(client->GetNetId(), 0xFF);
	fx::GameServerInstance::dropClientData.reset();

	fx::ServerDecorators::ReassembledEventV2PacketHandler handler(serverInstance.GetRef());
	auto processPayload = [&](uint64_t eventId)
	{
		std::array<uint8_t, 1> payload{ 0x42 };
		net::packet::ReassembledEventV2Packet payloadPacket;
		payloadPacket.data.eventId = eventId;
		payloadPacket.data.packetIdx = 0;
		payloadPacket.data.totalPackets = 1;
		payloadPacket.data.data.SetValue({ payload.data(), payload.size() });

		std::vector<uint8_t> packetBuffer(net::SerializableComponent::GetMaxSize<net::packet::ReassembledEventV2Packet>());
		net::ByteWriter writer{ packetBuffer.data(), packetBuffer.size() };
		REQUIRE(payloadPacket.Process(writer));

		net::ByteReader reader{ packetBuffer.data(), writer.GetOffset() };
		uint32_t packetType;
		REQUIRE(reader.Field(packetType));
		REQUIRE(packetType == fx::ServerDecorators::ReassembledEventV2PacketHandler::PacketType);

		fx::ENetPacketPtr packet = fx::ENetPacketInstance::Create(packetBuffer.data(), writer.GetOffset());
		REQUIRE(handler.Process(serverInstance.GetRef(), client, reader, packet));
	};

	for (size_t i = 0; i < kPacketsPastSoftBurst; ++i)
	{
		processPayload(i);
	}

	REQUIRE_FALSE(fx::GameServerInstance::dropClientData.has_value());

	for (size_t i = kPacketsPastSoftBurst; i < 500 && !fx::GameServerInstance::dropClientData; ++i)
	{
		processPayload(i);
	}
	REQUIRE(fx::GameServerInstance::dropClientData.has_value());
	REQUIRE(fx::GameServerInstance::dropClientData->client == client);
	REQUIRE(fx::GameServerInstance::dropClientData->clientDropReason == fx::ClientDropReason::LATENT_NET_EVENT_RATE_LIMIT);
}

TEST_CASE("V2 catch-up is globally bounded and rotates pending events")
{
	static constexpr size_t kEventCount = 20;
	static constexpr size_t kFragmentsPerEvent = 200;

	auto reassemblyComponent = fx::EventReassemblyComponent::Create();
	TestEventReassemblySink sink;
	reassemblyComponent->SetSink(&sink);
	reassemblyComponent->RegisterTarget(1, 0xFF);

	const size_t payloadSize = kFragmentsPerEvent * net::packet::ReassembledEventV2::kFragmentSize - 32;
	for (size_t i = 0; i < kEventCount; ++i)
	{
		reassemblyComponent->TriggerEventV2(1, "bounded", std::string(payloadSize, 'x'), 100000000);
	}
	reassemblyComponent->NetworkTick();

	auto countEventPackets = [&sink](uint64_t eventId) -> size_t
	{
		return static_cast<size_t>(std::count_if(sink.packetQueueV2.begin(), sink.packetQueueV2.end(), [eventId](const auto& entry)
		{
			return entry.second.eventId == eventId;
		}));
	};

	std::this_thread::sleep_for(std::chrono::milliseconds(110));
	std::vector<size_t> eventCountsBeforeFirstCatchUp(kEventCount);
	for (size_t i = 0; i < kEventCount; ++i)
	{
		eventCountsBeforeFirstCatchUp[i] = countEventPackets(i);
	}
	const size_t packetsBeforeFirstCatchUp = sink.packetQueueV2.size();
	reassemblyComponent->NetworkTick();
	REQUIRE(sink.packetQueueV2.size() - packetsBeforeFirstCatchUp == 1024);

	std::vector<uint64_t> deferredEvents;
	for (size_t i = 0; i < kEventCount; ++i)
	{
		if (countEventPackets(i) == eventCountsBeforeFirstCatchUp[i])
		{
			deferredEvents.push_back(i);
		}
	}
	REQUIRE_FALSE(deferredEvents.empty());

	std::this_thread::sleep_for(std::chrono::milliseconds(110));
	const size_t packetsBeforeSecondCatchUp = sink.packetQueueV2.size();
	reassemblyComponent->NetworkTick();
	REQUIRE(sink.packetQueueV2.size() - packetsBeforeSecondCatchUp == 1024);
	REQUIRE(std::any_of(deferredEvents.begin(), deferredEvents.end(), [&countEventPackets, &eventCountsBeforeFirstCatchUp](uint64_t eventId)
	{
		return countEventPackets(eventId) > eventCountsBeforeFirstCatchUp[eventId];
	}));
}
