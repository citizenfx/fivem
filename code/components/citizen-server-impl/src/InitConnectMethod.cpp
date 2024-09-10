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

#include <IteratorView.h>

#include <botan/base64.h>
#include <botan/sha160.h>
#include <botan/pubkey.h>
#include <botan/rsa.h>
#include <botan/ber_dec.h>

#include <ctime>

#include <ClientDeferral.h>

#include <ServerIdentityProvider.h>

#include <MonoThreadAttachment.h>
#include <GameBuilds.h>
#include <CrossBuildRuntime.h>
#include <PoolSizesState.h>

#include <json.hpp>

#define FOLLY_NO_CONFIG

#ifdef _WIN32
#undef ssize_t
#else
#include <sys/types.h>
#endif

#include <folly/String.h>
#include <boost/algorithm/string.hpp>

#include <utf8.h>

#include "NetBitVersion.h"

#include <HttpClient.h>

#include "CnlEndpoint.h"

using json = nlohmann::json;

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

namespace
{
std::mutex g_publicKeyMutex;
std::optional<Botan::RSA_PublicKey> g_publicKey {};
std::chrono::milliseconds g_publicKeyCreation {};

/// <summary>
/// Expires the public key.
/// </summary>
/// <returns>Returns true if the key is expired, false if the key creation was less then 5 minutes ago.</returns>
bool ExpirePublicKey()
{
	std::unique_lock lock(g_publicKeyMutex);
	// only expire the public key if the key is older than 5 minutes
	if (msec() - g_publicKeyCreation > std::chrono::minutes(5))
	{
		g_publicKey.reset();
		return true;
	}

	return false;
}

/// <summary>
/// Requests the public key. If the key is already present, the function returns the key. Otherwise, the key is requested from the server.
/// </summary>
/// <returns>Returns an optional RSA public key.</returns>
std::optional<Botan::RSA_PublicKey> GetPublicKey()
{
	static std::condition_variable requestCv;
	static std::atomic requestInProgress(false);

	// lock token access
	std::unique_lock lock(g_publicKeyMutex);

	if (g_publicKey.has_value())
	{
		// token already present
		return g_publicKey;
	}

	// only one request at the same time
	bool notInProgress = false;
	if (requestInProgress.compare_exchange_strong(notInProgress, true))
	{
		// http get request for loading the token
		Instance<HttpClient>::Get()->DoGetRequest(CNL_ENDPOINT "api/ticket/pubkey", [](bool success, const char* data, size_t length) {
			// lock to synchronize threads awaiting the token
			bool inProgress = true;
			if (!requestInProgress.compare_exchange_strong(inProgress, false))
			{
				// already finished, invalid state should not happen
				return;
			}

			// lock token access
			std::unique_lock lock(g_publicKeyMutex);

			if (success)
			{
				success = false;
				try
				{
					json jsonData = json::parse(data, data + length);
					if (jsonData.contains("key") && jsonData["key"].is_string())
					{
						auto publicKeyData = Botan::base64_decode(jsonData["key"]);

						Botan::BigInt n, e;
						Botan::BER_Decoder(publicKeyData)
							.start_cons(Botan::SEQUENCE)
							.decode(n)
							.decode(e)
							.end_cons();

						g_publicKey = Botan::RSA_PublicKey(n, e);
						g_publicKeyCreation = msec();
						success = true;
					}
				}
				catch (std::exception& e)
				{
					trace("exception while processing public key information call: %s\n", e.what());
				}
			}

			if (!success)
			{
				g_publicKey.reset();
			}

			requestCv.notify_all();
		});
	}

	// unlock token mutex and wait till request is done
	requestCv.wait(lock, [] { return !requestInProgress; });
	return g_publicKey;
}

std::mutex g_ticketMapMutex;
std::unordered_set<std::tuple<uint64_t, uint64_t>> g_ticketList;
std::chrono::milliseconds g_nextTicketGc;

enum class VerifyTicketResult
{
	InvalidLength,
	InvalidLength2,
	Expired,
	MismatchingGUID,
	Reused,
	InvalidSignatureLength,
	InvalidSignature,
	Success,
};

std::string GetVerifyTicketErrorString(VerifyTicketResult result)
{
	switch (result)
	{
		case VerifyTicketResult::InvalidLength:
			return "Invalid ticket length.";
		case VerifyTicketResult::InvalidLength2:
			return "Invalid ticket length. (2)";
		case VerifyTicketResult::Expired:
			return "Ticket expired. Please check your server's system time.";
		case VerifyTicketResult::MismatchingGUID:
			return "Mismatching GUID.";
		case VerifyTicketResult::Reused:
			return "Reused ticket.";
		case VerifyTicketResult::InvalidSignatureLength:
			return "Invalid signature length.";
		case VerifyTicketResult::InvalidSignature:
			return "Invalid ticket signature.";
		case VerifyTicketResult::Success:
			return "";
	}

	return "";
}

VerifyTicketResult VerifyTicket(const std::string& guid, const std::string& ticket, const Botan::RSA_PublicKey& pk)
{
	auto ticketData = Botan::base64_decode(ticket);

	// validate ticket length
	if (ticketData.size() < 20 + 4 + 128)
	{
		return VerifyTicketResult::InvalidLength;
	}

	uint32_t length = *(uint32_t*)&ticketData[0];

	if (length != 16)
	{
		return VerifyTicketResult::InvalidLength2;
	}

	uint64_t ticketGuid = *(uint64_t*)&ticketData[4];
	uint64_t ticketExpiry = *(uint64_t*)&ticketData[12];

	// check expiration

	// get UTC time
	std::time_t timeVal;
	std::time(&timeVal);

	// verify
	if (ticketExpiry < timeVal)
	{
		console::DPrintf("server", "Connecting player: ticket expired\n");
		return VerifyTicketResult::Expired;
	}

	// check the GUID
	uint64_t realGuid = strtoull(guid.c_str(), nullptr, 10);

	if (realGuid != ticketGuid)
	{
		console::DPrintf("server", "Connecting player: ticket GUID not matching\n");
		return VerifyTicketResult::MismatchingGUID;
	}

	{
		std::unique_lock<std::mutex> _(g_ticketMapMutex);

		if (g_ticketList.find({ ticketExpiry, ticketGuid }) != g_ticketList.end())
		{
			return VerifyTicketResult::Reused;
		}

		if (msec() > g_nextTicketGc)
		{
			g_ticketList.clear();
			g_nextTicketGc = msec() + std::chrono::minutes(30);
		}

		g_ticketList.insert({ ticketExpiry, ticketGuid });
	}

	// check the RSA signature
	uint32_t sigLength = *(uint32_t*)&ticketData[length + 4];

	if (sigLength != 128)
	{
		return VerifyTicketResult::InvalidSignatureLength;
	}

	Botan::SHA_160 hashFunction;
	auto result = hashFunction.process(&ticketData[4], length);

	std::vector<uint8_t> msg(result.size() + 1);
	msg[0] = 2;
	memcpy(&msg[1], &result[0], result.size());

	auto signer = std::make_unique<Botan::PK_Verifier>(pk, "EMSA_PKCS1(SHA-1)");

	bool valid = signer->verify_message(msg.data(), msg.size(), &ticketData[length + 4 + 4], sigLength);

	if (!valid)
	{
		return VerifyTicketResult::InvalidSignature;
	}

	return VerifyTicketResult::Success;
}

struct TicketData
{
	std::optional<std::array<uint8_t, 20>> entitlementHash;
	std::optional<std::string> extraJson;
};

std::optional<TicketData> VerifyTicketEx(const std::string& ticket, const Botan::RSA_PublicKey& pk)
{
	auto ticketData = Botan::base64_decode(ticket);

	// validate ticket length
	if (ticketData.size() < 20 + 4 + 128 + 4)
	{
		return {};
	}

	size_t length = static_cast<size_t>(*(uint32_t*)&ticketData[20 + 4 + 128]);

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

	auto signer = std::make_unique<Botan::PK_Verifier>(pk, "EMSA_PKCS1(SHA-1)");

	bool valid = signer->verify_message(msg.data(), msg.size(), &ticketData[length + 4 + 4 + 128 + 20 + 4], sigLength);

	if (!valid)
	{
		console::DPrintf("server", "Connecting player: ticket RSA signature not matching\n");
		return {};
	}

	TicketData outData;

	if (length >= 20)
	{
		std::array<uint8_t, 20> entitlementHash;
		memcpy(entitlementHash.data(), &extraData[0], entitlementHash.size());

		outData.entitlementHash = entitlementHash;
	}

	if (length >= 24)
	{
		uint32_t extraJsonLength = *(uint32_t*)&extraData[20];

		if (length >= (24 + extraJsonLength))
		{
			outData.extraJson = std::string{ (char*)&extraData[24], extraJsonLength };
		}
	}

	return outData;
}
}

