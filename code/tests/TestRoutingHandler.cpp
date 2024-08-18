#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "GameServer.h"
#include <state/ServerGameStatePublic.h>
#include "ServerGameStatePublicInstance.h"
#include "ServerInstance.h"
#include "packethandlers/RoutingPacketHandler.h"

#include "TestUtils.h"

TEST_CASE("Routing handler test")
{
	fx::SetOneSyncGetCallback([] { return true; });

	REQUIRE(std::string(RoutingPacketHandler::GetPacketId()) == "msgRoute");
	REQUIRE(HashRageString(RoutingPacketHandler::GetPacketId()) == 0xE938445B);
	// test is only implemented for onesync
	REQUIRE(fx::IsOneSync() == true);

	fx::ServerInstanceBase* serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ServerGameStatePublic>(fx::ServerGameStatePublicInstance::Create());

	net::Buffer buffer;
	buffer.Write<uint16_t>(1); // target net id
	std::vector<uint8_t> data(1200);
	fx::TestUtils::fillVectorU8Random(data);
	buffer.Write<uint16_t>(data.size()); // packet length
	buffer.Write(data.data(), data.size());
	buffer.Reset();

	fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall().reset();
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	RoutingPacketHandler handler(serverInstance);
	handler.Handle(serverInstance, client, buffer);

	REQUIRE(fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall().has_value() == true);
	REQUIRE(fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall().value().client == client);
	REQUIRE(fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall().value().packetData == data);

	delete serverInstance;
}
