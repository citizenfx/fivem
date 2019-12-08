/*
* This file is part of FiveM: https://fivem.net/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ServerIdentityProvider.h>

#include <tbb/concurrent_unordered_map.h>

#include <HttpClient.h>
#include <HttpServer.h>

static InitFunction initFunction([]()
{
	static struct EndPointIdProvider : public fx::ServerIdentityProviderBase
	{
		tbb::concurrent_unordered_map<std::string, bool> allowedByPolicyCache;

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

		virtual void RunAuthentication(const std::shared_ptr<fx::Client>& clientPtr, const fwRefContainer<net::HttpRequest>& request, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb) override
		{
			auto sourceIP = request->GetHeader("X-Cfx-Source-Ip", "");

			if (sourceIP.empty())
			{
				return RunAuthentication(clientPtr, postMap, cb);
			}

			auto doAccept = [sourceIP, clientPtr, cb]()
			{
				auto rSourceIP = sourceIP.substr(0, sourceIP.find_last_of(':'));

				clientPtr->AddIdentifier("ip:" + rSourceIP);
				clientPtr->SetTcpEndPoint(rSourceIP);

				cb({});
			};

			auto clep = clientPtr->GetTcpEndPoint();

			auto it = allowedByPolicyCache.find(clep);

			if (it != allowedByPolicyCache.end() && it->second)
			{
				doAccept();

				return;
			}

			Instance<HttpClient>::Get()->DoPostRequest("https://cfx.re/api/validateSource/?v=1", { { "ip", clep } }, [this, clep, doAccept, clientPtr, cb](bool success, const char* data, size_t size)
			{
				bool allowSourceIP = true;

				if (success)
				{
					std::string result{ data, size };

					if (result != "yes")
					{
						allowSourceIP = false;
					}
					else
					{
						allowedByPolicyCache.insert({ clep, true });
					}
				}

				if (allowSourceIP)
				{
					doAccept();
				}
				else
				{
					const auto& ep = clientPtr->GetTcpEndPoint();
					clientPtr->AddIdentifier("ip:" + ep);

					cb({});
				}
			});
		}
	} idp;

	fx::RegisterServerIdentityProvider(&idp);
}, 150);
