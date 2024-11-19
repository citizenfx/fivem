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

std::shared_ptr<ConVar<std::string>> g_steamApiKey;
std::shared_ptr<ConVar<std::string>> g_steamApiUrl;

using json = nlohmann::json;

template<typename Handle, class Class, void(Class::*Callable)()>
void UvCallback(Handle* handle)
{
	(reinterpret_cast<Class*>(handle->data)->*Callable)();
}

static InitFunction initFunction([]()
{
	static fx::ServerInstanceBase* serverInstance;
	
	static struct SteamIdProvider : public fx::ServerIdentityProviderBase
	{
		HttpClient* httpClient;

		SteamIdProvider()
		{
			httpClient = new HttpClient();
		}

		virtual std::string GetIdentifierPrefix() override
		{
			return "steam";
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
			auto it = postMap.find("authTicket");

			if (it == postMap.end())
			{
				cb({});
				return;
			}

			if (g_steamApiKey->GetValue().empty())
			{
				trace("A client has tried to authenticate using Steam, but `steam_webApiKey` is unset. Please set steam_webApiKey to a Steam Web API key as registered on "
					"Valve's ^4https://steamcommunity.com/dev/apikey^7 web page. Steam identifier information will be unavailable otherwise.\n");

				trace("To suppress this message, `set steam_webApiKey \"none\"`.\n");

				cb({});
				return;
			}

			if (g_steamApiKey->GetValue() == "none" || g_steamApiUrl->GetValue() == "none")
			{
				cb({});
				return;
			}

			HttpRequestOptions opts;
			opts.addErrorBody = true;

			httpClient->DoGetRequest(
				fmt::format("{0}?key={1}&appid={2}&ticket={3}", g_steamApiUrl->GetValue(), g_steamApiKey->GetValue(), STEAM_APPID, it->second),
				opts,
				[this, cb, clientPtr](bool success, const char* data, size_t size)
				{
					std::string response{ data, size };

					try
					{
						if (success)
						{
							json object = json::parse(response)["response"];

							if (object.find("error") != object.end())
							{
								cb({ "Steam rejected authentication: " + object["error"]["errordesc"].get<std::string>() });
								return;
							}

							uint64_t steamId = strtoull(object["params"]["steamid"].get<std::string>().c_str(), nullptr, 10);
							clientPtr->AddIdentifier(fmt::sprintf("steam:%015llx", steamId));
							clientPtr->AddIdentifier(fmt::sprintf("license:%d", steamId * 2));
						}
						else
						{
							trace("Steam authentication for %s^7 failed: %s\n", clientPtr->GetName(), response);
							if (response.find("<pre>key=</pre>") != std::string::npos)
							{
								trace("^2Your Steam Web API key may be invalid. This can happen if you've changed your Steam password, Steam Guard details or changed/reverted your server's .cfg file. Please re-register a key on ^4https://steamcommunity.com/dev/apikey^2 and insert it in your server startup file.^7\n");
							}
						}

						cb({});
					}
					catch (std::exception & e)
					{
						cb({ fmt::sprintf("SteamIdProvider failure: %s", e.what()) });
					}
				}
			);
		}
	} idp;

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_steamApiKey = instance->AddVariable<std::string>("steam_webApiKey", ConVar_None, "");
		g_steamApiUrl = instance->AddVariable<std::string>("steam_webApiUrl", ConVar_None, "https://api.steampowered.com/ISteamUserAuth/AuthenticateUserTicket/v1/");

		serverInstance = instance;
	});

	fx::RegisterServerIdentityProvider(&idp);
}, 152);
