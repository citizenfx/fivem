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

#include <cfx_version.h>
#include <optional>

using json = nlohmann::json;

inline uint32_t SwapLong(uint32_t x)
{
	x = ((x << 8) & 0xFF00FF00) | ((x >> 8) & 0xFF00FF);
	return (x << 16) | (x >> 16);
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		// TODO: make instanceable
		static auto instanceRef = instance;
		static auto ivVar = instance->AddVariable<int>("sv_infoVersion", ConVar_ServerInfo, 0);
		static auto maxClientsVar = instance->AddVariable<int>("sv_maxClients", ConVar_ServerInfo, 30);
		static auto iconVar = instance->AddVariable<std::string>("sv_icon", ConVar_None, "");
		static auto versionVar = instance->AddVariable<std::string>("version", ConVar_None, "FXServer-" GIT_DESCRIPTION);
		static auto crashCmd = instance->AddCommand("_crash", []()
		{
			*(volatile int*)0 = 0;
		});
		auto epPrivacy = instance->AddVariable<bool>("sv_endpointPrivacy", ConVar_None, false);

		// max clients cap
		maxClientsVar->GetHelper()->SetConstraints(1, MAX_CLIENTS);

		struct InfoData
		{
			json infoJson;
			std::recursive_mutex infoJsonMutex;
			int infoHash;

			InfoData()
				: infoHash(0), infoJson({ { "server", "FXServer-" GIT_DESCRIPTION }, { "enhancedHostSupport", true }, { "resources", { } } })
			{
				Update();
			}

			void Update()
			{
				std::unique_lock<std::recursive_mutex> lock(infoJsonMutex);

				auto varman = instanceRef->GetComponent<console::Context>()->GetVariableManager();

				infoJson["vars"] = json::object();

				varman->ForAllVariables([&](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& var)
				{
					// don't return more variable information
					if (name == "sv_infoVersion" || name == "sv_hostname")
					{
						return;
					}

					infoJson["vars"][name] = var->GetValue();
				}, ConVar_ServerInfo);

				infoJson["resources"] = json::array();
				infoJson["resources"].push_back("hardcap");

				auto resman = instanceRef->GetComponent<fx::ResourceManager>();
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

				infoJson["version"] = 0;

				infoHash = static_cast<int>(HashRageString(infoJson.dump(-1, ' ', false, json::error_handler_t::replace).c_str()) & 0x7FFFFFFF);
				infoJson["version"] = infoHash;

				ivVar->GetHelper()->SetRawValue(infoHash);
			}
		};

		static auto infoData = std::make_shared<InfoData>();

		static auto iconCmd = instance->AddCommand("load_server_icon", [=](const std::string& path)
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

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/info.json", [=](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			infoData->Update();

			{
				std::unique_lock<std::recursive_mutex> lock(infoData->infoJsonMutex);
				response->End(infoData->infoJson.dump(-1, ' ', false, json::error_handler_t::replace));
			}
		});

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/dynamic.json", [=](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			auto server = instance->GetComponent<fx::GameServer>();

			int numClients = 0;

			instance->GetComponent<fx::ClientRegistry>()->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
			{
				if (client->GetNetId() < 0xFFFF)
				{
					++numClients;
				}
			});

			auto json = json::object({
				{ "hostname", server->GetVariable("sv_hostname") },
				{ "gametype", server->GetVariable("gametype") },
				{ "mapname", server->GetVariable("mapname") },
				{ "clients", numClients },
				{ "iv", server->GetVariable("sv_infoVersion") },
				{ "sv_maxclients", server->GetVariable("sv_maxclients") },
			});

			response->End(json.dump(-1, ' ', false, json::error_handler_t::replace));
		});

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/players.json", [=](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			json data = json::array();

			clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
			{
				if (client->GetNetId() >= 0xFFFF)
				{
					return;
				}

				bool showEP = !epPrivacy->GetValue();

				auto identifiers = client->GetIdentifiers();

				if (!showEP)
				{
					auto newEnd = std::remove_if(identifiers.begin(), identifiers.end(), [](const std::string& identifier)
					{
						return (identifier.find("ip:") == 0);
					});

					identifiers.erase(newEnd, identifiers.end());
				}

				auto peer = gscomms_get_peer(client->GetPeer());

				data.push_back({
					{ "endpoint", (showEP) ? client->GetAddress().ToString() : "127.0.0.1" },
					{ "id", client->GetNetId() },
					{ "identifiers", identifiers },
					{ "name", client->GetName() },
					{ "ping", peer.GetRef() ? peer->GetPing() : -1 }
				});
			});

			response->End(data.dump(-1, ' ', false, json::error_handler_t::replace));
		});

		static std::optional<json> lastProfile;

		instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ProfilerComponent>()->OnRequestView.Connect([instance](const json& json)
		{
			lastProfile = json;

			auto baseUrl = instance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("web_baseUrl");
			
			if (baseUrl)
			{
				console::Printf("profiler", "You can view the recorded profile data at ^4https://frontend.chrome-dev.tools/serve_rev/@901bcc219d9204748f9c256ceca0f2cd68061006/inspector.html?loadTimelineFromURL=https://%s/profileData.json^7 in Chrome (or compatible).\n",
					baseUrl->GetValue());
			}
		});

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/profileData.json", [=](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			if (!lastProfile)
			{
				response->SetStatusCode(404);
				response->End("[]");

				return;
			}

			response->SetHeader("Access-Control-Allow-Origin", "*");

			response->End(lastProfile->dump(-1, ' ', false, json::error_handler_t::replace));
		});
	}, 1500);
});
