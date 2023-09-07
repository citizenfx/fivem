/*
* This file is part of FiveM: https://fivem.net/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ServerIdentityProvider.h>

#define STEAM_APPID 218

// this imports pplxtasks somewhere?
#define _PPLTASK_ASYNC_LOGGING 0

#include <cpr/cpr.h>
#include <json.hpp>

#include <tbb/concurrent_queue.h>

#include <TcpServerManager.h>
#include <ServerInstanceBase.h>

#include <HttpClient.h>

std::shared_ptr<ConVar<std::string>> g_devotedApiKey;

using json = nlohmann::json;

template<typename Handle, class Class, void(Class::*Callable)()>
void UvCallback(Handle* handle)
{
	(reinterpret_cast<Class*>(handle->data)->*Callable)();
}

static InitFunction initFunction([]()
{
	static fx::ServerInstanceBase* serverInstance;
	
	static struct DevotedIdProvider : public fx::ServerIdentityProviderBase
	{
		HttpClient* httpClient;

		DevotedIdProvider()
		{
			httpClient = new HttpClient();
		}

		virtual std::string GetIdentifierPrefix() override
		{
			return "devoted";
		}

		virtual int GetVarianceLevel() override
		{
			return 1;
		}

		virtual int GetTrustLevel() override
		{
			return 5;
		}

		virtual void RunAuthentication(const fx::ClientSharedPtr& clientPtr, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb) override
		{
			auto dsKey = postMap.find("ds");

			if (g_devotedApiKey->GetValue().empty())
			{
				trace("A client has tried to authenticate using Steam, but `steam_webApiKey` is unset. Please set steam_webApiKey to a Steam Web API key as registered on "
					"Valve's ^4https://steamcommunity.com/dev/apikey^7 web page. Steam identifier information will be unavailable otherwise.\n");

				trace("To suppress this message, `set steam_webApiKey \"none\"`.\n");

				cb({});
				return;
			}

			if (g_devotedApiKey->GetValue() == "none")
			{
				cb({});
				return;
			}

			auto devotedStudiosKey = dsKey->second;
			clientPtr->AddIdentifier(this->GetIdentifierPrefix() + ":" + devotedStudiosKey);
			cb({});
		}
	} idp;

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_devotedApiKey = instance->AddVariable<std::string>("devotedStudiosKey", ConVar_None, "");

		serverInstance = instance;
	});

	fx::RegisterServerIdentityProvider(&idp);
}, 153);
