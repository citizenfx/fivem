#include "StdInc.h"

#include <ClientRegistry.h>

#include <ServerInstanceBase.h>
#include <HttpServerManager.h>

#include <ResourceManager.h>

#include <VFSManager.h>

#include <botan/base64.h>

#include <json.hpp>

#include <cfx_version.h>

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
		auto epPrivacy = instance->AddVariable<bool>("sv_endpointPrivacy", ConVar_None, false);

		// max clients cap
		maxClientsVar->GetHelper()->SetConstraints(1, MAX_CLIENTS);

		struct InfoData
		{
			json infoJson;
			int infoHash;

			InfoData()
				: infoHash(0), infoJson({ { "server", "FXServer-" GIT_DESCRIPTION }, { "enhancedHostSupport", true }, { "resources", { } } })
			{
				Update();
			}

			void Update()
			{
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

				infoHash = static_cast<int>(std::hash<std::string>()(infoJson.dump()) & 0x7FFFFFFF);
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

							infoData->infoJson["icon"] = Botan::base64_encode(iconBytes);
							infoData->Update();
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

			response->End(infoData->infoJson.dump());
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

				data.push_back({
					{ "endpoint", (showEP) ? client->GetAddress().ToString() : "127.0.0.1" },
					{ "id", client->GetNetId() },
					{ "identifiers", identifiers },
					{ "name", client->GetName() },
					{ "ping", gscomms_get_peer(client->GetPeer())->roundTripTime }
				});
			});

			response->End(data.dump());
		});
	}, 1500);
});
