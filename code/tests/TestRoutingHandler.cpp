#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "GameServer.h"
#include "ServerInstance.h"
#include "packethandlers/RoutingPacketHandler.h"

#include "TestUtils.h"

struct ParseGameStatePacketData
{
	const fx::ClientSharedPtr client;
	const std::vector<uint8_t> packetData;

	ParseGameStatePacketData(const fx::ClientSharedPtr& _client, const std::vector<uint8_t>& _packetData)
		: client(_client),
		  packetData(_packetData)
	{
	}
};

class GameState : public fx::ServerGameStatePublic
{
public:
	static inline std::optional<ParseGameStatePacketData> parseGameStatePacketDataLastCall{};

	void SendObjectIds(const fx::ClientSharedPtr& client, int numIds) override
	{
	}

	fx::SyncStyle GetSyncStyle() override
	{
		return fx::SyncStyle::NAK;
	}

	fx::EntityLockdownMode GetEntityLockdownMode(const fx::ClientSharedPtr& client) override
	{
		return fx::EntityLockdownMode::Strict;
	}

	void ParseGameStatePacket(const fx::ClientSharedPtr& client, const std::vector<uint8_t>& packetData) override
	{
		parseGameStatePacketDataLastCall.emplace(client, packetData);
	}

	void ForAllEntities(const std::function<void(fx::sync::Entity*)>& cb) override
	{
		
	}
};

TEST_CASE("Routing handler test")
{
	fx::SetOneSyncGetCallback([] { return true; });

	REQUIRE(RoutingPacketHandler::GetPacketId() == "msgRoute");
	REQUIRE(HashRageString(RoutingPacketHandler::GetPacketId()) == 0xE938445B);
	// test is only implemented for onesync
	REQUIRE(fx::IsOneSync() == true);

	fx::ServerInstanceBase* serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ServerGameStatePublic>(new GameState());

	net::Buffer buffer;
	buffer.Write<uint16_t>(1); // target net id
	std::vector<uint8_t> data (1200);
	fx::TestUtils::fillVectorU8Random(data);
	buffer.Write<uint16_t>(data.size()); // packet length
	buffer.Write(data.data(), data.size());
	buffer.Reset();

	GameState::parseGameStatePacketDataLastCall.reset();
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	RoutingPacketHandler::Handle(serverInstance, client, buffer);

	REQUIRE(GameState::parseGameStatePacketDataLastCall.has_value() == true);
	REQUIRE(GameState::parseGameStatePacketDataLastCall.value().client == client);
	REQUIRE(GameState::parseGameStatePacketDataLastCall.value().packetData == data);

	delete serverInstance;
}
