#pragma once

#include "StdInc.h"

#include "ClientRegistry.h"
#include "KeyedRateLimiter.h"
#include "TcpListenManager.h"

struct GetInfoOOB
{
	template <typename ServerImpl>
	static void Process(const fwRefContainer<ServerImpl>& server, const net::PeerAddress& from,
	                    const std::string_view& data)
	{
		const auto limiter = server->GetInstance()->template GetComponent<fx::PeerAddressRateLimiterStore>()->
		                             GetRateLimiter(
			                             "getinfo", fx::RateLimiterDefaults{2.0, 10.0});

		if (!fx::IsProxyAddress(from) && !limiter->Consume(from))
		{
			return;
		}

		const uint32_t amountOfConnectedClients = server->GetInstance()->template GetComponent<fx::ClientRegistry>()->
		                                                  GetAmountOfConnectedClients();
		// reads the request data till a new line and when no new line is present it reads the full request data
		const std::string_view challenge = data.substr(0, data.find_first_of(" \n"));
		const std::string response = R"(infoResponse
\sv_maxclients\)" + server->GetVariable("sv_maxclients") +
			R"(\clients\)" + std::to_string(amountOfConnectedClients) + R"(\challenge\)" + std::string(challenge) +
			R"(\gamename\)" + "CitizenFX" + R"(\protocol\4\hostname\)" + server->GetVariable("sv_hostname") +
			R"(\gametype\)" + server->GetVariable("gametype") + R"(\mapname\)" + server->GetVariable("mapname") +
			R"(\iv\)" + server->GetVariable("sv_infoVersion");
		server->SendOutOfBand(from, response);
	}

	static constexpr const char* GetName()
	{
		return "getinfo";
	}
};
