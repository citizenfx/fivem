#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "GameServer.h"
#include <state/ServerGameStatePublic.h>

#include "ClientMetricData.h"
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

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
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
	RoutingPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

	REQUIRE(fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall().has_value() == true);
	REQUIRE(fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall().value().client == client);
	REQUIRE(fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall().value().packetData == data);
}

TEST_CASE("Routing handler none OneSync test")
{
	fx::SetOneSyncGetCallback([] { return false; });

	REQUIRE(std::string(RoutingPacketHandler::GetPacketId()) == "msgRoute");
	REQUIRE(HashRageString(RoutingPacketHandler::GetPacketId()) == 0xE938445B);
	// test is only implemented for none onesync
	REQUIRE(fx::IsOneSync() == false);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());

	net::Buffer buffer;
	buffer.Write<uint16_t>(2); // target net id
	std::vector<uint8_t> data(1200);
	fx::TestUtils::fillVectorU8Random(data);
	buffer.Write<uint16_t>(data.size()); // packet length
	buffer.Write(data.data(), data.size());
	buffer.Reset();

	fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall().reset();
	const fx::ClientSharedPtr client1 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test1");
	REQUIRE(client1->GetNetId() == 1);
	const fx::ClientSharedPtr client2 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test2");
	REQUIRE(client2->GetNetId() == 2);

	static std::optional<fx::ClientMetricData> lastClientMetricData;
	lastClientMetricData.reset();
	client2->SetNetworkMetricsSendCallback([](fx::Client* thisPtr, int channel, const net::Buffer& buffer, NetPacketType flags)
	{
		lastClientMetricData.emplace(thisPtr, channel, buffer, flags);
	});

	RoutingPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client1, buffer);

	REQUIRE(lastClientMetricData.has_value() == true);
	auto receivedBuffer = lastClientMetricData.value().buffer;
	receivedBuffer.Reset();
	REQUIRE(receivedBuffer.GetRemainingBytes() == 8 + data.size());
	REQUIRE(receivedBuffer.Read<uint32_t>() == HashRageString("msgRoute"));
	REQUIRE(receivedBuffer.Read<uint16_t>() == client1->GetNetId());
	REQUIRE(receivedBuffer.Read<uint16_t>() == data.size());
	std::vector<uint8_t> readData (data.size());
	receivedBuffer.Read(readData.data(), data.size());
	REQUIRE(readData == data);
	REQUIRE(lastClientMetricData.value().channel == 1);
	REQUIRE(lastClientMetricData.value().flags == NetPacketType_Unreliable);
	REQUIRE(lastClientMetricData.value().thisptr == client2.get());
}
