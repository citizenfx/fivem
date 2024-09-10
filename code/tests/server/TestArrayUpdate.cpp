#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <StateBagComponent.h>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "ConsoleContextInstance.h"
#include "GameServer.h"
#include "ServerGameStatePublicInstance.h"
#include "ServerInstance.h"
#include "TestUtils.h"
#include "packethandlers/ArrayUpdatePacketHandler.h"
#include "state/ServerGameStatePublic.h"

TEST_CASE("Array update test")
{
	const bool usePacket = GENERATE(true, false);
	fx::SetOneSyncGetCallback([] { return true; });

	REQUIRE(std::string(fx::ServerDecorators::ArrayUpdatePacketHandler::GetPacketId()) == "msgArrayUpdate");
	REQUIRE(HashRageString(fx::ServerDecorators::ArrayUpdatePacketHandler::GetPacketId()) == 0x976e783);
	// test is only implemented for onesync
	REQUIRE(fx::IsOneSync() == true);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ServerGameStatePublic>(fx::ServerGameStatePublicInstance::Create());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());

	uint8_t testHandler = 1;
	uint16_t testIndex = fx::TestUtils::u64Random(49);
	std::vector<uint8_t> elementData (128);
	fx::TestUtils::fillVectorU8Random(elementData);

	net::Buffer buffer;

	if (usePacket)
	{
		net::packet::ClientArrayUpdate clientArrayUpdate;
		clientArrayUpdate.handler = testHandler;
		clientArrayUpdate.index = testIndex;
		clientArrayUpdate.data = {elementData.data(), elementData.size()};

		std::vector<uint8_t> packetBuffer (net::SerializableComponent::GetSize<net::packet::ClientArrayUpdate>());
		net::ByteWriter writer {packetBuffer.data(), packetBuffer.size()};

		REQUIRE(clientArrayUpdate.Process(writer) == true);
		buffer.Write(packetBuffer.data(), packetBuffer.size());
	}
	else
	{
		buffer.Write<uint8_t>(testHandler);
		buffer.Write<uint16_t>(testIndex);
		buffer.Write<uint16_t>(elementData.size());
		buffer.Write(elementData.data(), elementData.size());
	}

	buffer.Reset();

	fx::ServerGameStatePublicInstance::GetArrayUpdateLastCall().reset();
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	fx::ServerDecorators::ArrayUpdatePacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

	REQUIRE(fx::ServerGameStatePublicInstance::GetArrayUpdateLastCall().has_value() == true);
	REQUIRE(fx::ServerGameStatePublicInstance::GetArrayUpdateLastCall().value().client == client);
	REQUIRE(fx::ServerGameStatePublicInstance::GetArrayUpdateLastCall().value().handler == testHandler);
	REQUIRE(fx::ServerGameStatePublicInstance::GetArrayUpdateLastCall().value().index == testIndex);
	REQUIRE(fx::ServerGameStatePublicInstance::GetArrayUpdateLastCall().value().data == elementData);
}
