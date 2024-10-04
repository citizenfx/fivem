#include <StdInc.h>
#include <random>

#include <Client.h>

#include "KeyedRateLimiter.h"
#include "ResourceEventComponent.h"
#include "ResourceManager.h"
#include "ResourceManagerInstance.h"
#include "ServerInstanceBase.h"

#include <catch_amalgamated.hpp>

#include "ServerInstance.h"
#include "TestUtils.h"
#include "packethandlers/ServerEventPacketHandler.h"

struct ServerEvent
{
	std::string name;
	std::string data;
	std::string source; // net:{netId}

	ServerEvent(const std::string& name, const std::string& data, const std::string& source)
		: name(name),
		  data(data),
		  source(source)
	{
	}
};

namespace
{
	std::optional<ServerEvent> g_serverEvent;
	std::optional<ServerEvent> g_serverEventFromResourceTick;

	fx::ServerInstanceBase* CreateTestServer()
	{
		fx::ServerInstanceBase* serverInstance = ServerInstance::Create();
		serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
		serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
			new fx::ResourceEventManagerComponent());
		fwRefContainer<console::Context> consoleContext;
		CreateContext(console::GetDefaultContext(), &consoleContext);
		serverInstance->SetComponent(consoleContext);
		serverInstance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>()->
		                OnQueueEvent.Connect([](const std::string& eventName, const std::string& eventData,
		                                        const std::string& eventSource)
		                {
			                g_serverEvent.emplace(eventName, eventData, eventSource);
		                });
		serverInstance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>()->
		                OnTriggerEvent.Connect([](const std::string& eventName, const std::string& eventData,
		                                          const std::string& eventSource, bool* eventCanceled)
		                {
			                g_serverEventFromResourceTick.emplace(eventName, eventData, eventSource);
			                // when it return true it will search for a resource to handle the event.
			                // resources can announce themself that they can handle the event with the AddResourceHandledEvent method in ResourceEventManagerComponent
			                // the eventCanceled is modified when the event is forwarded to a resource.
			                // when the resource has the event '*' added via AddResourceHandledEvent it will be triggered for every event. 
			                return true;
		                });
		return serverInstance;
	}

	fx::ServerInstanceBase* g_serverInstance = CreateTestServer();

	fx::ClientSharedPtr CreateClient(const uint32_t netId)
	{
		// guid not relevant for server events
		const fx::ClientSharedPtr client = fx::ClientSharedPtr::Construct("");
		client->SetNetId(netId);
		return client;
	}

	void SendServerEvent(const fx::ClientSharedPtr& client, net::Buffer& buffer)
	{
		g_serverEvent.reset();
		g_serverEventFromResourceTick.reset();
		ServerEventPacketHandler handler(g_serverInstance);
		handler.Handle(g_serverInstance, client, buffer);
		g_serverInstance->GetComponent<fx::ResourceManager>()->Tick();
	}
}

TEST_CASE("Server event")
{
	uint32_t netId = fx::TestUtils::u64Random(10000) + 1;
	const fx::ClientSharedPtr client = CreateClient(netId);
	net::Buffer buffer;
	const std::string name = fx::TestUtils::asciiRandom(20);
	buffer.Write<uint16_t>(name.size() + 1);
	buffer.Write(name.data(), name.size());
	buffer.Write<uint8_t>(0);
	const std::string data = fx::TestUtils::asciiRandom(100);
	buffer.Write(data.data(), data.size());
	buffer.Reset();
	SendServerEvent(client, buffer);
	REQUIRE(g_serverEvent.has_value() == true);
	REQUIRE(g_serverEvent.value().name == name);
	REQUIRE(g_serverEvent.value().data == data);
	REQUIRE(g_serverEvent.value().source == "net:" + std::to_string(netId));
}

TEST_CASE("Server event event name size missmatch")
{
	uint32_t netId = fx::TestUtils::u64Random(10000) + 1;
	const fx::ClientSharedPtr client = CreateClient(netId);
	net::Buffer buffer;
	const std::string name = fx::TestUtils::asciiRandom(20);
	buffer.Write<uint16_t>(1);
	buffer.Write(name.data(), name.size());
	buffer.Write<uint8_t>(0);
	const std::string data = "{}";
	buffer.Write(data.data(), data.size());
	buffer.Reset();
	SendServerEvent(client, buffer);
	REQUIRE(g_serverEvent.has_value() == false);
}

TEST_CASE("Server event with only a name without data")
{
	uint32_t netId = fx::TestUtils::u64Random(10000) + 1;
	const fx::ClientSharedPtr client = CreateClient(netId);
	net::Buffer buffer;
	const std::string name = fx::TestUtils::asciiRandom(20);
	buffer.Write<uint16_t>(name.size() + 1);
	buffer.Write(name.data(), name.size());
	buffer.Write<uint8_t>(0);
	buffer.Reset();
	SendServerEvent(client, buffer);
	REQUIRE(g_serverEvent.has_value() == true);
	REQUIRE(g_serverEvent.value().name == name);
	REQUIRE(g_serverEvent.value().data.empty() == true);
	REQUIRE(g_serverEvent.value().source == "net:" + std::to_string(netId));
}

TEST_CASE("Server event with invalid event name length")
{
	uint32_t netId = fx::TestUtils::u64Random(10000) + 1;
	const fx::ClientSharedPtr client = CreateClient(netId);
	net::Buffer buffer;
	const std::string name = fx::TestUtils::asciiRandom(20);
	uint16_t nameLength = 1;
	buffer.Write<uint16_t>(nameLength + 1);
	buffer.Write(name.data(), name.size());
	buffer.Write<uint8_t>(0);
	const std::string data = "{}";
	buffer.Write(data.data(), data.size());
	buffer.Reset();
	SendServerEvent(client, buffer);
	REQUIRE(g_serverEvent.has_value() == true);
	REQUIRE(g_serverEvent.value().name == name.substr(0, nameLength));
	// the name is now part of the data, because it was only read with the given length, so the data won't be correctly read anymore
	REQUIRE(g_serverEvent.value().data != "{}");
	REQUIRE(g_serverEvent.value().source == "net:" + std::to_string(netId));
}

TEST_CASE("Server event and check that the resource tick triggers the event handler")
{
	uint32_t netId = fx::TestUtils::u64Random(10000) + 1;
	const fx::ClientSharedPtr client = CreateClient(netId);
	net::Buffer buffer;
	const std::string name = fx::TestUtils::asciiRandom(20);
	uint16_t nameLength = name.size();
	buffer.Write<uint16_t>(nameLength + 1);
	buffer.Write(name.data(), name.size());
	buffer.Write<uint8_t>(0);
	const std::string data = "{}";
	buffer.Write(data.data(), data.size());
	buffer.Reset();
	SendServerEvent(client, buffer);
	REQUIRE(g_serverEvent.has_value() == true);
	REQUIRE(g_serverEvent.value().name == name);
	REQUIRE(g_serverEvent.value().data == data);
	REQUIRE(g_serverEvent.value().source == "net:" + std::to_string(netId));
	REQUIRE(g_serverEventFromResourceTick.has_value() == true);
	REQUIRE(g_serverEventFromResourceTick.value().name == name);
	REQUIRE(g_serverEventFromResourceTick.value().data == data);
	REQUIRE(g_serverEventFromResourceTick.value().source == "net:" + std::to_string(netId));
}
