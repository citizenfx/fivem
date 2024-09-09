#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "GameServer.h"
#include "GameServerInstance.h"
#include "ServerInstance.h"

#include "TestUtils.h"
#include "packethandlers/IQuitPacketHandler.h"

TEST_CASE("IQuit test")
{
	// test with new and old packet buffer
	bool newPacketBuffer = GENERATE(true, false);

	REQUIRE(std::string(fx::ServerDecorators::IQuitPacketHandler::GetPacketId()) == std::string("msgIQuit"));
	REQUIRE(HashRageString(fx::ServerDecorators::IQuitPacketHandler::GetPacketId()) == 0x522cadd1);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::GameServer>(fx::GameServerInstance::Create());

	std::string quitReason = fx::TestUtils::asciiRandom(1024 - 1/*null terminator*/);
	net::packet::ClientIQuit clientIQuit;
	clientIQuit.reason = quitReason;

	REQUIRE(net::SerializableComponent::GetSize<net::packet::ClientIQuit>() == 1024);

	std::vector<uint8_t> packetBuffer (net::SerializableComponent::GetSize<net::packet::ClientIQuit>());
	net::ByteWriter writer {packetBuffer.data(), packetBuffer.size()};

	REQUIRE(clientIQuit.Process(writer) == true);

	const fx::ClientSharedPtr client1 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	REQUIRE(client1->GetNetId() == 1);
	client1->SetHasRouted();
	
	fx::GameServerInstance::broadcastData.reset();
	
	fx::ServerDecorators::IQuitPacketHandler handler(serverInstance.GetRef());
	net::Buffer buffer {packetBuffer.data(), writer.GetOffset()};

	if (newPacketBuffer)
	{
		buffer.Seek(writer.GetOffset());
		// add null terminator
		buffer.Write<uint8_t>(0);
		buffer.Reset();
		handler.Handle(serverInstance.GetRef(), client1, buffer);
	}
	else
	{
		net::Buffer oldNetBuffer;
		oldNetBuffer.Write(quitReason.data(), quitReason.size());
		// add null terminator
		oldNetBuffer.Write<uint8_t>(0);
		oldNetBuffer.Reset();
		handler.Handle(serverInstance.GetRef(), client1, oldNetBuffer);
	}

	REQUIRE(fx::GameServerInstance::dropClientData.has_value() == true);
	REQUIRE(fx::GameServerInstance::dropClientData.value().reason == quitReason);
	REQUIRE(fx::GameServerInstance::dropClientData.value().client == client1);
	REQUIRE(fx::GameServerInstance::dropClientData.value().clientDropReason == fx::ClientDropReason::CLIENT);
	REQUIRE(fx::GameServerInstance::dropClientData.value().resourceName == fx::clientDropResourceName);
}
