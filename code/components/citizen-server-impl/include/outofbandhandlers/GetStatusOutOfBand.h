#pragma once

#include "StdInc.h"

#include "ClientRegistry.h"
#include "KeyedRateLimiter.h"
#include "TcpListenManager.h"

struct GetStatusOutOfBand
{
	template <typename ServerImpl>
	static void Process(const fwRefContainer<ServerImpl>& server, const net::PeerAddress& from, const std::string_view& data)
	{
		const auto limiter = server->GetInstance()->template GetComponent<fx::PeerAddressRateLimiterStore>()->GetRateLimiter("getstatus", fx::RateLimiterDefaults{ 1.0, 5.0 });

		if (!fx::IsProxyAddress(from) && !limiter->Consume(from))
		{
			return;
		}

		const uint32_t numClients = server->GetInstance()->template GetComponent<fx::ClientRegistry>()->GetAmountOfConnectedClients();
		// std::string is 30 times faster then stringstream
		std::stringstream clientList;

		server->GetInstance()->template GetComponent<fx::ClientRegistry>()->ForAllClients([&](const fx::ClientSharedPtr& client)
		{
			if (client->GetNetId() < 0xFFFF)
			{
				clientList << fmt::sprintf("%d %d \"%s\"\n", 0, 0, client->GetName());
			}
		});

		std::stringstream infoVars;

		auto addInfo = [&](const std::string& key, const std::string& value)
		{
			infoVars << "\\" << key << "\\" << value;
		};

		addInfo("sv_maxclients", server->GetVariable("sv_maxclients"));
		addInfo("clients", std::to_string(numClients));

		server->GetInstance()->template GetComponent<console::Context>()->GetVariableManager()->ForAllVariables([&](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& var)
		{
			addInfo(name, var->GetValue());
		}, ConVar_ServerInfo);

		server->SendOutOfBand(from, fmt::format(
			"statusResponse\n"
			"{0}\n"
			"{1}",
			infoVars.str(),
			clientList.str()
		));
	}

	static constexpr const char* GetName()
	{
		return "getstatus";
	}
};
