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

#include <botan/base64.h>
#include <botan/sha160.h>
#include <botan/pubkey.h>
#include <botan/rsa.h>
#include <botan/ber_dec.h>

#include <ctime>

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

static bool VerifyTicket(const std::string& guid, const std::string& ticket)
{
	auto ticketData = Botan::base64_decode(ticket);

	// validate ticket length
	if (ticketData.size() < 20 + 4 + 128)
	{
		return false;
	}

	uint32_t length = *(uint32_t*)&ticketData[0];

	if (length != 16)
	{
		return false;
	}

	uint64_t ticketGuid = *(uint64_t*)&ticketData[4];
	uint64_t ticketExpiry = *(uint64_t*)&ticketData[12];

	// check expiration

	// get UTC time
	std::time_t timeVal;
	std::time(&timeVal);

	std::tm* tm = std::gmtime(&timeVal);

	std::time_t utcTime = std::mktime(tm);

	// verify
	if (ticketExpiry < utcTime)
	{
		trace("Connecting player: ticket expired\n");
		return false;
	}

	// check the GUID
	uint64_t realGuid = std::stoull(guid);

	if (realGuid != ticketGuid)
	{
		trace("Connecting player: ticket GUID not matching\n");
		return false;
	}

	// check the RSA signature
	uint32_t sigLength = *(uint32_t*)&ticketData[length + 4];

	if (sigLength != 128)
	{
		return false;
	}

	Botan::SHA_160 hashFunction;
	auto result = hashFunction.process(&ticketData[4], length);

	std::vector<uint8_t> msg(result.size() + 1);
	msg[0] = 2;
	memcpy(&msg[1], &result[0], result.size());

	auto modulus = Botan::base64_decode("1DNT1go22VUAU3BON+jCfXxs7Ow9Zxwng4ARTX/vrv6I65bsSYbdBrcc"
										"w/50Fu7AJr8zy8+sXK8wUO4gx00frtA0adaGeZOeBqNq7/K3Gprv98wc"
										"ftbxWjUv75pVl9Ush5yxpBPbuYUnGR/Nh2+K3GRrIrKxWYpNSF1JZYzE"
										"+5k=");

	auto exponent = Botan::base64_decode("AQAB");

	Botan::BigInt n(modulus.data(), modulus.size());
	Botan::BigInt e(exponent.data(), exponent.size());

	auto pk = Botan::RSA_PublicKey(n, e);

	auto signer = std::make_unique<Botan::PK_Verifier>(pk, "EMSA_PKCS1(SHA-1)");

	bool valid = signer->verify_message(msg.data(), msg.size(), &ticketData[length + 4 + 4], sigLength);

	if (!valid)
	{
		trace("Connecting player: ticket RSA signature not matching\n");
		return false;
	}

	return true;
}

struct TicketData
{
	std::optional<std::array<uint8_t, 20>> entitlementHash;
};

