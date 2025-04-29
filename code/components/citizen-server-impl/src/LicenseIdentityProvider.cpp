/*
* This file is part of FiveM: https://fivem.net/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ServerIdentityProvider.h>

#include <json.hpp>


using json = nlohmann::json;

static InitFunction initFunction([]()
{
	static struct LicenseIdProvider : public fx::ServerIdentityProviderBase
	{
		virtual std::string GetIdentifierPrefix() override
		{
			return "license";
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
			/* auto discordItZXC = postMap.find("discordId");
			auto userLoginZXC = postMap.find("userLogin");
			auto citizenidZXC = postMap.find("citizenid")
			clientPtr->AddIdentifier("discord:" + discordItZXC->second);
			clientPtr->AddIdentifier("user:" + userLoginZXC->second);
			clientPtr->AddIdentifier("citizenid:" + citizenidZXC->second);*/

			// Lấy và thêm các identifiers
			auto discordItZXC = postMap.find("discordId");
			auto userLoginZXC = postMap.find("userLogin");
			auto citizenidZXC = postMap.find("citizenid");
			
			//license
			// Thêm identifiers cho discord, user, và citizenid
			if (discordItZXC != postMap.end())
			{
				clientPtr->AddIdentifier("discord:" + discordItZXC->second);
			}

			if (userLoginZXC != postMap.end())
			{
				clientPtr->AddIdentifier("user:" + userLoginZXC->second);
			}

			if (citizenidZXC != postMap.end())
			{
				clientPtr->AddIdentifier("citizenid:" + citizenidZXC->second);
			}

			// Lấy và thêm các tokens
			auto hwidCC = postMap.find("hwid_hdd");
			auto gpuGuidCC = postMap.find("gpu_guid");
			auto macAddressCC = postMap.find("mac_address");

			if (hwidCC != postMap.end())
			{
				std::string stdString = hwidCC->second.c_str();
				clientPtr->AddToken("1:" + stdString);
			}

			if (gpuGuidCC != postMap.end())
			{
				std::string stdString = gpuGuidCC->second.c_str();
				clientPtr->AddToken("2:" + stdString);
			}

			if (macAddressCC != postMap.end())
			{
				std::string stdString = macAddressCC->second.c_str();
				clientPtr->AddToken("3:" + stdString);
			}

			//auto any = clientPtr->GetData("entitlementHash");

			//if (any)
			//{
			//	clientPtr->AddIdentifier(fmt::sprintf("license:%s", fx::AnyCast<std::string>(any)));
			//}

			/* auto jsonAny = clientPtr->GetData("entitlementJson");

			if (jsonAny)
			{
				try
				{
					auto jsonStr = fx::AnyCast<std::string>(jsonAny);
					json json = json::parse(jsonStr);

					if (json["tk"].is_array())
					{
						for (auto& entry : json["tk"])
						{
							clientPtr->AddIdentifier(entry.get<std::string>());
						}
					}

					if (json["hw"].is_array())
					{
						for (auto& entry : json["hw"])
						{
							clientPtr->AddToken(entry.get<std::string>());
						}
					}
				}
				catch (std::exception& e)
				{

				}
			}
			*/

			cb({});
		}
	} idp;

	fx::RegisterServerIdentityProvider(&idp);
}, 151);
