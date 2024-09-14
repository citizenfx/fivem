#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ConsoleContextInstance.h"
#include "GameServer.h"
#include "GameStateNAck.h"
#include "NetBuffer.h"
#include "ServerGameStatePublicInstance.h"
#include "ServerInstance.h"
#include "TestUtils.h"
#include "packethandlers/GameStateAckPacketHandler.h"
#include "state/ServerGameStatePublic.h"

TEST_CASE("game state ack handler test")
{
	fx::SetOneSyncGetCallback([] { return true; });

	REQUIRE(std::string(fx::ServerDecorators::GameStateAckPacketHandler::GetPacketId()) == "gameStateAck");
	REQUIRE(HashRageString(fx::ServerDecorators::GameStateAckPacketHandler::GetPacketId()) == 0xa5d4e2bc);
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
	uint64_t frameIndex = fx::TestUtils::u64Random(UINT64_MAX);
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

	net::Buffer outBuffer;
	outBuffer.Write<uint64_t>(frameIndex);
	outBuffer.Write<uint8_t>(static_cast<uint8_t>(ignoreList.size()));
	for (auto [entry, lastFrame] : ignoreList)
	{
		outBuffer.Write<uint16_t>(entry);
		outBuffer.Write<uint64_t>(lastFrame);
	}
	
	outBuffer.Write<uint8_t>(static_cast<uint8_t>(recreateList.size()));
	for (uint16_t entry : recreateList)
	{
		outBuffer.Write<uint16_t>(entry);
	}

	fx::ServerGameStatePublicInstance::GetGameStateAckLastCall().reset();
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	fx::ServerDecorators::GameStateAckPacketHandler handler(serverInstance.GetRef());
	outBuffer.Reset();
	handler.Handle(serverInstance.GetRef(), client, outBuffer);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateAckLastCall().has_value() == true);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateAckLastCall().value().client == client);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateAckLastCall().value().frameIndex == frameIndex);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateAckLastCall().value().ignoreList.size() == ignoreList.size());
	REQUIRE(memcmp(fx::ServerGameStatePublicInstance::GetGameStateAckLastCall().value().ignoreList.data(), ignoreList.data(), ignoreList.size() * sizeof(net::packet::ClientGameStateNAck::IgnoreListEntry)) == 0);
	REQUIRE(fx::ServerGameStatePublicInstance::GetGameStateAckLastCall().value().recreateList == recreateList);
}
