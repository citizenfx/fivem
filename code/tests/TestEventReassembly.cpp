#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <StateBagComponent.h>

#include "ByteReader.h"
#include "EventReassemblyComponent.h"
#include "TestUtils.h"
#include "state/RlMessageBuffer.h"

class TestEventReassemblySink : public fx::EventReassemblySink
{
public:
	std::vector<std::pair<int, std::string>> packetQueue;

	void SendPacket(int target, std::string_view data) override
	{
		packetQueue.emplace_back(target, data);
	}

	bool LimitEvent(int source) override
	{
		// only used for rate limiting, can be always false for tests
		return false;
	}
};

TEST_CASE("Reassembly client to server event test")
{
	std::string testEventName = fx::TestUtils::asciiRandom(10);
	std::string testEventData = fx::TestUtils::asciiRandom(50);

	auto reassemblyComponentServer = fx::EventReassemblyComponent::Create();
	TestEventReassemblySink reassemblySinkServer;
	reassemblyComponentServer->SetSink(&reassemblySinkServer);
	// register client net id 1 target
	reassemblyComponentServer->RegisterTarget(1, 0xFF);

	auto reassemblyComponentClient = fx::EventReassemblyComponent::Create();
	TestEventReassemblySink reassemblySinkClient;
	reassemblyComponentClient->SetSink(&reassemblySinkClient);
	// register server target
	reassemblyComponentClient->RegisterTarget(0, 0xFF);

	int serverTarget = 0;
	int bytesPerSecond = 50000;
	reassemblyComponentClient->TriggerEvent(serverTarget, testEventName, testEventData, bytesPerSecond);

	// should be empty when network tick does not run
	REQUIRE(reassemblySinkClient.packetQueue.empty() == true);

	reassemblyComponentClient->NetworkTick();

	REQUIRE(reassemblySinkClient.packetQueue.size() == 1);

	REQUIRE(std::get<0>(reassemblySinkClient.packetQueue[0]) == 0);

	std::string packet = std::get<1>(reassemblySinkClient.packetQueue[0]);

	rl::MessageBuffer buffer(packet.data(), packet.size());

	uint32_t eventIdLow = 0;
	uint32_t eventIdHigh = 0;

	if (!buffer.Read(32, &eventIdLow) || !buffer.Read(32, &eventIdHigh))
	{
		REQUIRE(false);
	}

	constexpr const uint32_t kPacketSizeBits = 17;
	constexpr const uint32_t kFragmentSize = 1024 - 1;
	constexpr const uint32_t kFragmentSizeBits = 10;
	constexpr const uint32_t kMaxPacketSize = (1 << kPacketSizeBits) * kFragmentSize;

	uint64_t eventId = ((uint64_t)eventIdHigh << 32) | eventIdLow;

	uint32_t packetIdx;
	uint32_t totalPackets;
	uint32_t thisBytes;

	std::array<uint8_t, kFragmentSize> payload;

	if (!buffer.Read(kPacketSizeBits, &packetIdx))
	{
		REQUIRE(false);
	}

	if (!buffer.Read(kPacketSizeBits, &totalPackets))
	{
		REQUIRE(false);
	}

	if (!buffer.Read(kFragmentSizeBits, &thisBytes))
	{
		REQUIRE(false);
	}

	if (thisBytes > 0)
	{
		buffer.ReadBits(payload.data(), std::min(size_t(thisBytes), payload.size()) * 8);
	}

	REQUIRE(eventId == 0);
	REQUIRE(packetIdx == 0);
	REQUIRE(totalPackets == 1);
	REQUIRE(thisBytes == testEventData.size() + testEventName.size() + 2);

	rl::MessageBuffer payloadReader(payload.data(), thisBytes);

	static char eventName[65536];

	uint16_t nameLength = payloadReader.Read<uint16_t>(16);
	payloadReader.ReadBits(eventName, nameLength * 8);
	eventName[nameLength] = '\0';

	REQUIRE(testEventName == std::string(eventName));
	REQUIRE(testEventData == std::string{
	reinterpret_cast<const char*>(payloadReader.GetBuffer().data() + (payloadReader.GetCurrentBit() / 8)),
	payloadReader.GetBuffer().size() - (payloadReader.GetCurrentBit() / 8)
	});
}
