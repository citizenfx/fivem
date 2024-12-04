#include "StdInc.h"

#include <ClientRegistry.h>

#include <ServerInstanceBase.h>
#include <HttpServerManager.h>

#include <ResourceManager.h>
#include <Profiler.h>

#include <GameServer.h>
#include <VFSManager.h>

#include <botan/base64.h>

#include <json.hpp>

#include <KeyedRateLimiter.h>
#include <TcpListenManager.h>

#include <InfoHttpHandler.h>

#include <cfx_version.h>
#include <optional>
#include <skyr/v1/url.hpp>

#include "ForceConsteval.h"

using json = nlohmann::json;

inline uint32_t SwapLong(uint32_t x)
{
	x = ((x << 8) & 0xFF00FF00) | ((x >> 8) & 0xFF00FF);
	return (x << 16) | (x >> 16);
}

extern std::shared_ptr<ConVar<std::string>> g_steamApiKey;

namespace fx
{
struct InfoHttpHandlerComponentLocals : fwRefCountable
{
	void AttachToObject(ServerInstanceBase* instance);

	struct InfoData
	{
		json infoJson;
		std::recursive_mutex infoJsonMutex;

		std::string infoJsonStr;
		std::shared_mutex infoJsonStrMutex;
		int infoHash;

		ServerInstanceBase* m_instance;
		ConVar<int>* ivVar;

		InfoData(ServerInstanceBase* instance, ConVar<int>* ivVar)
			: infoHash(0), infoJson({ { "server", "FXServer-" GIT_DESCRIPTION }, { "enhancedHostSupport", true }, { "resources", {} } }), m_instance(instance), ivVar(ivVar)
		{
			Update();
		}

		void Update()
		{
			if (infoJsonMutex.try_lock())
			{
				infoJson["vars"] = json::object();
					
				for (auto varman : { m_instance->GetComponent<console::Context>()->GetVariableManager(), console::GetDefaultContext()->GetVariableManager() })
				{
					varman->ForAllVariables([&](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& var)
					{
						// don't return more variable information
						if (name == "sv_infoVersion" || name == "sv_hostname")
						{
							return;
						}

						infoJson["vars"][name] = var->GetValue();
					},
					ConVar_ServerInfo);
				}

				infoJson["resources"] = json::array();
				infoJson["resources"].push_back("hardcap");

				auto resman = m_instance->GetComponent<fx::ResourceManager>();
				resman->ForAllResources([&](fwRefContainer<fx::Resource> resource)
				{
					// we've already listed hardcap, no need to actually return it again
					if (resource->GetName() == "hardcap")
					{
						return;
					}

					// only output started resources
					if (resource->GetState() != fx::ResourceState::Started)
					{
						return;
					}

					infoJson["resources"].push_back(resource->GetName());
				});

				std::string requestSteamTicket = "on";
				if (g_steamApiKey->GetValue().empty())
				{
					requestSteamTicket = "unset";
				}
				else if (g_steamApiKey->GetValue() == "none")
				{
					requestSteamTicket = "off";
				}

				infoJson["requestSteamTicket"] = requestSteamTicket;

				infoJson["version"] = 0;

				infoHash = static_cast<int>(HashRageString(infoJson.dump(-1, ' ', false, json::error_handler_t::replace).c_str()) & 0x7FFFFFFF);
				infoJson["version"] = infoHash;

				ivVar->GetHelper()->SetRawValue(infoHash);

				{
					std::lock_guard<std::shared_mutex> _(infoJsonStrMutex);
					infoJsonStr = infoJson.dump(-1, ' ', false, json::error_handler_t::replace);
				}

				infoJsonMutex.unlock();
			}
		}
	};



	std::shared_ptr<ConVar<int>> ivVar;
	std::shared_ptr<ConVar<int>> maxClientsVar;
	std::shared_ptr<ConVar<std::string>> iconVar;
	std::shared_ptr<ConVar<std::string>> versionVar;
	std::shared_ptr<ConVar<int>> versionBuildNoVar;
	std::shared_ptr<ConsoleCommand> crashCmd;
	int paranoiaLevel = 0;
	std::shared_ptr<ConVar<int>> paranoiaVar;
	std::shared_ptr<ConVar<bool>> epPrivacy;
	std::shared_ptr<ConVar<bool>> exposePlayerIdentifiersInHttpEndpoint;
	std::shared_ptr<InfoData> infoData;
	std::shared_ptr<ConsoleCommand> iconCmd;

	std::shared_mutex playerBlobMutex;
	std::string playerBlob;
	std::string publicPlayerBlob;

