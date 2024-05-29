#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "GameServer.h"
#include "KeyedRateLimiter.h"
#include "ResourceEventComponent.h"
#include "ResourceManager.h"

#include "packethandlers/ServerCommandPacketHandler.h"

#include "PrintListener.h"
#include "ServerEventComponent.h"

ServerCommandPacketHandler::ServerCommandPacketHandler(fx::ServerInstanceBase* instance)
{
	instance->GetComponent<console::Context>()->GetCommandManager()->FallbackEvent.Connect(
		[this, instance](const std::string& commandName, const ProgramArguments& arguments, const std::string& context)
		{
			auto resourceManager = instance->GetComponent<fx::ResourceManager>();
			if (!context.empty())
			{
				auto eventComponent = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

				try
				{
					return eventComponent->TriggerEvent2("__cfx_internal:commandFallback", {"internal-net:" + context},
					                                     rawCommand);
				}
				catch (std::bad_any_cast& e)
				{
					trace("caught bad_any_cast in FallbackEvent handler for %s\n", commandName);
				}
			}

			return true;
		}, 99999);

	instance->GetComponent<console::Context>()->GetCommandManager()->FallbackEvent.Connect(
		[instance](const std::string& commandName, const ProgramArguments& arguments, const std::string& context)
		{
			auto resourceManager = instance->GetComponent<fx::ResourceManager>();
			auto eventComponent = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

			// assert privilege
			if (!seCheckPrivilege(fmt::sprintf("command.%s", commandName)))
			{
				return true;
			}

			// if canceled, the command was handled, so cancel the fwEvent
			return eventComponent->TriggerEvent2("rconCommand", {}, commandName, arguments.GetArguments());
		}, -100);
}

void ServerCommandPacketHandler::Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
                                        net::Buffer& buffer)
{
	static fx::RateLimiterStore<uint32_t, false> netEventRateLimiterStore{
		instance->GetComponent<console::Context>().GetRef()
	};
	static auto netEventRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netCommand", fx::RateLimiterDefaults{7.f, 14.f});
	static auto netFloodRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netCommandFlood", fx::RateLimiterDefaults{25.f, 45.f});
	static auto netSizeRateLimiter = netEventRateLimiterStore.GetRateLimiter(
		"netCommandSize", fx::RateLimiterDefaults{1 * 1024.0, 8 * 1024.0});

	const uint32_t netId = client->GetNetId();

	if (!netEventRateLimiter->Consume(netId))
	{
		if (!netFloodRateLimiter->Consume(netId))
		{
			instance->GetComponent<fx::GameServer>()->DropClient(client, "Reliable server command overflow.");
		}

		return;
	}

	// contains the command length
	// kept for compatibility reasons, can be removed in future net version
	buffer.Read<uint16_t>();

	if (!netSizeRateLimiter->Consume(netId, static_cast<double>(buffer.GetRemainingBytes())))
	{
		const auto cmdLen = buffer.GetRemainingBytes();
		std::string command;
		if (cmdLen)
		{
			command.resize(cmdLen);
			buffer.Read(command.data(), cmdLen);
		}
		// if this happens, try increasing rateLimiter_netCommandSize_rate and rateLimiter_netCommandSize_burst
		// preferably, fix client scripts to not have this large a set of server commands with high frequency
		instance->GetComponent<fx::GameServer>()->DropClient(client, "Reliable server command size overflow: %s",
		                                                     command);

		return;
	}

	const auto cmdLen = buffer.GetRemainingBytes();

	if (cmdLen == 0)
	{
		// skip empty commands
		return;
	}

	std::string commandName = std::string(buffer.Read<std::string_view>(cmdLen));

	gscomms_execute_callback_on_main_thread([this, instance, client, commandName = std::move(commandName)]
	{
		// save the raw command for fallback usage inside the static variable
		rawCommand = commandName;

		std::string printString;

		fx::PrintListenerContext context([&printString](std::string_view print)
		{
			printString += print;
		});

		fx::PrintFilterContext filterContext([&client](ConsoleChannel& channel, std::string_view print)
		{
			channel = fmt::sprintf("forward:%d/%s", client->GetNetId(), channel);
		});

		fx::ScopeDestructor destructor([&]()
		{
			msgpack::sbuffer sb;

			msgpack::packer<msgpack::sbuffer> packer(sb);
			packer.pack_array(1).pack(printString);

			instance->GetComponent<fx::ServerEventComponent>()->TriggerClientEvent(
				"__cfx_internal:serverPrint", sb.data(), sb.size(), {std::to_string(client->GetNetId())});
		});

		// invoke
		auto consoleCxt = instance->GetComponent<console::Context>();
		consoleCxt->GetCommandManager()->Invoke(rawCommand, std::to_string(client->GetNetId()));

		// unset raw command
		rawCommand = "";
	});
}
