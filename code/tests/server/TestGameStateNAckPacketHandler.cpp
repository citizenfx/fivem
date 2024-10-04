#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ConsoleContextInstance.h"
#include "GameServer.h"
#include "GameStateNAck.h"
#include "NetBuffer.h"
#include "ServerGameStatePublicInstance.h"
#include "ServerInstance.h"
#include "TestUtils.h"
#include "packethandlers/GameStateNAckPacketHandler.h"
#include "state/ServerGameStatePublic.h"

TEST_CASE("game state nack handler test")
{
	fx::SetOneSyncGetCallback([] { return true; });

	REQUIRE(std::string(fx::ServerDecorators::GameStateNAckPacketHandler::GetPacketId()) == "gameStateNAck");
	REQUIRE(HashRageString(fx::ServerDecorators::GameStateNAckPacketHandler::GetPacketId()) == 0xd2f86a6e);
	// test is only implemented for onesync
	REQUIRE(fx::IsOneSync() == true);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ServerGameStatePublic>(fx::ServerGameStatePublicInstance::Create());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	
	// for serialization of the ignore list with a net::Span the struct needs to be correctly matching the size
	REQUIRE(sizeof(net::packet::ClientGameStateNAck::IgnoreListEntry) == 10);

	bool emptyIgnoreList = GENERATE(true, false);
	bool emptyRecreateList = GENERATE(true, false);
	bool isMissingFrames = GENERATE(true, false);
	uint64_t frameIndex = fx::TestUtils::u64Random(UINT64_MAX);
	uint64_t firstMissingFrame = fx::TestUtils::u64Random(UINT64_MAX);
	uint64_t lastMissingFrame = fx::TestUtils::u64Random(UINT64_MAX);
	std::vector<net::packet::ClientGameStateNAck::IgnoreListEntry> ignoreList(emptyIgnoreList ? 0 : fx::TestUtils::u64Random(UINT8_MAX - 1));
	for (uint64_t i = 0; i < ignoreList.size(); ++i)
	{
		ignoreList[i].entry = fx::TestUtils::u64Random(UINT16_MAX);
		ignoreList[i].lastFrame = fx::TestUtils::u64Random(UINT64_MAX);
	}

	std::vector<uint16_t> recreateList(emptyRecreateList ? 0 : fx::TestUtils::u64Random(UINT8_MAX - 1));
	for (uint64_t i = 0; i < recreateList.size(); ++i)
	{
		recreateList[i] = fx::TestUtils::u64Random(UINT16_MAX);
	}

	uint8_t flags = 8;
	if (isMissingFrames)
	{
		flags |= 1;
	}

	if (!ignoreList.empty())
	{
		flags |= 2;
	}

	if (!recreateList.empty())
	{
		flags |= 4;
	}

	net::Buffer outBuffer;
	outBuffer.Write<uint8_t>(flags);

	outBuffer.Write<uint64_t>(frameIndex);

	if (isMissingFrames)
	{
		outBuffer.Write<uint64_t>(firstMissingFrame);
		outBuffer.Write<uint64_t>(lastMissingFrame);
	}

	if (!ignoreList.empty())
	{
		outBuffer.Write<uint8_t>(static_cast<uint8_t>(ignoreList.size()));
		for (auto [entry, lastFrame] : ignoreList)
		{
			outBuffer.Write<uint16_t>(entry);
			outBuffer.Write<uint64_t>(lastFrame);
		}
	}

	if (!recreateList.empty())
	{
		outBuffer.Write<uint8_t>(static_cast<uint8_t>(recreateList.size()));
		for (uint16_t entry : recreateList)
		{
			outBuffer.Write<uint16_t>(entry);
		}
	}

	fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().reset();
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	fx::ServerDecorators::GameStateNAckPacketHandler handler(serverInstance.GetRef());
	outBuffer.Reset();
	handler.Handle(serverInstance.GetRef(), client, outBuffer);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().has_value() == true);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().value().client == client);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().value().flags == flags);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().value().frameIndex == frameIndex);
	if (isMissingFrames)
	{
		REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().value().firstMissingFrame == firstMissingFrame);
		REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().value().lastMissingFrame == lastMissingFrame);
	}

	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().value().ignoreList.size() == ignoreList.size());
	REQUIRE(memcmp(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().value().ignoreList.data(), ignoreList.data(), ignoreList.size() * sizeof(net::packet::ClientGameStateNAck::IgnoreListEntry)) == 0);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall().value().recreateList == recreateList);
}