	std::chrono::milliseconds nextPlayerUpdate{ 0 };

	ServerInstanceBase* m_instance;

	json GetPlayersJson();
	json GetInfoJson();
	json GetDynamicJson();
};

void InfoHttpHandlerComponentLocals::AttachToObject(fx::ServerInstanceBase* instance)
{
	m_instance = instance;
	ivVar = instance->AddVariable<int>("sv_infoVersion", ConVar_ServerInfo, 0);
	maxClientsVar = instance->AddVariable<int>("sv_maxClients", ConVar_ServerInfo, 30);
	iconVar = instance->AddVariable<std::string>("sv_icon", ConVar_Internal, "");
	versionVar = instance->AddVariable<std::string>("version", ConVar_Internal, "FXServer-" GIT_DESCRIPTION);
	const char* lastPeriod = strrchr(GIT_TAG, '.');
	int versionBuildNo = lastPeriod == nullptr ? 0 : strtol(lastPeriod + 1, nullptr, 10);
	versionBuildNoVar = instance->AddVariable<int>("buildNumber", ConVar_Internal, versionBuildNo);
	crashCmd = instance->AddCommand("_crash", []()
	{
		*(volatile int*)0 = 0;
	});
	paranoiaLevel = 0;
	paranoiaVar = instance->AddVariable<int>("sv_requestParanoia", ConVar_None, 0, &paranoiaLevel);
	epPrivacy = instance->AddVariable<bool>("sv_endpointPrivacy", ConVar_None, true);
	exposePlayerIdentifiersInHttpEndpoint = instance->AddVariable<bool>("sv_exposePlayerIdentifiersInHttpEndpoint", ConVar_None, false);

	auto processRequestParanoia = [this](const fwRefContainer<net::HttpRequest>& request)
	{
		bool shouldMurder = false;

		if (!request->GetHeader("via", "").empty())
		{
			shouldMurder = paranoiaLevel >= 1;
		}
		else if (!request->GetHeader("upgrade-insecure-requests", "").empty())
		{
			shouldMurder = paranoiaLevel >= 2;
		}

		if (shouldMurder)
		{
			auto tcpListenMgr = m_instance->GetComponent<fx::TcpListenManager>();
			tcpListenMgr->BlockPeer(request->GetRemotePeer());
		}

		return shouldMurder;
	};

	// max clients cap
	maxClientsVar->GetHelper()->SetConstraints(1, MAX_CLIENTS);

	infoData = std::make_shared<InfoData>(instance, ivVar.get());

	iconCmd = instance->AddCommand("load_server_icon", [=](const std::string& path)
	{
		auto stream = vfs::OpenRead(path);

		if (!stream.GetRef())
		{
			trace("Could not open %s for reading.\n", path);
			return;
		}

		auto header = stream->Read(8);

		static const uint8_t pngBytes[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

		if (header.size() == 8 && memcmp(header.data(), pngBytes, 8) == 0)
		{
			struct chunkHdr
			{
				// all in big endian
				uint32_t length;
				uint32_t code;
			};

			chunkHdr chunk;
			stream->Read(&chunk, sizeof(chunk));

			if (SwapLong(chunk.length) >= 13)
			{
				if (chunk.code == 'RDHI')
				{
					struct ihdr
					{
						uint32_t width;
						uint32_t height;
						uint8_t depth;
						uint8_t colorType;
						uint8_t compression;
						uint8_t filter;
						uint8_t interlace;
					};

					ihdr hdr;
					stream->Read(&hdr, sizeof(hdr));

					if (SwapLong(hdr.width) == 96 && SwapLong(hdr.height) == 96)
					{
						stream->Seek(0, SEEK_SET);
						auto iconBytes = stream->ReadToEnd();
						auto iconString = Botan::base64_encode(iconBytes);;

						infoData->infoJson["icon"] = iconString;
						infoData->Update();

						iconVar->GetHelper()->SetRawValue(iconString);
					}
					else
					{
						trace("The file %s must be a 96x96 PNG image to be used as icon.\n", path);
					}
				}
				else
				{
					trace("The file %s is not a valid PNG file.\n", path);
				}
			}
			else
			{
				trace("The file %s is not a valid PNG file.\n", path);
			}
		}
		else
		{
			trace("The file %s is not a PNG file.\n", path);
		}
	});

	instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/info.json", [=](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
	{
		if (processRequestParanoia(request))
		{
			response->SetStatusCode(403);
			response->End("Nope.");

			if (paranoiaLevel >= 3)
			{
				response->CloseSocket();
			}

			return;
		}

		static auto limiter = instance->GetComponent<fx::PeerAddressRateLimiterStore>()->GetRateLimiter("http_info", fx::RateLimiterDefaults{ 4.0, 10.0 });
		auto address = request->GetRemotePeer();

		bool cooldown = false;

		if (!fx::IsProxyAddress(address) && !limiter->Consume(address, 1.0, &cooldown))
		{
			if (cooldown)
			{
				response->CloseSocket();
				return;
			}

			response->SetStatusCode(429);
			response->End("Rate limit exceeded.");
			return;
		}

		infoData->Update();

		{
			std::shared_lock<std::shared_mutex> lock(infoData->infoJsonStrMutex);
			response->End(infoData->infoJsonStr);
		}
	});

	instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/dynamic.json", [=](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
	{
		if (processRequestParanoia(request))
		{
			response->SetStatusCode(403);
			response->End("Nope.");
			return;
		}

		auto json = GetDynamicJson();

		response->End(json.dump(-1, ' ', false, json::error_handler_t::replace));
	});

	instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/players.json", [this, instance, processRequestParanoia](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
	{
		if (processRequestParanoia(request))
		{
			response->SetStatusCode(403);
			response->End("Nope.");

			if (paranoiaLevel >= 3)
			{
				response->CloseSocket();
			}

			return;
		}

		static auto limiter = instance->GetComponent<fx::PeerAddressRateLimiterStore>()->GetRateLimiter("http_players", fx::RateLimiterDefaults{ 4.0, 10.0 });
		auto address = request->GetRemotePeer();

		bool cooldown = false;

		if (!fx::IsProxyAddress(address) && !limiter->Consume(address, 1.0, &cooldown))
		{
			if (cooldown)
			{
				response->CloseSocket();
				return;
			}

			response->SetStatusCode(429);
			response->End("Rate limit exceeded.");
			return;
		}

		const auto server = instance->GetComponent<fx::GameServer>();

		bool authorizedRequest = false;
		if (const auto playersToken = request->GetHeader("X-Players-Token", ""); !playersToken.empty() && server->GetPlayersToken() == playersToken)
		{
			authorizedRequest = true;
		}
		
		if (auto path = request->GetPath(); std::string_view{path.data(), path.size()}.rfind("/players.json", 0) == 0)
		{
			constexpr uint8_t pathLength = net::force_consteval<int, std::string_view("/players.json").size()>;
			skyr::v1::url_search_parameters searchParameters (std::string_view{path.data() + pathLength, path.size() - pathLength});
			if (auto token = searchParameters.get("token"); token.has_value() && server->GetPlayersToken() == token.value())
			{
				authorizedRequest = true;
			}
		}

		if (authorizedRequest)
		{
			std::shared_lock<std::shared_mutex> lock(playerBlobMutex);
			response->End(playerBlob);
			return;
		}

		std::shared_lock<std::shared_mutex> lock(playerBlobMutex);
		response->End(publicPlayerBlob);
	});

	instance->GetComponent<fx::GameServer>()->OnTick.Connect([this, instance]()
	{
		auto now = msec();

		if (now < nextPlayerUpdate)
		{
			return;
		}

		nextPlayerUpdate = now + 1s;

		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

		json data = json::array();
		json publicData = json::array();

		bool showEndpoint = !epPrivacy->GetValue();
		const bool hidePlayerIdentifiers = !exposePlayerIdentifiersInHttpEndpoint->GetValue();

		clientRegistry->ForAllClients([&](const fx::ClientSharedPtr& client)
		{
			if (!client->HasConnected())
			{
				return;
			}

			auto identifiers = client->GetIdentifiers();
			if (!showEndpoint)
			{
				auto newEnd = std::remove_if(identifiers.begin(), identifiers.end(), [](const std::string& identifier)
				{
					return (identifier.find("ip:") == 0);
				});

				identifiers.erase(newEnd, identifiers.end());
			}

			fx::NetPeerStackBuffer stackBuffer;
			gscomms_get_peer(client->GetPeer(), stackBuffer);
			auto peer = stackBuffer.GetBase();

			auto playerData = json::object({
				{ "endpoint", (showEndpoint) ? client->GetAddress().ToString() : "127.0.0.1" },
				{ "id", client->GetNetId() },
				{ "identifiers", json::array() },
				{ "name", client->GetName() },
				{ "ping", peer ? peer->GetPing() : -1 }
			});

			// pushes the player data without identifiers to the public api
			publicData.push_back(playerData);

			auto privatePlayerData = playerData;

			// adds the identifiers
			privatePlayerData["identifiers"] = identifiers;

			// pushes the player data with identifiers to the private api
			data.push_back(privatePlayerData);
		});

		std::unique_lock<std::shared_mutex> lock(playerBlobMutex);
		playerBlob = data.dump(-1, ' ', false, json::error_handler_t::replace);

		if (hidePlayerIdentifiers)
		{
			publicPlayerBlob = publicData.dump(-1, ' ', false, json::error_handler_t::replace);
		}
		else
		{
			publicPlayerBlob = playerBlob;
		}
	});

	static std::optional<json> lastProfile;

	instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ProfilerComponent>()->OnRequestView.Connect([instance](const json& json)
	{
		lastProfile = json;

		auto baseUrl = instance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("web_baseUrl");
			
		if (baseUrl)
		{
			const auto server = instance->GetComponent<fx::GameServer>();

			std::string authentication{};

			if (!server->GetProfileDataToken().empty())
			{
				authentication = "?token=" + server->GetProfileDataToken();
			}

			console::Printf("profiler", "You can view the recorded profile data at ^4%s?loadTimelineFromURL=https://%s/profileData.json%s^7 in Chrome (or compatible).\n",
				fx::ProfilerComponent::GetDevToolsURL(), baseUrl->GetValue(), authentication.c_str());
		}
	});

	instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/profileData.json", [this, instance](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
	{
		const auto server = instance->GetComponent<fx::GameServer>();

		bool needsAuthorization = !server->GetProfileDataToken().empty();
		bool authorizedRequest = false;

		if (needsAuthorization)
		{
			if (auto path = request->GetPath(); std::string_view{path.data(), path.size()}.rfind("/profileData.json", 0) == 0)
			{
				constexpr uint8_t pathLength = net::force_consteval<int, std::string_view("/profileData.json").size()>;
				skyr::v1::url_search_parameters searchParameters (std::string_view{path.data() + pathLength, path.size() - pathLength});
				if (auto token = searchParameters.get("token"); token.has_value() && server->GetProfileDataToken() == token.value())
				{
					authorizedRequest = true;
				}
			}

			if (!authorizedRequest)
			{
				response->SetStatusCode(401);
				response->End("Unauthorized");
				return;
			}
		}

		if (!lastProfile)
		{
			response->SetStatusCode(404);
			response->End("[]");

			return;
		}

		response->SetHeader("Access-Control-Allow-Origin", "*");

		response->End(lastProfile->dump(-1, ' ', false, json::error_handler_t::replace));
	});
}

