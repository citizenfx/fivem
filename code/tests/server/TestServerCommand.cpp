#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ByteWriter.h"
#include "ConsoleContextInstance.h"
#include "GameServer.h"
#include "ServerEventComponent.h"
#include "ServerEventComponentInstance.h"
#include "ServerInstance.h"
#include "ResourceEventComponent.h"
#include "ResourceManager.h"
#include "ResourceManagerInstance.h"
#include "ServerCommand.h"
#include "packethandlers/ServerCommandPacketHandler.h"

#include "TestUtils.h"

TEST_CASE("Server command test")
{
	REQUIRE(std::string(ServerCommandPacketHandler::GetPacketId()) == "msgServerCommand");

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
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

	ConsoleCommand testCommand(serverInstance->GetComponent<console::Context>().GetRef(), "testCommand",
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

	ServerCommandPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

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
}

TEST_CASE("Server command not existing test")
{
	REQUIRE(std::string(ServerCommandPacketHandler::GetPacketId()) == "msgServerCommand");

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
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

	ServerCommandPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

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
}

TEST_CASE("Server command no access test")
{
	REQUIRE(std::string(ServerCommandPacketHandler::GetPacketId()) == "msgServerCommand");

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	serverInstance->SetComponent(new fx::ServerEventComponent());
	// create client with privileges
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");

	auto principalScope = client->EnterPrincipalScope();

	ConsoleCommand testCommandNoAccess(serverInstance->GetComponent<console::Context>().GetRef(), "testCommandNoAccess",
	                                   [=](const std::string& echo)
	                                   {
	                                   });

	net::Buffer buffer;
	std::string command = "testCommandNoAccess";
	buffer.Write<uint16_t>(command.size()); // command length, unused
	buffer.Write(command.data(), command.size());
	buffer.Write<uint32_t>(HashString(command.c_str()));
	buffer.Reset();

	ServerCommandPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

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
}

TEST_CASE("Server command wrong argument count test")
{
	REQUIRE(std::string(ServerCommandPacketHandler::GetPacketId()) == "msgServerCommand");

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
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

	ConsoleCommand testCommandWith2Args(serverInstance->GetComponent<console::Context>().GetRef(), "testCommandWith2Args",
	                                    [=](const std::string& p1, const std::string& p2)
	                                    {
	                                    });

	net::Buffer buffer;
	std::string command = "testCommandWith2Args";
	buffer.Write<uint16_t>(command.size()); // command length, unused
	buffer.Write(command.data(), command.size());
	buffer.Write<uint32_t>(HashString(command.c_str()));
	buffer.Reset();

	ServerCommandPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

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
}

TEST_CASE("Server command packet test")
{
	REQUIRE(std::string(ServerCommandPacketHandler::GetPacketId()) == "msgServerCommand");
	REQUIRE(net::SerializableComponent::GetSize<net::packet::ClientServerCommand>() == 4102);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
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

	ConsoleCommand testCommand(serverInstance->GetComponent<console::Context>().GetRef(), "testCommand",
							   [=](const std::string& echo)
							   {
								   REQUIRE(echo == "test");
								   console::Printf("console", "echo:" + echo + "\n");
							   });

	std::string commandName = "testCommand test";

	net::packet::ClientServerCommand clientServerCommand;
	clientServerCommand.command = commandName;
	net::Buffer buffer (net::SerializableComponent::GetSize<net::packet::ClientServerCommand>());
	net::ByteWriter writer {buffer.GetBuffer(), net::SerializableComponent::GetSize<net::packet::ClientServerCommand>()};
	REQUIRE(clientServerCommand.Process(writer) == true);

	fx::ServerEventComponentInstance::lastClientEvent.reset();
	
	ServerCommandPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

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
}

TEST_CASE("Server command empty string test")
{
	REQUIRE(std::string(ServerCommandPacketHandler::GetPacketId()) == "msgServerCommand");
	REQUIRE(net::SerializableComponent::GetSize<net::packet::ClientServerCommand>() == 4102);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	serverInstance->SetComponent(new fx::ServerEventComponent());
	// create client with privileges
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");

	fx::ServerEventComponentInstance::lastClientEvent.reset();
	
	net::Buffer buffer;
	ServerCommandPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.has_value() == false);
}

TEST_CASE("Server command invalid length test")
{
	REQUIRE(std::string(ServerCommandPacketHandler::GetPacketId()) == "msgServerCommand");
	REQUIRE(net::SerializableComponent::GetSize<net::packet::ClientServerCommand>() == 4102);

	fwRefContainer<fx::ServerInstanceBase> serverInstance = ServerInstance::Create();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ResourceManager>(fx::ResourceManagerInstance::Create());
	serverInstance->GetComponent<fx::ResourceManager>()->SetComponent<fx::ResourceEventManagerComponent>(
		new fx::ResourceEventManagerComponent());
	serverInstance->SetComponent<console::Context>(ConsoleContextInstance::Get());
	serverInstance->SetComponent(new fx::ServerEventComponent());
	// create client with privileges
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");

	fx::ServerEventComponentInstance::lastClientEvent.reset();
	
	net::Buffer buffer;
	buffer.Write<uint16_t>(1024);
	ServerCommandPacketHandler handler(serverInstance.GetRef());
	handler.Handle(serverInstance.GetRef(), client, buffer);

	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.has_value() == false);

	fx::ServerEventComponentInstance::lastClientEvent.reset();

	std::vector<uint8_t> commandBuffer (4097);
	fx::TestUtils::fillVectorU8Random(commandBuffer);
	
	net::Buffer buffer2;
	buffer2.Write<uint16_t>(4097);
	buffer.Write(commandBuffer.data(), commandBuffer.size());
	buffer.Write<uint32_t>(0);
	ServerCommandPacketHandler handler2(serverInstance.GetRef());
	handler2.Handle(serverInstance.GetRef(), client, buffer2);

	REQUIRE(fx::ServerEventComponentInstance::lastClientEvent.has_value() == false);
}
