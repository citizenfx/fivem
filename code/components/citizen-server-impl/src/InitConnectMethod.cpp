#include "StdInc.h"
#include <ClientHttpHandler.h>

#include <ClientRegistry.h>

#include <ServerInstanceBase.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceCallbackComponent.h>

#include <boost/random/random_device.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <ServerIdentityProvider.h>

static std::forward_list<fx::ServerIdentityProviderBase*> g_serverProviders;
static std::map<std::string, fx::ServerIdentityProviderBase*> g_providersByType;

namespace fx
{
void RegisterServerIdentityProvider(ServerIdentityProviderBase* provider)
{
	g_serverProviders.push_front(provider);
	g_providersByType.insert({ provider->GetIdentifierPrefix(), provider });
}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		auto minTrustVar = instance->AddVariable<int>("sv_authMinTrust", ConVar_None, 1);
		minTrustVar->GetHelper()->SetConstraints(1, 5);

		auto maxVarianceVar = instance->AddVariable<int>("sv_authMaxVariance", ConVar_None, 5);
		maxVarianceVar->GetHelper()->SetConstraints(1, 5);

		auto shVar = instance->AddVariable<bool>("sv_scriptHookAllowed", ConVar_ServerInfo, false);
		auto ehVar = instance->AddVariable<bool>("sv_enhancedHostSupport", ConVar_ServerInfo, false);

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("initConnect", [=](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const std::function<void(const json&)>& cb)
		{
			auto nameIt = postMap.find("name");
			auto guidIt = postMap.find("guid");

			auto protocolIt = postMap.find("protocol");

			if (nameIt == postMap.end() || guidIt == postMap.end() || protocolIt == postMap.end())
			{
				cb(json::object({ {"error", "fields missing"} }));
				return;
			}

			auto name = nameIt->second;
			auto guid = guidIt->second;

			std::string token = boost::uuids::to_string(boost::uuids::basic_random_generator<boost::random_device>()());

			json json = json::object();
			json["protocol"] = 4;
			json["sH"] = shVar->GetValue();
			json["enhancedHostSupport"] = ehVar->GetValue();
			json["token"] = token;
			json["netlibVersion"] = 2;

			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			auto ra = request->GetRemoteAddress();

			static std::atomic<uint32_t> g_tempIds;
			auto tempId = g_tempIds.fetch_add(1);

			auto client = clientRegistry->MakeClient(guid);
			client->SetName(name);
			client->SetConnectionToken(token);
			client->SetTcpEndPoint(ra.substr(0, ra.find_last_of(':')));
			client->SetNetId(0x10000 + tempId);
			client->Touch();

			auto it = g_serverProviders.begin();

			auto done = [=]()
			{
				int maxTrust = INT_MIN;
				int minVariance = INT_MAX;

				for (const auto& identifier : client->GetIdentifiers())
				{
					std::string idType = identifier.substr(0, identifier.find_first_of(':'));

					auto provider = g_providersByType[idType];
					maxTrust = std::max(provider->GetTrustLevel(), maxTrust);
					minVariance = std::min(provider->GetVarianceLevel(), minVariance);
				}

				if (maxTrust < minTrustVar->GetValue() || minVariance > maxVarianceVar->GetValue())
				{
					cb({ {"error", "You can not join this server due to your identifiers being insufficient. Please try starting Steam or another identity provider and try again."} });
					return;
				}

				auto resman = instance->GetComponent<fx::ResourceManager>();
				auto eventManager = resman->GetComponent<fx::ResourceEventManagerComponent>();
				auto cbComponent = resman->GetComponent<fx::ResourceCallbackComponent>();

				// TODO: replace with event stacks once implemented
				std::string noReason("Resource prevented connection.");

				std::map<std::string, fx::ResourceCallbackComponent::CallbackRef> cbs;
				bool isDeferred = false;

				cbs["defer"] = cbComponent->CreateCallback([&](const msgpack::unpacked& unpacked)
				{
					isDeferred = true;
				});

				cbs["done"] = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
				{
					auto obj = unpacked.get().as<std::vector<msgpack::object>>();

					if (obj.size() == 1)
					{
						cb({ { "error", obj[0].as<std::string>() } });
					}
					else
					{
						cb(json);
					}
				});

				bool shouldAllow = eventManager->TriggerEvent2("playerConnecting", { fmt::sprintf("%d", client->GetNetId()) }, client->GetName(), cbComponent->CreateCallback([&](const msgpack::unpacked& unpacked)
				{
					auto obj = unpacked.get().as<std::vector<msgpack::object>>();

					if (obj.size() == 1)
					{
						noReason = obj[0].as<std::string>();
					}
				}), cbs);

				if (!isDeferred)
				{
					if (!shouldAllow)
					{
						cb({ {"error", noReason} });
						return;
					}

					cb(json);
				}
			};

			// seriously C++?
			auto runOneIdentifier = std::make_shared<std::unique_ptr<std::function<void(decltype(g_serverProviders.begin()))>>>();

			*runOneIdentifier = std::make_unique<std::function<void(decltype(g_serverProviders.begin()))>>([=](auto it)
			{
				if (it == g_serverProviders.end())
				{
					done();
				}
				else
				{
					auto auth = (*it);

					auto thisIt = ++it;

					auth->RunAuthentication(client, postMap, [=](std::optional<std::string> err)
					{
						if (err)
						{
							clientRegistry->RemoveClient(client);

							cb(json::object({ {"error", *err} }));

							return;
						}

						(**runOneIdentifier)(thisIt);
					});
				}
			});

			(**runOneIdentifier)(it);
		});
	}, 50);
});
