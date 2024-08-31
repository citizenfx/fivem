#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "GameServer.h"
#include "GameServerInstance.h"
#include "HeHost.h"
#include "ServerInstance.h"

#include "TestUtils.h"
#include "packethandlers/HeHostPacketHandler.h"

TEST_CASE("HeHost test")
{
	fx::SetOneSyncGetCallback([] { return false; });

	REQUIRE(std::string(fx::ServerDecorators::HeHostPacketHandler::GetPacketId()) == std::string("msgHeHost"));
	REQUIRE(HashRageString(fx::ServerDecorators::HeHostPacketHandler::GetPacketId()) == 0x86e9f87b);
	// test is only implemented for disabled onesync
	REQUIRE(fx::IsOneSync() == false);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::GameServer>(fx::GameServerInstance::Create());
	serverInstance->SetComponent(new fx::ServerDecorators::HostVoteCount());

	net::packet::ClientHeHost clientHeHost;
	clientHeHost.baseNum = fx::TestUtils::u64Random(UINT32_MAX);
	// vote for client 1 to become the new host
	clientHeHost.allegedNewId = 1;

	std::vector<uint8_t> packetBuffer (net::SerializableComponent::GetSize<net::packet::ClientHeHost>());
	net::ByteWriter writer {packetBuffer.data(), packetBuffer.size()};

	REQUIRE(clientHeHost.Process(writer) == true);

	// create clients with routed bool true so they count against total voting clients
	
	const fx::ClientSharedPtr client1 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	REQUIRE(client1->GetNetId() == 1);
	client1->SetHasRouted();

	const fx::ClientSharedPtr client2 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test2");
	REQUIRE(client2->GetNetId() == 2);
	client2->SetHasRouted();

	const fx::ClientSharedPtr client3 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test3");
	REQUIRE(client3->GetNetId() == 3);
	client3->SetHasRouted();

	const fx::ClientSharedPtr noneVotingClient4 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test4");
	REQUIRE(noneVotingClient4->GetNetId() == 4);
	noneVotingClient4->SetHasRouted();

	const fx::ClientSharedPtr noneVotingClient5 = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test5");
	REQUIRE(noneVotingClient5->GetNetId() == 5);
	noneVotingClient4->SetHasRouted();

	// 3 votes are required when 5 clients are connected
	REQUIRE(0.6 * 5 == 3);
	
	fx::GameServerInstance::broadcastData.reset();
	
	fx::ServerDecorators::HeHostPacketHandler handler(serverInstance.GetRef());
	net::Buffer buffer {packetBuffer.data(), writer.GetOffset()};

	// client 1 vote for 1
	handler.Handle(serverInstance.GetRef(), client1, buffer);
	buffer.Reset();
	REQUIRE(serverInstance->GetComponent<fx::ServerDecorators::HostVoteCount>()->voteCounts.size() == 1);
	// should be one, but it is two votes, because the first vote is count twice
	REQUIRE(serverInstance->GetComponent<fx::ServerDecorators::HostVoteCount>()->voteCounts[1] == 2);
	
	// client 2 vote for 1 with old net buffer to ensure compatibility
	net::Buffer oldBuffer;
	oldBuffer.Write<uint32_t>(clientHeHost.allegedNewId);
	oldBuffer.Write<uint32_t>(clientHeHost.baseNum);
	oldBuffer.Reset();
	handler.Handle(serverInstance.GetRef(), client2, oldBuffer);

	// count is 0, because it gets cleared after voice was successfully
	REQUIRE(serverInstance->GetComponent<fx::ServerDecorators::HostVoteCount>()->voteCounts.size() == 0);
	
	// client 3 vote for 1
	// but only 2 are actually required, because the HeHost code counts the first vote twice
	//handler.Handle(serverInstance, client3, buffer);
	//REQUIRE(serverInstance->GetComponent<fx::ServerDecorators::HostVoteCount>()->voteCounts.size() == 1);

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
	REQUIRE(base == clientHeHost.baseNum);
}
