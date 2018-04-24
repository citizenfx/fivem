/*
* This file is part of FiveM: https://fivem.net/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ServerIdentityProvider.h>

#define STEAM_API_KEY "04246554B51E8C14ED537C55A4BA4CD7"
#define STEAM_APPID 218

// this imports pplxtasks somewhere?
#define _PPLTASK_ASYNC_LOGGING 0

#include <cpr/cpr.h>
#include <json.hpp>

using json = nlohmann::json;

static InitFunction initFunction([]()
{
	static struct SteamIdProvider : public fx::ServerIdentityProviderBase
	{
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

		virtual void RunAuthentication(const std::shared_ptr<fx::Client>& clientPtr, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb) override
		{
			auto it = postMap.find("authTicket");

			if (it == postMap.end())
			{
				cb({});
				return;
			}

			cpr::GetCallback([=](cpr::Response r)
			{
				try
				{
					if (r.error)
					{
						cb({ "Steam authentication failed: " + r.error.message });
					}
					else if (r.status_code >= 400)
					{
						cb({ fmt::sprintf("Steam authentication failed: HTTP %d", r.status_code) });
					}
					else
					{
						json object = json::parse(r.text)["response"];

						if (object.find("error") != object.end())
						{
							cb({ "Steam rejected authentication: " + object["error"]["errordesc"].get<std::string>() });
							return;
						}

						uint64_t steamId = strtoull(object["params"]["steamid"].get<std::string>().c_str(), nullptr, 10);
						clientPtr->AddIdentifier(fmt::sprintf("steam:%015llx", steamId));

						cb({});
					}
				}
				catch (std::exception& e)
				{
					cb({ fmt::sprintf("SteamIdProvider failure: %s", e.what()) });
				}
			}, cpr::Url{fmt::format("https://api.steampowered.com/ISteamUserAuth/AuthenticateUserTicket/v1/?key={0}&appid={1}&ticket={2}", STEAM_API_KEY, STEAM_APPID, it->second)}, cpr::VerifySsl{ false });
		}
	} idp;

	fx::RegisterServerIdentityProvider(&idp);
}, 152);
