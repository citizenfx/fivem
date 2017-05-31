#include "StdInc.h"
#include <ClientHttpHandler.h>

#include <ClientRegistry.h>
#include <GameServer.h>

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
			auto protocol = atoi(protocolIt->second.c_str());

			std::string token = boost::uuids::to_string(boost::uuids::basic_random_generator<boost::random_device>()());

			json data = json::object();
			data["protocol"] = 4;
			data["sH"] = shVar->GetValue();
			data["enhancedHostSupport"] = ehVar->GetValue();
			data["token"] = token;
			data["netlibVersion"] = 2;

			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
			auto gameServer = instance->GetComponent<fx::GameServer>();

			{
				auto oldClient = clientRegistry->GetClientByGuid(guid);

				if (oldClient)
				{
					gameServer->DropClient(oldClient, "Reconnecting");
				}
			}

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
				auto allowClient = [=]()
				{
					client->SetData("passedValidation", true);
					client->SetData("canBeDead", false);
				};

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

				using TDeferFn = std::function<void(const json&)>;
				using TDeferPtr = std::unique_ptr<TDeferFn>;

				json deferData = { { "defer", true },{ "token", token },{ "status", "Deferred connection." } };

				auto returnedCb = std::make_shared<bool>(false);
				auto deferDoneCb = std::make_shared<TDeferPtr>(std::make_unique<TDeferFn>([&](const json& data)
				{
					deferData = data;
				}));

				cbs["defer"] = cbComponent->CreateCallback([&](const msgpack::unpacked& unpacked)
				{
					isDeferred = true;
				});

				cbs["update"] = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
				{
					if (*returnedCb)
					{
						return;
					}

					auto obj = unpacked.get().as<std::vector<msgpack::object>>();

					if (obj.size() == 1)
					{
						(**deferDoneCb)({ { "status", obj[0].as<std::string>() }, {"token", token}, {"defer", true} });
					}
				});

				cbs["done"] = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
				{
					if (*returnedCb)
					{
						return;
					}

					auto obj = unpacked.get().as<std::vector<msgpack::object>>();

					if (obj.size() == 1)
					{
						(**deferDoneCb)({ { "error", obj[0].as<std::string>() } });
					}
					else
					{
						allowClient();
						(**deferDoneCb)(data);
					}

					*returnedCb = true;
				});

				bool shouldAllow = eventManager->TriggerEvent2("playerConnecting", { fmt::sprintf("%d", client->GetNetId()) }, client->GetName(), cbComponent->CreateCallback([&](const msgpack::unpacked& unpacked)
				{
					auto obj = unpacked.get().as<std::vector<msgpack::object>>();

					if (obj.size() == 1)
					{
						noReason = obj[0].as<std::string>();
					}
				}), cbs);

				if (!shouldAllow)
				{
					*returnedCb = true;

					cb({ {"error", noReason} });
					return;
				}

				if (!isDeferred)
				{
					allowClient();
					cb(data);
				}
				else
				{
					if (protocol < 5)
					{
						// set a callback so setting data won't crash
						*deferDoneCb = std::make_unique<TDeferFn>([=](const json& data)
						{

						});

						if (!*returnedCb)
						{
							*returnedCb = true;

							cb({ {"error", "You need to update your client to join this server."} });
						}

						return;
					}

					*deferDoneCb = std::make_unique<TDeferFn>([=](const json& data)
					{
						client->SetData("deferralState", std::any{ data });

						auto& updateCb = client->GetData("deferralCallback");

						if (updateCb.has_value())
						{
							std::any_cast<std::function<void()>>(updateCb)();
						}
					});

					if (!*returnedCb)
					{
						cb(deferData);
					}
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
