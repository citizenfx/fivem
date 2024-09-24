#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "GameServer.h"
#include <state/ServerGameStatePublic.h>

#include "ByteReader.h"
#include "ClientMetricData.h"
#include "ObjectIds.h"
#include "RoundToType.h"
#include "ServerGameStatePublicInstance.h"
#include "ServerInstance.h"

#include "TestUtils.h"
#include "packethandlers/RequestObjectIdsPacketHandler.h"

TEST_CASE("Send object ids test")
{
	// bitsets inside OneSync use the following logic to calculate the size
	static constexpr const int MaxObjectId = (1 << 16) - 1;
	REQUIRE(net::roundToType<size_t>(MaxObjectId) == 65536);
	// because this is 2^16 eastl::bitset<roundToWord(MaxObjectId)> the bitset only has 2^16 bits
	// the size of the bitset -1 is the maximum possible id, so entities ids are 16 bit
	// so the id packet can be modified to use uint16_t

	const bool isBigMode = GENERATE(true, false);
	const bool usePacket = GENERATE(true, false);
	fx::SetBigModeHack(isBigMode, true);

	fx::SetOneSyncGetCallback([]
	{
		return true;
	});

	int idCount;
	if (isBigMode)
	{
		idCount = GENERATE(0, 1, 2, 3, 4, 5, 6);
	} else
	{
		idCount = fx::TestUtils::u64Random(32);
	}

	fx::ServerGameStatePublicInstance::GetFreeObjectIds().clear();
	for (int i = 0; i < idCount; i++)
	{
		fx::ServerGameStatePublicInstance::GetFreeObjectIds().push_back(fx::TestUtils::u64Random(UINT16_MAX - 1) + 1);
	}

	// object ids need to be sorted from small to large for the algorithm to work
	std::sort(fx::ServerGameStatePublicInstance::GetFreeObjectIds().begin(), fx::ServerGameStatePublicInstance::GetFreeObjectIds().end());

	REQUIRE(std::string(RequestObjectIdsPacketHandler::GetPacketId()) == "msgRequestObjectIds");
	REQUIRE(HashRageString(RequestObjectIdsPacketHandler::GetPacketId()) == 0xb8e611cf);
	// test is only implemented for onesync
	REQUIRE(fx::IsOneSync() == true);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ServerGameStatePublic>(fx::ServerGameStatePublicInstance::Create());
	
	net::Buffer buffer;

	fx::ServerGameStatePublicInstance::GetFreeObjectIdsLastCall().reset();
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");

	static std::optional<fx::ClientMetricData> lastClientMetricData;
	lastClientMetricData.reset();
	client->SetNetworkMetricsSendCallback([](fx::Client* thisPtr, int channel, const net::Buffer& buffer, NetPacketType flags)
	{
		lastClientMetricData.emplace(thisPtr, channel, buffer, flags);
	});

	RequestObjectIdsPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

	REQUIRE(fx::ServerGameStatePublicInstance::GetFreeObjectIdsLastCall().has_value() == true);
	REQUIRE(fx::ServerGameStatePublicInstance::GetFreeObjectIdsLastCall().value().client == client);

	if (isBigMode)
	{
		REQUIRE(fx::ServerGameStatePublicInstance::GetFreeObjectIdsLastCall().value().numIds == 6);
	}
	else
	{
		REQUIRE(fx::ServerGameStatePublicInstance::GetFreeObjectIdsLastCall().value().numIds == 32);
	}

	REQUIRE(lastClientMetricData.has_value() == true);
	REQUIRE(lastClientMetricData.value().channel == 1);
	REQUIRE(lastClientMetricData.value().flags == NetPacketType_Reliable);

	net::Buffer clientReceiveBuffer = lastClientMetricData.value().buffer;
	clientReceiveBuffer.Reset();

	std::vector<uint16_t> receivedIds;
	if (usePacket)
	{
		if (isBigMode)
		{
			net::packet::ServerObjectIdsPacket<true> packet;
			net::ByteReader reader{ clientReceiveBuffer.GetBuffer(), clientReceiveBuffer.GetLength() };
			REQUIRE(packet.Process(reader) == true);
			packet.data.GetIds(receivedIds);
		}
		else
		{
			net::packet::ServerObjectIdsPacket<false> packet;
			net::ByteReader reader{ clientReceiveBuffer.GetBuffer(), clientReceiveBuffer.GetLength() };
			REQUIRE(packet.Process(reader) == true);
			packet.data.GetIds(receivedIds);
		}
	}
	else
	{
		REQUIRE(clientReceiveBuffer.Read<uint32_t>() == HashRageString("msgObjectIds"));
	
		auto numIds = clientReceiveBuffer.Read<uint16_t>();

		uint16_t last = 0;
		for (uint16_t i = 0; i < numIds; i++)
		{
			auto skip = clientReceiveBuffer.Read<uint16_t>();
			auto take = clientReceiveBuffer.Read<uint16_t>();

			last += skip + 1;

			for (uint16_t j = 0; j <= take; j++)
			{
				int objectId = last++;
				receivedIds.push_back(objectId);
			}
		}
	}

	REQUIRE(receivedIds == fx::ServerGameStatePublicInstance::GetFreeObjectIds());
}