extern std::shared_ptr<ConVar<bool>> g_oneSyncVar;
fx::GameBuild g_enforcedGameBuild;

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		auto minTrustVar = instance->AddVariable<int>("sv_authMinTrust", ConVar_None, 1);
		minTrustVar->GetHelper()->SetConstraints(1, 5);

		auto maxVarianceVar = instance->AddVariable<int>("sv_authMaxVariance", ConVar_None, 5);
		maxVarianceVar->GetHelper()->SetConstraints(1, 5);

		auto pureVar = instance->AddVariable<int>("sv_pureLevel", ConVar_ServerInfo, 0);
		auto shVar = instance->AddVariable<bool>("sv_scriptHookAllowed", ConVar_ServerInfo, false);
		auto ehVar = instance->AddVariable<bool>("sv_enhancedHostSupport", ConVar_ServerInfo, false);

		// list of space-separated endpoints that can but don't have to include a port
		// for example: sv_endpoints "123.123.123.123 124.124.124.124"
		auto srvEndpoints = instance->AddVariable<std::string>("sv_endpoints", ConVar_None, "");
		auto lanVar = instance->AddVariable<bool>("sv_lan", ConVar_ServerInfo, false);

		g_enforcedGameBuild = "1604";
		auto enforceGameBuildVar = instance->AddVariable<fx::GameBuild>("sv_enforceGameBuild", ConVar_ReadOnly | ConVar_ServerInfo, "1604", &g_enforcedGameBuild);

		auto poolSizesIncrease = std::make_shared<std::unordered_map<std::string, uint32_t>>();
		auto poolSizesIncreaseVar = instance->AddVariable<std::string>("sv_poolSizesIncrease", ConVar_ServerInfo | ConVar_Internal, "");
		auto poolSizesIncreaseCmd = instance->AddCommand("increase_pool_size", [instance, poolSizesIncreaseVar, poolSizesIncrease](const std::string& poolName, int sizeIncrease)
		{
			static fx::GameName previousTitle = fx::GameName::GTA5;

			fx::GameName gameName = instance->GetComponent<fx::GameServer>()->GetGameName();

			if (!fx::PoolSizeManager::LimitsLoaded() || previousTitle != gameName)
			{
				previousTitle = gameName;

				std::string limitsFileUrl = "https://content.cfx.re/mirrors/client/pool-size-limits/";
				limitsFileUrl += gameName == fx::GameName::GTA5 ? "fivem.json" : "redm.json";

				fx::PoolSizeManager::FetchLimits(limitsFileUrl, true);
			}

			auto validationError = fx::PoolSizeManager::Validate(poolName, sizeIncrease);
			if (validationError.has_value())
			{
				trace("Requested pool size increase is invalid: %s\n", validationError.value());
				return;
			}

			(*poolSizesIncrease)[poolName] = sizeIncrease;
			// Set server variable value. It will automatically be set to client as part of connection response data.
			poolSizesIncreaseVar->GetHelper()->SetRawValue(nlohmann::json(*poolSizesIncrease).dump());
		});

		instance->GetComponent<fx::GameServer>()->OnTick.Connect([instance, enforceGameBuildVar]()
		{
			if (instance->GetComponent<fx::GameServer>()->GetGameName() == fx::GameName::RDR3)
			{
				if (g_enforcedGameBuild == "1604")
				{
					enforceGameBuildVar->GetHelper()->SetRawValue("1311");
				}
			}

			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			clientRegistry->ForAllClients([](const fx::ClientSharedPtr& client)
			{
				auto deferralAny = client->GetData("deferralPtr");

				if (deferralAny)
				{
					auto weakDeferral = fx::AnyCast<std::weak_ptr<fx::ClientDeferral>>(deferralAny);

					if (!weakDeferral.expired())
					{
						client->Touch();
					}
				}
			});
		});

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("getEndpoints", [instance, srvEndpoints](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const std::function<void(const json&)>& cb)
		{
			auto sendError = [=](const std::string& error)
			{
				cb(json::object({ { "error", error } }));
				cb(json(nullptr));
			};

			auto tokenIt = postMap.find("token");

			if (tokenIt == postMap.end())
			{
				sendError("fields missing");
				return;
			}

			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
			auto client = clientRegistry->GetClientByConnectionToken(tokenIt->second);
			if (!client)
			{
				cb(false);
			}
			else
			{
				auto endpointList = srvEndpoints->GetValue();
				if (endpointList.empty()) 
				{
					cb(json::array());
				}
				else 
				{
					json endpoints;
					for (auto item :
						fx::GetIteratorView(
							std::make_pair(
								boost::algorithm::make_split_iterator(
									endpointList,
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
						auto endpoint = folly::range(&*item.begin(), &*item.end());
						endpoints += endpoint;
					}
					cb(endpoints);
				}
			}

			cb(json(nullptr));
		});

		auto experimentalStateBagsHandler = instance->AddVariable<bool>("sv_experimentalStateBagsHandler", ConVar_None, false);
		auto experimentalOneSyncPopulation = instance->AddVariable<bool>("sv_experimentalOneSyncPopulation", ConVar_None, false);
		// todo: remove fx::ServerGameState::GetGameEventHandler, fx::ServerGameState::GetHandler and fx::ServerGameState::GetRequestControlEventHandler when experimentalNetEvents is enabled by default and no longer a experiment
		auto experimentalNetEvents = instance->AddVariable<bool>("sv_experimentalNetGameEventHandler", ConVar_None, false);

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("initConnect", [=](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const std::function<void(const json&)>& cb)
		{
			auto sendError = [=](const std::string& error)
			{
				cb(json::object({ { "error", error } }));
				cb(json(nullptr));
			};

			auto gameServer = instance->GetComponent<fx::GameServer>();

			if (!gameServer->HasSettled())
			{
				sendError("The server is starting up.");
				return;
			}

			auto nameIt = postMap.find("name");
			auto guidIt = postMap.find("guid");
			auto gameBuildIt = postMap.find("gameBuild");
			auto gameNameIt = postMap.find("gameName");

			auto protocolIt = postMap.find("protocol");

			if (nameIt == postMap.end() || guidIt == postMap.end() || protocolIt == postMap.end())
			{
				sendError("fields missing");
				return;
			}

			auto name = nameIt->second;
			auto guid = guidIt->second;
			auto protocol = atoi(protocolIt->second.c_str());
			auto gameBuildField = (gameBuildIt != postMap.end()) ? gameBuildIt->second : "0";
			auto gameName = (gameNameIt != postMap.end()) ? gameNameIt->second : "";

			if (protocol < 12)
			{
				sendError("Client/server version mismatch. Restart your game client to update. If that did not help, "
					"this server is using too new a version for the current game client.");
				return;
			}

			// verify game name
			bool validGameName = false;
			std::string intendedGameName;

			switch (instance->GetComponent<fx::GameServer>()->GetGameName())
			{
			case fx::GameName::GTA4:
				intendedGameName = "gta4";

				if (gameName == "gta4")
				{
					validGameName = true;
				}
				break;
			case fx::GameName::GTA5:
				intendedGameName = "gta5";

				if (gameName.empty() || gameName == "gta5")
				{
					validGameName = true;
				}
				break;
			case fx::GameName::RDR3:
				intendedGameName = "rdr3";

				if (gameName == "rdr3")
				{
					validGameName = true;
				}
				break;
			}

			if (!validGameName)
			{
				sendError(fmt::sprintf("Client/Server game mismatch: %s/%s", gameName, intendedGameName));
				return;
			}

			// limit name length
			if (name.length() >= 200)
			{
				name = name.substr(0, 200);
			}

			// replace invalid UTF8 sequences in name
			{
				std::string validName;

				try
				{
					utf8::replace_invalid(name.begin(), name.end(), std::back_inserter(validName));
				}
				catch (std::exception& e)
				{
					sendError("Parsing name failed.");
					return;
				}

				name = validName;
			}

			TicketData ticketData;

			if (!lanVar->GetValue())
			{
				auto ticketIt = postMap.find("cfxTicket2");

				if (ticketIt == postMap.end())
				{
					sendError("No authentication ticket was specified.");
					return;
				}

				auto requestedPublicKey = GetPublicKey();
				
				if (!requestedPublicKey)
				{
					sendError("public key request failed.");
					return;
				}

				try
				{
					VerifyTicketResult verifyResult = VerifyTicket(guid, ticketIt->second, requestedPublicKey.value());
					
					if (verifyResult == VerifyTicketResult::InvalidSignature)
					{
						// expire the public key if the signature is wrong in case the key got rotated
						if (ExpirePublicKey())
						{
							// request a new key if the expiry was successful
							requestedPublicKey = GetPublicKey();
				
							if (!requestedPublicKey)
							{
								sendError("public key request failed (2).");
								return;
							}

							verifyResult = VerifyTicket(guid, ticketIt->second, requestedPublicKey.value());
						}
					}

					if (verifyResult != VerifyTicketResult::Success)
					{
						sendError(fmt::sprintf("Ticket authorization failed. %s", GetVerifyTicketErrorString(verifyResult)));
						return;
					}

					auto optionalTicket = VerifyTicketEx(ticketIt->second, requestedPublicKey.value());

					if (!optionalTicket)
					{
						sendError("Ticket authorization failed. (2)");
						return;
					}

					ticketData = *optionalTicket;
				}
				catch (const std::exception& e)
				{
					sendError(fmt::sprintf("Parsing error while verifying ticket. %s", e.what()));
					return;
				}
			}

			std::string token = boost::uuids::to_string(boost::uuids::basic_random_generator<boost::random_device>()());

			json data = json::object();
			data["protocol"] = 5;
			
			if (experimentalNetEvents->GetValue())
			{
				data["bitVersion"] = net::NetBitVersion::netVersion4;
			}
			else if (experimentalOneSyncPopulation->GetValue())
			{
				data["bitVersion"] = net::NetBitVersion::netVersion3;
			}
			else if (experimentalStateBagsHandler->GetValue())
			{
				data["bitVersion"] = net::NetBitVersion::netVersion2;
			}
			else
			{
				data["bitVersion"] = net::NetBitVersion::netVersion1;
			}

			data["pure"] = pureVar->GetValue();
			data["sH"] = shVar->GetValue();
			data["enhancedHostSupport"] = ehVar->GetValue() && !fx::IsOneSync();
			data["onesync"] = fx::IsOneSync();
			data["onesync_big"] = fx::IsBigMode();
			data["onesync_lh"] = fx::IsLengthHack();
			if (experimentalOneSyncPopulation->GetValue())
			{
				data["onesync_population"] = fx::IsOneSyncPopulation();
			}

			data["token"] = token;
			data["gamename"] = gameName;

			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			data["netlibVersion"] = gameServer->GetNetLibVersion();
			data["maxClients"] = atoi(gameServer->GetVariable("sv_maxclients").c_str());

			// This expression is noop and should never be true.
			// Capture poolSizesIncreaseCmd just to prolong it's lifetime until connection is initialized.
			// Otherwise the command will be destroyed too soon and won't be available for the server to use.
			if (poolSizesIncreaseCmd == nullptr)
			{
				trace("Something went wrong. Pool sizes increase may not be set.");
			}

			{
				auto oldClient = clientRegistry->GetClientByGuid(guid);

				if (oldClient)
				{
					gameServer->DropClientWithReason(oldClient, fx::serverDropResourceName, fx::ClientDropReason::CLIENT_REPLACED, "Reconnecting");
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

			bool gameNameMatch = false;

			if (ticketData.extraJson)
			{
				try
				{
					json json = json::parse(*ticketData.extraJson);

					if (json["gn"].is_string())
					{
						auto sentGameName = json["gn"].get<std::string>();

						if (sentGameName == intendedGameName)
						{
							gameNameMatch = true;
						}
					}
				}
				catch (std::exception& e)
				{

				}

				client->SetData("entitlementJson", *ticketData.extraJson);
			}

			if (lanVar->GetValue())
			{
				gameNameMatch = true;
			}

			if (!gameNameMatch)
			{
				sendError("CitizenFX ticket authorization failed. (3)");
				return;
			}

			client->Touch();

			auto it = g_serverProviders.begin();

			fx::ClientWeakPtr clientWeak{ client };

			struct ClientHolder
			{
				explicit ClientHolder(fx::ClientRegistry* clientRegistry, fx::ClientWeakPtr client)
					: clientRegistry(clientRegistry), client(client)
				{

				}

				void Disown()
				{
					client = {};
				}

				~ClientHolder()
				{
					if (client)
					{
						if (auto clientPtr = client.lock())
						{
							clientRegistry->RemoveClient(clientPtr);
						}
					}
				}

			private:
				fx::ClientRegistry* clientRegistry;

				fx::ClientWeakPtr client;
			};

			auto clientHolder = std::make_shared<ClientHolder>(clientRegistry.GetRef(), clientWeak);

			auto done = [=]()
			{
				auto lockedClient = clientWeak.lock();
				if (!lockedClient)
				{
					return;
				}

				auto didSucceed = std::make_shared<bool>(false);
				auto weakSuccess = std::weak_ptr<bool>(didSucceed);

				auto allowClient = [=]()
				{
					clientHolder->Disown();

					auto client = clientWeak.lock();

					if (client)
					{
						client->SetData("deferralPtr", nullptr);
						client->SetData("passedValidation", true);
						client->SetData("canBeDead", false);
					}

					auto success = weakSuccess.lock();

					if (success)
					{
						*success = true;
					}
				};

				auto addData = [](json& data, const std::shared_ptr<fx::ClientDeferral>& deferrals)
				{
					json handoverData = json::object();

					for (const auto& [ key, value ] : deferrals->GetHandoverData())
					{
						std::vector<char> handoverValue;
						utf8::replace_invalid(value.begin(), value.end(), std::back_inserter(handoverValue));

						try
						{
							handoverData[key] = json::parse(handoverValue);
						}
						catch (std::exception&)
						{

						}
					}

					data["handover"] = std::move(handoverData);
				};

				int maxTrust = INT_MIN;
				int minVariance = INT_MAX;

				for (const auto& identifier : lockedClient->GetIdentifiers())
				{
					std::string idType = identifier.substr(0, identifier.find_first_of(':'));

					auto provider = g_providersByType[idType];

					if (provider)
					{
						maxTrust = std::max(provider->GetTrustLevel(), maxTrust);
						minVariance = std::min(provider->GetVarianceLevel(), minVariance);
					}
				}

				if (maxTrust < minTrustVar->GetValue() || minVariance > maxVarianceVar->GetValue())
				{
					sendError("You can not join this server due to your identifiers being insufficient. Please try starting Steam or another identity provider and try again.");
					return;
				}

				auto svGame = instance->GetComponent<fx::GameServer>()->GetGameName();
				bool canEnforceBuild = (svGame == fx::GameName::GTA5 || svGame == fx::GameName::RDR3);

				if (canEnforceBuild)
				{
					const auto gameBuildData = xbr::ParseGameBuildFromString(gameBuildField);

					if (gameBuildData.first == 0 && gameBuildData.second == 0)
					{
						sendError(fmt::sprintf("Invalid game build has been passed (%s).", gameBuildField));
						return;
					}

					const auto buildNumberStr = fmt::sprintf("%d", gameBuildData.first);

					if (!enforceGameBuildVar->GetValue().empty() && enforceGameBuildVar->GetValue() != buildNumberStr)
					{
						sendError(
							fmt::sprintf(
								"This server requires a different game build (%s) from the one you're using (%s).%s",
								enforceGameBuildVar->GetValue(),
								buildNumberStr,
								(svGame == fx::GameName::GTA5) ? " Tell the server owner to remove this check." : ""
							)
						);

						return;
					}

					// Default expected revision is always "0".
					auto expectedRevision = 0;

					if (const auto uniquifier = xbr::GetGameBuildUniquifier(gameName, gameBuildData.first))
					{
						expectedRevision = uniquifier->m_revision;
					}

					if (expectedRevision != gameBuildData.second)
					{
						sendError(
							fmt::sprintf(
								"Client/Server game build revision mismatch: you are running game build %d revision %d, "
								"while server expects revision %d.\n\nFor more information, please "
								"<a href=\"https://aka.cfx.re/game-build-revision-mismatch\">click here</a>.",
								gameBuildData.first, gameBuildData.second, expectedRevision
							)
						);

						return;
					}	
				}

				auto resman = instance->GetComponent<fx::ResourceManager>();
				auto eventManager = resman->GetComponent<fx::ResourceEventManagerComponent>();
				auto cbComponent = resman->GetComponent<fx::ResourceCallbackComponent>();

				// TODO: replace with event stacks once implemented
				auto noReason = std::make_shared<std::shared_ptr<std::string>>();
				*noReason = std::make_shared<std::string>("Resource prevented connection.");
				
				auto deferrals = std::make_shared<std::shared_ptr<fx::ClientDeferral>>();
				*deferrals = std::make_shared<fx::ClientDeferral>(instance, lockedClient);

				lockedClient->SetData("deferralPtr", std::weak_ptr<fx::ClientDeferral>(*deferrals));

				// *copy* the callback into a *shared* reference
				auto cbRef = std::make_shared<std::shared_ptr<std::decay_t<decltype(cb)>>>(std::make_shared<std::decay_t<decltype(cb)>>(cb));

				(*deferrals)->SetMessageCallback([cbRef](const std::string& message)
				{
					auto ref1 = *cbRef;

					if (ref1)
					{
						(*ref1)(json::object({ { "defer", true }, { "message", message.empty() ? json(nullptr) : json(message) }, { "deferVersion", 2 } }));
					}
				});

				(*deferrals)->SetCardCallback([cbRef, token](const std::string& card)
				{
					auto ref1 = *cbRef;

					if (ref1)
					{
						(*ref1)(json::object({ { "defer", true }, { "card", card }, { "token", token }, { "deferVersion", 2 } }));
					}
				});

				(*deferrals)->SetResolveCallback([addData, data, deferrals, cbRef, allowClient]()
				{
					allowClient();

					json dataNew = data;

					if (deferrals && *deferrals)
					{
						addData(dataNew, *deferrals);
					}

					auto ref1 = *cbRef;

					if (ref1)
					{
						(**cbRef)(dataNew);
						(**cbRef)(json(nullptr));
					}

					*cbRef = nullptr;
					*deferrals = nullptr;
				});

				auto earlyReject = std::make_shared<bool>(false);
				auto weakEarlyReject = std::weak_ptr(earlyReject);
				auto weakNoReason = std::weak_ptr(noReason);

				(*deferrals)->SetRejectCallback([deferrals, cbRef, clientWeak, weakEarlyReject, weakNoReason](const std::string& message)
				{
					auto earlyReject = weakEarlyReject.lock();
					auto noReason = weakNoReason.lock();

					if (earlyReject && noReason)
					{
						*noReason = std::make_shared<std::string>(message);
						*earlyReject = true;
					}

					auto newLockedClient = clientWeak.lock();
					if (newLockedClient)
					{
						auto ref1 = *cbRef;

						if (ref1)
						{
							(**cbRef)(json::object({ { "error", message } }));
							(**cbRef)(json(nullptr));
						}
					}

					*cbRef = nullptr;
					*deferrals = nullptr;
				});

				request->SetCancelHandler([cbRef, deferrals]()
				{
					*cbRef = nullptr;
					*deferrals = nullptr;
				});

				gscomms_execute_callback_on_main_thread([=]
				{
					auto deferralsRef = *deferrals;

					/*NETEV playerConnecting SERVER
					/#*
					 * A server-side event that is triggered when a player is trying to connect.
					 *
					 * This event can be canceled to reject the player *instantly*, assuming you haven't yielded.
					 *
					 * @param playerName - The display name of the player connecting.
					 * @param setKickReason - A function used to set a reason message for when the event is canceled.
					 * @param deferrals - An object to control deferrals.
					 * @param source - The player's *temporary* NetID (a number in Lua/JS), **not a real argument, use [FromSource] or source**.
					 #/
					declare function playerConnecting(playerName: string, setKickReason: (reason: string) => void, deferrals: {
						/#*
						 * `deferrals.defer` will initialize deferrals for the current resource. It is required to wait for at least a tick after calling defer before calling `update`, `presentCard` or `done`.
						 #/
						defer(): void,

						/#*
						 * `deferrals.update` will send a progress message to the connecting client.
						 *
						 * @param message - The string to send to the client.
						 #/
						update(message: string): void,

						/#*
						 * `deferrals.presentCard` will send an [Adaptive Card](https://adaptivecards.io/) to the client.
						 *
						 * @param card - An object containing card data, or a serialized JSON string with the card information.
						 * @param cb - If present, will be invoked on an `Action.Submit` event from the Adaptive Card.
						 #/
						presentCard(
							card: object | string,
							cb?:
							/#*
							 * A callback to be invoked for `Action.Submit`.
							 *
							 * @param data - A parsed version of the data sent from the card.
							 * @param rawData - A JSON string containing the data sent from the card.
							 #/
							  (data: any, rawData: string) => void
						): void,

						/#*
						 * `deferrals.done` finalizes a deferral. It is required to wait for at least a tick before calling `done` after calling a prior deferral method.
						 *
						 * @param failureReason - If specified, the connection will be refused, and the user will see the specified message as a result. If this is not specified, the user will be allowed to connect.
						 #/
						done(failureReason?: string): void,

						/#*
						 * `deferrals.handover` adds handover data for the client to be able to use at a later point.
						 *
						 * @param data - Data to pass to the connecting client.
						 #/
						handover(data: { [key: string]: any }): void,
					}, source: string): void;
					*/
					bool shouldAllow = (deferralsRef) ? (eventManager->TriggerEvent2("playerConnecting", { fmt::sprintf("internal-net:%d", lockedClient->GetNetId()) }, lockedClient->GetName(), cbComponent->CreateCallback([noReason](const msgpack::unpacked& unpacked)
					{
						auto obj = unpacked.get().as<std::vector<msgpack::object>>();

						if (obj.size() == 1)
						{
							**noReason = obj[0].as<std::string>();
						}
					}),
					deferralsRef->GetCallbacks())) : false;

					if (deferralsRef)
					{
						deferralsRef->RanEvents();
					}

					if (!shouldAllow)
					{
						*deferrals = {};
						sendError(**noReason);
						return;
					}

					if (*earlyReject)
					{
						*deferrals = {};
						sendError(**noReason);
						return;
					}

					// was the deferral already completed/canceled this frame? if so, just don't respond at all
					if (!deferralsRef)
					{
						return;
					}

					if (!deferralsRef->IsDeferred())
					{
						allowClient();

						json dataNew = data;
						addData(dataNew, *deferrals);

						*cbRef = nullptr;
						*deferrals = nullptr;

						cb(dataNew);
						cb(json(nullptr));
					}
				});
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

					// if the client randomly disconnects, let's just bail
					auto clientLocked = clientWeak.lock();
					if (!clientLocked)
					{
						// unset the callback
						*runOneIdentifier = nullptr;

						return;
					}

					auth->RunAuthentication(clientLocked, request, postMap, [=](boost::optional<std::string> err)
					{
						// if the client randomly disconnects, let's just bail (again)
						auto newClientLocked = clientWeak.lock();
						if (!newClientLocked)
						{
							// unset the callback
							*runOneIdentifier = nullptr;

							return;
						}

						// if an auth method fails, bail
						if (err)
						{
							sendError(*err);

							// unset the callback
							*runOneIdentifier = nullptr;
						}
						else
						{
							(**runOneIdentifier)(thisIt);
						}
					});
				}
			});

			(**runOneIdentifier)(it);
		});

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("submitCard", [=](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const std::function<void(const json&)>& cb)
		{
			auto dataIt = postMap.find("data");
			auto tokenIt = postMap.find("token");

			if (dataIt == postMap.end() || tokenIt == postMap.end())
			{
				cb(json::object({ {"error", "fields missing"} }));
				cb(json(nullptr));
				return;
			}

			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
			auto client = clientRegistry->GetClientByConnectionToken(tokenIt->second);

			if (!client)
			{
				cb(json::object({ {"error", "no client"} }));
				cb(json(nullptr));
				return;
			}

			auto deferralRef = client->GetData("deferralPtr");

			if (deferralRef)
			{
				auto deferralPtr = fx::AnyCast<std::weak_ptr<fx::ClientDeferral>>(deferralRef);
				auto deferrals = deferralPtr.lock();

				if (deferrals)
				{
					deferrals->HandleCardResponse(dataIt->second);
				}
			}

			cb(json::object({ { "result", "ok" } }));
			cb(json(nullptr));
		});
	}, 50);
});
