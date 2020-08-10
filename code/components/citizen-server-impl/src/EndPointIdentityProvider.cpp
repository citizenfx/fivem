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

#define FOLLY_NO_CONFIG

#ifdef _WIN32
#undef ssize_t
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/types.h>
#endif

#include <folly/IPAddress.h>
#include <folly/String.h>

#include <boost/algorithm/string.hpp>

#include <CoreConsole.h>
#include <IteratorView.h>

static InitFunction initFunction([]()
{
	static ConVar<std::string> allowedIpCidr("sv_proxyIPRanges", ConVar_None, "10.0.0.0/8 127.0.0.0/8 192.168.0.0/16 172.16.0.0/12");

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

		virtual void RunAuthentication(const fx::ClientSharedPtr& clientPtr, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb) override
		{
			const auto& ep = clientPtr->GetTcpEndPoint();
			clientPtr->AddIdentifier("ip:" + ep);

			cb({});
		}

		void RunRealIPAuthentication(const fx::ClientSharedPtr& clientPtr, const fwRefContainer<net::HttpRequest>& request, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb, const std::string& realIP)
		{
			auto ipCidrList = allowedIpCidr.GetValue();
			bool found = false;

			const auto& ep = clientPtr->GetTcpEndPoint();

			auto ipAddress = folly::IPAddress::tryFromString(ep);

			if (ipAddress)
			{
				for (auto item :
					fx::GetIteratorView(
						std::make_pair(
							boost::algorithm::make_split_iterator(
								ipCidrList,
								boost::algorithm::token_finder(
									boost::algorithm::is_space(),
									boost::algorithm::token_compress_on
								)
							),
							boost::algorithm::split_iterator<std::string::iterator>()
						)
					)
				)
				{
					auto network = folly::IPAddress::tryCreateNetwork(folly::range(&*item.begin(), &*item.end()));

					if (network)
					{
						if (ipAddress->inSubnet(network->first, network->second))
						{
							found = true;
							break;
						}
					}
				}
			}

			if (!found)
			{
				clientPtr->AddIdentifier("ip:" + ep);
			}
			else
			{
				clientPtr->AddIdentifier("ip:" + realIP);
				clientPtr->SetTcpEndPoint(realIP);
			}

			cb({});
		}

		virtual void RunAuthentication(const fx::ClientSharedPtr& clientPtr, const fwRefContainer<net::HttpRequest>& request, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb) override
		{
			auto sourceIP = request->GetHeader("X-Cfx-Source-Ip", "");
			auto realIP = request->GetHeader("X-Real-Ip", "");

			if (sourceIP.empty() && realIP.empty())
			{
				return RunAuthentication(clientPtr, postMap, cb);
			}

			if (!realIP.empty())
			{
				return RunRealIPAuthentication(clientPtr, request, postMap, cb, realIP);
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
