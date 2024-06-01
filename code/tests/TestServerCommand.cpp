#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ConsoleContextInstance.h"
#include "GameServer.h"
#include "ServerEventComponent.h"
#include "ServerEventComponentInstance.h"
#include "ServerInstance.h"
#include "packethandlers/ServerCommandPacketHandler.h"

#include "TestUtils.h"

TEST_CASE("Server command test")
{
	REQUIRE(ServerCommandPacketHandler::GetPacketId() == "msgServerCommand");

	fx::ServerInstanceBase* serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	serverInstance->SetComponent(new fx::ServerEventComponent());
	// create client with privileges
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");

	seGetCurrentContext()->AddAccessControlEntry(
		se::Principal{fmt::sprintf("player.%d", client->GetNetId())},
		se::Object{"command.testCommand"},
		se::AccessType::Allow
	);

	auto principalScope = client->EnterPrincipalScope();

	ConsoleCommand testCommand(ConsoleContextInstance::Get(), "testCommand",
	                           [=](const std::string& echo)
	                           {
		                           REQUIRE(echo == "test");
		                           console::Printf("console", "echo:" + echo + "\n");
	                           });

	net::Buffer buffer;
	std::string command = "testCommand test";
	buffer.Write<uint16_t>(command.size()); // command length, unused
	buffer.Write(command.data(), command.size());
	buffer.Write<uint32_t>(HashString(command.c_str()));
	buffer.Reset();

	ServerCommandPacketHandler handler(serverInstance);
	handler.Handle(serverInstance, client, buffer);

	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.has_value() == true);
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().eventName == "__cfx_internal:serverPrint");
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc.has_value() == true);
	REQUIRE(
		fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc.value() == std::to_string(client->GetNetId()
		));

	auto msgPackData = msgpack::unpack(
		reinterpret_cast<const char*>(fx::ServerEventComponentInstance::lastClientEvent.value().data.data()),
		fx::ServerEventComponentInstance::lastClientEvent.value().data.size());
	std::stringstream stream;
	msgpack::object_stringize_visitor vis(stream);
	msgpack::object_parser(msgPackData.get()).parse(vis);

	REQUIRE(stream.str() == std::string("[\"") + "echo:test\\n" + "\"]");

	//TODO: add __cfx_internal:commandFallback test after !309

	delete serverInstance;
}

TEST_CASE("Server command not existing test")
{
	REQUIRE(ServerCommandPacketHandler::GetPacketId() == "msgServerCommand");

	fx::ServerInstanceBase* serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	serverInstance->SetComponent(new fx::ServerEventComponent());
	// create client with privileges
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");

	seGetCurrentContext()->AddAccessControlEntry(
		se::Principal{fmt::sprintf("player.%d", client->GetNetId())},
		se::Object{"command.testCommandNotExisting"},
		se::AccessType::Allow
	);

	auto principalScope = client->EnterPrincipalScope();

	net::Buffer buffer;
	std::string command = "testCommandNotExisting";
	buffer.Write<uint16_t>(command.size()); // command length, unused
	buffer.Write(command.data(), command.size());
	buffer.Write<uint32_t>(HashString(command.c_str()));
	buffer.Reset();

	ServerCommandPacketHandler handler(serverInstance);
	handler.Handle(serverInstance, client, buffer);

	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.has_value() == true);
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().eventName == "__cfx_internal:serverPrint");
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc.has_value() == true);
	REQUIRE(
		fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc.value() == std::to_string(client->GetNetId()
		));

	auto msgPackData = msgpack::unpack(
		reinterpret_cast<const char*>(fx::ServerEventComponentInstance::lastClientEvent.value().data.data()),
		fx::ServerEventComponentInstance::lastClientEvent.value().data.size());
	std::stringstream stream;
	msgpack::object_stringize_visitor vis(stream);
	msgpack::object_parser(msgPackData.get()).parse(vis);

	REQUIRE(stream.str() == "[\"\"]");

	// REQUIRE(stream.str() == "[\"Access denied for command testCommand.\n\"]");
	// REQUIRE(stream.str() == "[\"Argument count mismatch (passed 0, wanted 2)\n"\"]");

	//TODO: add __cfx_internal:commandFallback test after !309

	delete serverInstance;
}

