#include "StdInc.h"
#include <ClientHttpHandler.h>
#include <ResourceManager.h>
#include <ResourceFilesComponent.h>
#include <ResourceStreamComponent.h>
#include <ResourceMetaDataComponent.h>
#include <ResourceConfigurationCacheComponent.h>

#include <Client.h>
#include <ClientRegistry.h>
#include <ServerInstanceBase.h>

#include <KeyedRateLimiter.h>

#include <regex>

extern std::shared_mutex g_resourceStartOrderLock;
extern std::list<std::string> g_resourceStartOrder;

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		fx::ResourceManager* resman = instance->GetComponent<fx::ResourceManager>().GetRef();

		struct FileServerEntry
		{
			std::string reString;
			std::regex re;
			std::string url;
		};

		static std::shared_mutex fileServerLock;
		static std::vector<FileServerEntry> fileServers;

		static auto addFileServerCmd = instance->AddCommand("fileserver_add", [](const std::string& resourcePattern, const std::string& fileServer)
		{
			FileServerEntry entry;
			entry.re = resourcePattern;
			entry.reString = resourcePattern;
			entry.url = fileServer;

			std::unique_lock<std::shared_mutex> lock(fileServerLock);
			fileServers.push_back(std::move(entry));

			const auto resourceManager = fx::ResourceManager::GetCurrent();
			if (!resourceManager)
			{
				return;
			}

			resourceManager->ForAllResources([](const fwRefContainer<fx::Resource>& resource)
			{
				auto& configurationCache = resource->GetComponent<fx::ResourceConfigurationCacheComponent>();
				configurationCache->Invalidate();
			});
		});

		static auto removeFileServerCmd = instance->AddCommand("fileserver_remove", [](const std::string& resourcePattern)
		{
			std::unique_lock<std::shared_mutex> lock(fileServerLock);

			for (auto it = fileServers.begin(); it != fileServers.end();)
			{
				if (it->reString == resourcePattern)
				{
					it = fileServers.erase(it);
				}
				else
				{
					it++;
				}
			}

			const auto resourceManager = fx::ResourceManager::GetCurrent();
			if (!resourceManager)
			{
				return;
			}

			resourceManager->ForAllResources([](const fwRefContainer<fx::Resource>& resource)
			{
				auto& configurationCache = resource->GetComponent<fx::ResourceConfigurationCacheComponent>();
				configurationCache->Invalidate();
			});
		});

		static auto listFileServerCmd = instance->AddCommand("fileserver_list", []()
		{
			std::shared_lock<std::shared_mutex> lock(fileServerLock);

			for (auto& entry : fileServers)
			{
				console::Printf("fileserver", "%s -> %s\n", entry.reString, entry.url);
			}
		});

		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("getConfiguration", [=](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const fx::ClientMethodRegistry::TCallbackFast& cb)
		{
			// todo: log thread
			auto ra = request->GetRemoteAddress();
			auto token = request->GetHeader("X-CitizenFX-Token");

			fx::ClientSharedPtr client;

			if (!token.empty())
			{
				client = clientRegistry->GetClientByConnectionToken(token);
			}

			if (!client)
			{
				client = clientRegistry->GetClientByTcpEndPoint(ra.substr(0, ra.find_last_of(':')));
			}

			auto sendError = [&cb](const char* e)
			{
				rapidjson::Document retval;
				retval.SetObject();

				retval.AddMember("error", rapidjson::Value{ e, retval.GetAllocator() }, retval.GetAllocator());

				cb(retval);

				rapidjson::Document nil;
				nil.SetNull();

				cb(nil);
			};

			if (!client)
			{
				sendError("Not a valid client.");
				return;
			}

			client->Touch();

			if (!client->GetData("passedValidation"))
			{
				sendError("Client is still connecting.");
				return;
			}

			auto rl = instance->GetComponent<fx::TokenRateLimiter>();
			if (!rl->Consume(client->GetConnectionToken()))
			{
				sendError("Rate limit exceeded.");
				return;
			}

			rapidjson::Document retval;
			retval.SetObject();

			rapidjson::Value resources;
			resources.SetArray();

			auto resourceIt = postMap.find("resources");
			std::set<std::string_view> filters;

			if (resourceIt != postMap.end())
			{
				std::string_view filterValues = resourceIt->second;

				size_t lastPos = 0;
				size_t pos = -1;

				do
				{
					lastPos = pos + 1;
					pos = filterValues.find_first_of(';', pos + 1);

					auto thisValue = filterValues.substr(lastPos, pos - lastPos);

					// empty values are not valid
					if (thisValue.empty())
					{
						// do not continue filter inserting
						break;
					}

					filters.insert(thisValue);
				} while (pos != std::string::npos);
			}

			std::set<std::string_view> resourceNames;

			auto appendResource = [&](const fwRefContainer<fx::Resource>& resource)
			{
				// filtering out duplicates
				if (!resourceNames.insert(resource->GetName()).second)
				{
					return;
				}

				// client only requested a specific resource configuration
				if (!filters.empty() && filters.find(resource->GetName()) == filters.end())
				{
					return;
				}

				// filtering out none started resources
				if (resource->GetState() != fx::ResourceState::Started && resource->GetState() != fx::ResourceState::Starting)
				{
					return;
				}

				auto metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();
				auto iv = metaData->GetEntries("server_only");

				// do not send server_only resources to the client
				if (iv.begin() != iv.end())
				{
					return;
				}

				auto& configurationCache = resource->GetComponent<fx::ResourceConfigurationCacheComponent>();
				rapidjson::Value configurationObj(rapidjson::kObjectType);
				configurationCache->GetConfiguration(configurationObj, retval.GetAllocator(), [](const std::string& resourceName)
				{
					std::unique_lock<std::shared_mutex> lock(fileServerLock);

					for (const auto& entry : fileServers)
					{
						if (std::regex_match(resourceName, entry.re))
						{
							return std::optional{entry.url};
						}
					}

					return std::optional<std::string>{};
				});

				resources.PushBack(configurationObj, retval.GetAllocator());
			};

			// first append in start order
			decltype(g_resourceStartOrder) resourceOrder;

			{
				std::shared_lock<std::shared_mutex> _(g_resourceStartOrderLock);
				resourceOrder = g_resourceStartOrder;
			}

			for (auto& resourceName : resourceOrder)
			{
				auto resource = resman->GetResource(resourceName, false);

				if (resource.GetRef())
				{
					appendResource(resource);
				}
			}

			// then append any leftovers
			resman->ForAllResources(appendResource);

			retval.AddMember("fileServer", "https://%s/files", retval.GetAllocator());
			retval.AddMember("resources", std::move(resources), retval.GetAllocator());

			cb(retval);

			rapidjson::Document nil;
			nil.SetNull();

			cb(nil);
		});
	},
	5000);
});
