#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ClientRegistry.h"
#include "ServerEventComponent.h"
#include "ServerEventComponentInstance.h"
#include "ServerInstance.h"

TEST_CASE("onPlayerJoining event test")
{
	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	const fx::ClientSharedPtr entityClient = client;
	const int slotId = 127;
	auto sec = fx::ServerEventComponentInstance::Create();
	sec->TriggerClientEvent("onPlayerJoining", fmt::sprintf("%d", client->GetNetId()), entityClient->GetNetId(), entityClient->GetName(), slotId);	
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.has_value() == true);
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().eventName == "onPlayerJoining");
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc == std::to_string(client->GetNetId()));
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().data.size() == 8);
}