TEST_CASE("Server command no access test")
{
	REQUIRE(ServerCommandPacketHandler::GetPacketId() == "msgServerCommand");

	fx::ServerInstanceBase* serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	serverInstance->SetComponent(new fx::ServerEventComponent());
	// create client with privileges
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");

	auto principalScope = client->EnterPrincipalScope();

	ConsoleCommand testCommandNoAccess(ConsoleContextInstance::Get(), "testCommandNoAccess",
	                                   [=](const std::string& echo)
	                                   {
	                                   });

	net::Buffer buffer;
	std::string command = "testCommandNoAccess";
	buffer.Write<uint16_t>(command.size()); // command length, unused
	buffer.Write(command.data(), command.size());
	buffer.Write<uint32_t>(HashString(command.c_str()));
	buffer.Reset();

	ServerCommandPacketHandler handler(serverInstance);
	handler.Handle(serverInstance, client, buffer);

	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.has_value() == true);
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().eventName == "__cfx_internal:serverPrint");
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc.has_value() == true);
	REQUIRE(
		fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc.value() == std::to_string(client->GetNetId()
		));

	auto msgPackData = msgpack::unpack(
		reinterpret_cast<const char*>(fx::ServerEventComponentInstance::lastClientEvent.value().data.data()),
		fx::ServerEventComponentInstance::lastClientEvent.value().data.size());
	std::stringstream stream;
	msgpack::object_stringize_visitor vis(stream);
	msgpack::object_parser(msgPackData.get()).parse(vis);

	REQUIRE(stream.str() == "[\"Access denied for command testCommandNoAccess.\\n\"]");

	//TODO: add __cfx_internal:commandFallback test after !309

	delete serverInstance;
}

TEST_CASE("Server command wrong argument count test")
{
	REQUIRE(ServerCommandPacketHandler::GetPacketId() == "msgServerCommand");

	fx::ServerInstanceBase* serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	serverInstance->SetComponent(new fx::ServerEventComponent());
	// create client with privileges
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");

	auto principalScope = client->EnterPrincipalScope();

	seGetCurrentContext()->AddAccessControlEntry(
		se::Principal{fmt::sprintf("player.%d", client->GetNetId())},
		se::Object{"command.testCommandWith2Args"},
		se::AccessType::Allow
	);

	ConsoleCommand testCommandWith2Args(ConsoleContextInstance::Get(), "testCommandWith2Args",
	                                    [=](const std::string& p1, const std::string& p2)
	                                    {
	                                    });

	net::Buffer buffer;
	std::string command = "testCommandWith2Args";
	buffer.Write<uint16_t>(command.size()); // command length, unused
	buffer.Write(command.data(), command.size());
	buffer.Write<uint32_t>(HashString(command.c_str()));
	buffer.Reset();

	ServerCommandPacketHandler handler(serverInstance);
	handler.Handle(serverInstance, client, buffer);

	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.has_value() == true);
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().eventName == "__cfx_internal:serverPrint");
	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc.has_value() == true);
	REQUIRE(
		fx::ServerEventComponentInstance::lastClientEvent.value().targetSrc.value() == std::to_string(client->GetNetId()
		));

	auto msgPackData = msgpack::unpack(
		reinterpret_cast<const char*>(fx::ServerEventComponentInstance::lastClientEvent.value().data.data()),
		fx::ServerEventComponentInstance::lastClientEvent.value().data.size());
	std::stringstream stream;
	msgpack::object_stringize_visitor vis(stream);
	msgpack::object_parser(msgPackData.get()).parse(vis);

	REQUIRE(stream.str() == "[\"Argument count mismatch (passed 0, wanted 2)\\n\"]");

	//TODO: add __cfx_internal:commandFallback test after !309

	delete serverInstance;
}