json InfoHttpHandlerComponentLocals::GetInfoJson()
{
	infoData->Update();

	{
		std::unique_lock<std::recursive_mutex> lock(infoData->infoJsonMutex);
		return infoData->infoJson;
	}
}

json InfoHttpHandlerComponentLocals::GetPlayersJson()
{
	std::shared_lock<std::shared_mutex> lock(playerBlobMutex);

	if (playerBlob.empty())
	{
		return json::array();
	}

	return json::parse(playerBlob);
}

json InfoHttpHandlerComponentLocals::GetDynamicJson()
{
	auto server = m_instance->GetComponent<fx::GameServer>();

	auto json = json::object({
		{ "hostname", server->GetVariable("sv_hostname") },
		{ "gametype", server->GetVariable("gametype") },
		{ "mapname", server->GetVariable("mapname") },
		{ "clients", m_instance->GetComponent<fx::ClientRegistry>()->GetAmountOfConnectedClients() },
		{ "iv", server->GetVariable("sv_infoVersion") },
		{ "sv_maxclients", server->GetVariable("sv_maxclients") },
	});

	return json;
}

void InfoHttpHandlerComponent::AttachToObject(fx::ServerInstanceBase* instance)
{
	fwRefContainer<InfoHttpHandlerComponentLocals> impl = new InfoHttpHandlerComponentLocals();
	impl->AttachToObject(instance);

	m_impl = impl;
}

void InfoHttpHandlerComponent::GetJsonData(nlohmann::json* infoJson, nlohmann::json* dynamicJson, nlohmann::json* playersJson)
{
	auto impl = static_cast<InfoHttpHandlerComponentLocals*>(m_impl.GetRef());
	
	if (infoJson)
	{
		*infoJson = impl->GetInfoJson();
	}

	if (dynamicJson)
	{
		*dynamicJson = impl->GetDynamicJson();
	}

	if (playersJson)
	{
		*playersJson = impl->GetPlayersJson();
	}
}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::InfoHttpHandlerComponent);
	}, 1500);
});
