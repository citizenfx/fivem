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
			return 4;
		}

		virtual void RunAuthentication(const fx::ClientSharedPtr& clientPtr, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb) override
		{
			auto jsonAny = clientPtr->GetData("entitlementJson");

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

			cb({});
		}
	} idp;

	fx::RegisterServerIdentityProvider(&idp);
}, 151);
