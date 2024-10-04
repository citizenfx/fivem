#pragma once

#include "StdInc.h"

#include "ClientRegistry.h"
#include "KeyedRateLimiter.h"
#include "TcpListenManager.h"

class GetStatusOutOfBand
{
	std::shared_ptr<ConVar<bool>> m_returnClientsListInGetStatus;
	std::shared_ptr<ConVar<bool>> m_enableGetStatus;

public:
	template <typename ServerImpl>
	GetStatusOutOfBand(const fwRefContainer<ServerImpl>& server)
	{
		m_returnClientsListInGetStatus = server->GetInstance()->template AddVariable<bool>("sv_returnClientsListInGetStatus", ConVar_None, true);
		m_enableGetStatus = server->GetInstance()->template AddVariable<bool>("sv_enableGetStatus", ConVar_None, false);
	}

	template <typename ServerImpl>
	void Process(const fwRefContainer<ServerImpl>& server, const net::PeerAddress& from, const std::string_view& data)
	{
		if (!m_enableGetStatus->GetValue())
		{
			return;
		}
		
		const auto limiter = server->GetInstance()->template GetComponent<fx::PeerAddressRateLimiterStore>()->
		                             GetRateLimiter("getstatus", fx::RateLimiterDefaults{1.0, 5.0});

		if (!fx::IsProxyAddress(from) && !limiter->Consume(from))
		{
			return;
		}

		const uint32_t numClients = server->GetInstance()->template GetComponent<fx::ClientRegistry>()->
		                                    GetAmountOfConnectedClients();

		std::string response = "statusResponse\n";

		auto addInfo = [&](const std::string& key, const std::string& value)
		{
			response += "\\" + key + "\\" + value;
		};

		addInfo("sv_maxclients", server->GetVariable("sv_maxclients"));
		addInfo("clients", std::to_string(numClients));

		response += "\n";

		if (m_returnClientsListInGetStatus->GetValue())
		{
			server->GetInstance()->template GetComponent<fx::ClientRegistry>()->ForAllClients(
				[&](const fx::ClientSharedPtr& client)
				{
					if (client->HasConnected())
					{
						response += "0 0 \"" + client->GetName() + "\"\n";
					}
				});
		}

		server->GetInstance()->template GetComponent<console::Context>()->GetVariableManager()->ForAllVariables(
			[&](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& var)
			{
				addInfo(name, var->GetValue());
			}, ConVar_ServerInfo);

		server->SendOutOfBand(from, std::move(response));
	}

	static constexpr const char* GetName()
	{
		return "getstatus";
	}
};
