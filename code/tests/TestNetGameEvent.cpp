#include <StdInc.h>

#include <state/ServerGameStatePublic.h>
#include "ServerGameStatePublicInstance.h"

#include <catch_amalgamated.hpp>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "GameServer.h"
#include "NetGameEventV2.h"
#include "ServerInstance.h"
#include "packethandlers/NetGameEventPacketHandler.h"

#include "TestUtils.h"

namespace fx
{
	class ClientMetricData
	{
	public:
		fx::Client* thisptr;
		int channel;
		const net::Buffer buffer;
		NetPacketType flags;

		ClientMetricData(fx::Client* const thisptr, const int channel, const net::Buffer& buffer, const NetPacketType flags)
			: thisptr(thisptr),
			  channel(channel),
			  buffer(buffer),
			  flags(flags)
		{
		}
	};
}

TEST_CASE("NetGameEventV2 test")
{
	fx::SetOneSyncGetCallback([] { return true; });

	REQUIRE(std::string(NetGameEventPacketHandlerV2::GetPacketId()) == std::string("msgNetGameEventV2"));
	REQUIRE(HashRageString(NetGameEventPacketHandlerV2::GetPacketId()) == 0x2b513e19);
	// test is only implemented for onesync
	REQUIRE(fx::IsOneSync() == true);

	fx::ServerInstanceBase* serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ServerGameStatePublic>(fx::ServerGameStatePublicInstance::Create());
	
	std::vector<uint8_t> randomEventData (1024);
	fx::TestUtils::fillVectorU8Random(randomEventData);

	const uint16_t eventId = fx::TestUtils::u64Random(UINT16_MAX);
	const bool isReply = fx::TestUtils::u64Random(1) == 1;
	const uint32_t eventNameHash = HashString("test_event2");

	net::packet::ClientNetGameEventV2 clientData;
	clientData.isReply = isReply;
	clientData.eventId = eventId;
	std::vector<uint16_t> targetPlayers;
	targetPlayers.push_back(1);
	clientData.targetPlayers = {targetPlayers.data(), targetPlayers.size()};
	clientData.eventNameHash = eventNameHash;
	clientData.data = {randomEventData.data(), randomEventData.size()};

	std::vector<uint8_t> packetBuffer (net::SerializableComponent::GetSize<net::packet::ClientNetGameEventV2>());
	net::ByteWriter writer {packetBuffer.data(), packetBuffer.size()};

	REQUIRE(clientData.Process(writer) == true);

	fx::ServerGameStatePublicInstance::GetGameEventHandlerLastCall().reset();
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	static std::vector<fx::ClientMetricData> lastClientMetricData;
	lastClientMetricData.clear();
	client->SetNetworkMetricsSendCallback([](fx::Client* thisPtr, int channel, const net::Buffer& buffer, NetPacketType flags)
	{
		lastClientMetricData.emplace_back(thisPtr, channel, buffer, flags);
	});
	fx::ServerGameStatePublicInstance::SetClientRoutingBucket(client, 1);

	NetGameEventPacketHandlerV2 handler(serverInstance);
	net::Buffer buffer {packetBuffer.data(), writer.GetOffset()};
	buffer.Reset();
	handler.Handle(serverInstance, client, buffer);

	REQUIRE(fx::ServerGameStatePublicInstance::GetGameEventHandlerLastCall().has_value() == true);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameEventHandlerLastCall().value().client == client);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameEventHandlerLastCall().value().eventId == eventId);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameEventHandlerLastCall().value().isReply == isReply);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameEventHandlerLastCall().value().data == randomEventData);

	REQUIRE(lastClientMetricData.size() == 1);
	REQUIRE(lastClientMetricData[0].thisptr == client.get());
	REQUIRE(lastClientMetricData[0].buffer.GetLength() == 13 + randomEventData.size());
	REQUIRE(lastClientMetricData[0].channel == 1);
	REQUIRE(lastClientMetricData[0].flags == NetPacketType::NetPacketType_Reliable);

	net::ByteReader reader {lastClientMetricData[0].buffer.GetBuffer(), lastClientMetricData[0].buffer.GetLength()};
	net::packet::ServerNetGameEventV2 serverNetGameEvent;
	REQUIRE(serverNetGameEvent.Process(reader) == true);
	REQUIRE(reader.GetOffset() == reader.GetCapacity());
	REQUIRE(serverNetGameEvent.data.GetValue() == net::Span<uint8_t>{randomEventData.data(), randomEventData.size()});
	REQUIRE(serverNetGameEvent.eventId == eventId);
	REQUIRE(serverNetGameEvent.isReply == isReply);
	REQUIRE(serverNetGameEvent.eventNameHash == eventNameHash);
	REQUIRE(serverNetGameEvent.clientNetId == static_cast<uint16_t>(client->GetNetId()));

	delete serverInstance;
}
