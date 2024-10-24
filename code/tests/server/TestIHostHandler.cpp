#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "ENetPacketInstance.h"
#include "GameServer.h"
#include "GameServerInstance.h"
#include "ServerInstance.h"

#include "TestUtils.h"
#include "packethandlers/IHostPacketHandler.h"

TEST_CASE("IHost test")
{
	// test with new and old packet buffer
	bool newPacketBuffer = GENERATE(true, false);

	fx::SetOneSyncGetCallback([] { return false; });

	REQUIRE(fx::ServerDecorators::IHostPacketHandler::PacketType == HashRageString("msgIHost"));
	// test is only implemented for disabled onesync
	REQUIRE(fx::IsOneSync() == false);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::GameServer>(fx::GameServerInstance::Create());

	net::packet::ClientIHost clientIHost;
	clientIHost.baseNum = fx::TestUtils::u64Random(UINT32_MAX);

	std::vector<uint8_t> packetBuffer (net::SerializableComponent::GetMaxSize<net::packet::ClientIHost>());
	net::ByteWriter writer {packetBuffer.data(), packetBuffer.size()};

	REQUIRE(clientIHost.Process(writer) == true);
	
	const fx::ClientSharedPtr client1 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	REQUIRE(client1->GetNetId() == 1);
	client1->SetHasRouted();
	
	fx::GameServerInstance::broadcastData.reset();
	
	fx::ServerDecorators::IHostPacketHandler handler(serverInstance.GetRef());
	net::Buffer buffer {packetBuffer.data(), writer.GetOffset()};

	if (newPacketBuffer)
	{
		fx::ENetPacketPtr packetPtr = fx::ENetPacketInstance::Create(buffer.GetBuffer(), buffer.GetLength());
		net::ByteReader handlerReader(buffer.GetBuffer(), buffer.GetLength());
		handler.Process(serverInstance.GetRef(), client1, handlerReader, packetPtr);
	}
	else
	{
		net::Buffer oldNetBuffer;
		oldNetBuffer.Write<uint32_t>(clientIHost.baseNum);
		oldNetBuffer.Reset();
		fx::ENetPacketPtr packetPtr = fx::ENetPacketInstance::Create(oldNetBuffer.GetBuffer(), oldNetBuffer.GetLength());
		net::ByteReader handlerReader(oldNetBuffer.GetBuffer(), oldNetBuffer.GetLength());
		handler.Process(serverInstance.GetRef(), client1, handlerReader, packetPtr);
	}

	REQUIRE(fx::GameServerInstance::broadcastData.has_value() == true);
	REQUIRE(fx::GameServerInstance::broadcastData.value().size() == 10);
	net::ByteReader reader {fx::GameServerInstance::broadcastData.value().data(), fx::GameServerInstance::broadcastData.value().size()};
	uint32_t packet;
	uint16_t netId;
	uint32_t base;
	REQUIRE(reader.Field(packet) == true);
	REQUIRE(reader.Field(netId) == true);
	REQUIRE(reader.Field(base) == true);
	REQUIRE(packet == HashRageString("msgIHost"));
	REQUIRE(netId == client1->GetNetId());
	REQUIRE(base == clientIHost.baseNum);
}
