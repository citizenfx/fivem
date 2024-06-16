#pragma once

#include "StdInc.h"

#include "ClientRegistry.h"
#include "KeyedRateLimiter.h"
#include "PrintListener.h"
#include "TcpListenManager.h"

class RconOutOfBand
{
public:
	template <typename ServerImpl>
	RconOutOfBand(const fwRefContainer<ServerImpl>& server)
	{
	}

	template <typename ServerImpl>
	void Process(const fwRefContainer<ServerImpl>& server, const net::PeerAddress& from,
	             const std::string_view& dataView)
	{
		auto limiter = server->GetInstance()->template GetComponent<fx::PeerAddressRateLimiterStore>()->GetRateLimiter(
			"rcon", fx::RateLimiterDefaults{0.2, 5.0});

		if (!fx::IsProxyAddress(from) && !limiter->Consume(from))
		{
			return;
		}

		const int spacePos = dataView.find_first_of(" \n");
		if (spacePos == 0 || spacePos == std::string::npos || spacePos + 1 >= dataView.size())
		{
			// ignore packets that doesn't contain a command or password
			return;
		}

		const std::string_view passwordView = dataView.substr(0, spacePos);
		const std::string_view commandView = dataView.substr(spacePos);

		if (server->GetRconPassword().empty())
		{
			static const char* response = "print The server must set rcon_password to be able to use this command.\n";
			server->SendOutOfBand(from, response);
			return;
		}

		if (passwordView != server->GetRconPassword())
		{
			static const char* response = "print Invalid password.\n";
			server->SendOutOfBand(from, response);
			return;
		}

		// reset rate limit for this client, because the command is authorized
		limiter->Reset(from);

		std::string command(commandView);

		gscomms_execute_callback_on_main_thread([server, from, command = std::move(command)]()
		{
			try
			{
				std::string printString;

				fx::ScopeDestructor destructor([&]()
				{
					server->SendOutOfBand(from, "print " + printString);
				});

				// log rcon request
				console::Printf("rcon", "Rcon from %s\n%s\n", from.ToString(), command);

				fx::PrintListenerContext context([&printString](std::string_view print)
				{
					printString += print;
				});

				fx::PrintFilterContext filterContext([](ConsoleChannel& channel, std::string_view print)
				{
					channel = "rcon/" + channel;
				});

				auto ctx = server->GetInstance()->template GetComponent<console::Context>();
				ctx->ExecuteBuffer();

				se::ScopedPrincipal principalScope(se::Principal{"system.console"});
				ctx->AddToBuffer(command);
				ctx->ExecuteBuffer();
			}
			catch (std::exception& e)
			{
			}
		});
	}

	static constexpr const char* GetName()
	{
		return "rcon";
	}
};