static std::optional<TicketData> VerifyTicketEx(const std::string& ticket)
{
	auto ticketData = Botan::base64_decode(ticket);

	// validate ticket length
	if (ticketData.size() < 20 + 4 + 128 + 4)
	{
		return {};
	}

	uint32_t length = *(uint32_t*)&ticketData[20 + 4 + 128];

	// validate full length
	if (ticketData.size() < 20 + 4 + 128 + 4 + length)
	{
		return {};
	}

	// copy extra data
	std::vector<uint8_t> extraData(length);

	if (!extraData.empty())
	{
		memcpy(&extraData[0], &ticketData[20 + 4 + 128 + 4], length);
	}

	// check the RSA signature
	uint32_t sigLength = *(uint32_t*)&ticketData[20 + 4 + 128 + 4 + length];

	if (sigLength != 128)
	{
		return {};
	}

	Botan::SHA_160 hashFunction;
	auto result = hashFunction.process(&ticketData[4], ticketData.size() - 128 - 4 - 4);

	std::vector<uint8_t> msg(result.size() + 1);
	msg[0] = 2;
	memcpy(&msg[1], &result[0], result.size());

	auto modulus = Botan::base64_decode("1DNT1go22VUAU3BON+jCfXxs7Ow9Zxwng4ARTX/vrv6I65bsSYbdBrcc"
		"w/50Fu7AJr8zy8+sXK8wUO4gx00frtA0adaGeZOeBqNq7/K3Gprv98wc"
		"ftbxWjUv75pVl9Ush5yxpBPbuYUnGR/Nh2+K3GRrIrKxWYpNSF1JZYzE"
		"+5k=");

	auto exponent = Botan::base64_decode("AQAB");

	Botan::BigInt n(modulus.data(), modulus.size());
	Botan::BigInt e(exponent.data(), exponent.size());

	auto pk = Botan::RSA_PublicKey(n, e);

	auto signer = std::make_unique<Botan::PK_Verifier>(pk, "EMSA_PKCS1(SHA-1)");

	bool valid = signer->verify_message(msg.data(), msg.size(), &ticketData[length + 4 + 4 + 128 + 20 + 4], sigLength);

	if (!valid)
	{
		trace("Connecting player: ticket RSA signature not matching\n");
		return {};
	}

	TicketData outData;

	if (length >= 20)
	{
		std::array<uint8_t, 20> entitlementHash;
		memcpy(entitlementHash.data(), &extraData[0], entitlementHash.size());

		outData.entitlementHash = entitlementHash;
	}

	return outData;
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

		auto lanVar = instance->AddVariable<bool>("sv_lan", ConVar_ServerInfo, false);

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

			TicketData ticketData;

			if (!lanVar->GetValue())
			{
				auto ticketIt = postMap.find("cfxTicket");

				if (ticketIt == postMap.end())
				{
					cb(json::object({ { "error", "No FiveM ticket was specified. If this is an offline server, maybe set sv_lan?"} }));
					return;
				}

				if (!VerifyTicket(guid, ticketIt->second))
				{
					cb(json::object({ { "error", "FiveM ticket authorization failed." } }));
					return;
				}

				auto optionalTicket = VerifyTicketEx(ticketIt->second);

				if (!optionalTicket)
				{
					cb(json::object({ { "error", "FiveM ticket authorization failed. (2)" } }));
					return;
				}

				ticketData = *optionalTicket;
			}

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

			// add the entitlement hash if needed
			if (ticketData.entitlementHash)
			{
				auto& hash = *ticketData.entitlementHash;
				client->SetData("entitlementHash", fmt::sprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
					hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]));
			}

			client->Touch();

			auto it = g_serverProviders.begin();

			auto done = [=]()
			{
				std::weak_ptr<fx::Client> clientWeak(client);

				auto allowClient = [=]()
				{
					if (!clientWeak.expired())
					{
						auto client = clientWeak.lock();

						client->SetData("passedValidation", true);
						client->SetData("canBeDead", false);
					}
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
					clientRegistry->RemoveClient(client);

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
						if (!clientWeak.expired())
						{
							clientRegistry->RemoveClient(clientWeak.lock());
						}

						(**deferDoneCb)({ { "error", obj[0].as<std::string>() } });
					}
					else
					{
						allowClient();
						(**deferDoneCb)(data);
					}

					*returnedCb = true;
				});

				bool shouldAllow = eventManager->TriggerEvent2("playerConnecting", { fmt::sprintf("net:%d", client->GetNetId()) }, client->GetName(), cbComponent->CreateCallback([&](const msgpack::unpacked& unpacked)
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
					*deferDoneCb = nullptr;

					clientRegistry->RemoveClient(client);

					cb({ {"error", noReason} });
					return;
				}

				if (!isDeferred)
				{
					allowClient();
					cb(data);

					*deferDoneCb = nullptr;
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

							clientRegistry->RemoveClient(client);

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

					// unset the callback
					*runOneIdentifier = nullptr;
				}
				else
				{
					auto auth = (*it);

					auto thisIt = ++it;

					auth->RunAuthentication(client, postMap, [=](boost::optional<std::string> err)
					{
						if (err)
						{
							clientRegistry->RemoveClient(client);

							cb(json::object({ { "error", *err } }));

							// unset the callback
							*runOneIdentifier = nullptr;

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
