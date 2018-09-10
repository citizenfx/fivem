/*
* This file is part of FiveM: https://fivem.net/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ServerIdentityProvider.h>

static InitFunction initFunction([]()
{
	static struct EndPointIdProvider : public fx::ServerIdentityProviderBase
	{
		virtual std::string GetIdentifierPrefix() override
		{
			return "ip";
		}

		virtual int GetVarianceLevel() override
		{
			return 5;
		}

		virtual int GetTrustLevel() override
		{
			return 5;
		}

		virtual void RunAuthentication(const std::shared_ptr<fx::Client>& clientPtr, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb) override
		{
			const auto& ep = clientPtr->GetTcpEndPoint();
			clientPtr->AddIdentifier("ip:" + ep);

			cb({});
		}
	} idp;

	fx::RegisterServerIdentityProvider(&idp);
}, 150);
