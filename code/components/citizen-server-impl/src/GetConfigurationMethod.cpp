#include "StdInc.h"
#include <ClientHttpHandler.h>
#include <ResourceManager.h>
#include <ResourceFilesComponent.h>
#include <ResourceStreamComponent.h>
#include <ResourceMetaDataComponent.h>

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
		});

		static auto removeFileServerCmd = instance->AddCommand("fileserver_remove", [](const std::string& resourcePattern)
		{
			std::unique_lock<std::shared_mutex> lock(fileServerLock);

			for (auto it = fileServers.begin(); it != fileServers.end(); )
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

		instance->SetComponent(new fx::TokenRateLimiter(1.0f, 3.0f));

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("getConfiguration", [=](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const fx::ClientMethodRegistry::TCallbackFast& cb)
		{
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

			if (!client->GetData("passedValidation").has_value())
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

				int lastPos = 0;
				int pos = -1;

				do 
				{
					lastPos = pos + 1;
					pos = filterValues.find_first_of(';', pos + 1);

					auto thisValue = filterValues.substr(lastPos, (pos - lastPos));

					filters.insert(thisValue);
				} while (pos != std::string::npos);
			}

			std::set<std::string_view> resourceNames;

			auto appendResource = [&](const fwRefContainer<fx::Resource>& resource)
			{
				if (resourceNames.find(resource->GetName()) != resourceNames.end())
				{
					return;
				}

				if (resource->GetName() == "_cfx_internal")
				{
					return;
				}

				if (!filters.empty() && filters.find(resource->GetName()) == filters.end())
				{
					return;
				}

				if (resource->GetState() != fx::ResourceState::Started && resource->GetState() != fx::ResourceState::Starting)
				{
					return;
				}

				auto metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();
				auto iv = metaData->GetEntries("server_only");

				if (iv.begin() != iv.end())
				{
					return;
				}

				resourceNames.insert(resource->GetName());

				rapidjson::Value resourceFiles;
				resourceFiles.SetObject();

				fwRefContainer<fx::ResourceFilesComponent> files = resource->GetComponent<fx::ResourceFilesComponent>();

				for (const auto& entry : files->GetFileHashPairs())
				{
					auto key = rapidjson::Value{ entry.first.c_str(), static_cast<rapidjson::SizeType>(entry.first.length()), retval.GetAllocator() };
					auto value = rapidjson::Value{ entry.second.c_str(), static_cast<rapidjson::SizeType>(entry.second.length()), retval.GetAllocator() };

					resourceFiles.AddMember(std::move(key), std::move(value), retval.GetAllocator());
				}

				rapidjson::Value resourceStreamFiles;
				resourceStreamFiles.SetObject();

				fwRefContainer<fx::ResourceStreamComponent> streamFiles = resource->GetComponent<fx::ResourceStreamComponent>();

				for (const auto& entry : streamFiles->GetStreamingList())
				{
					if (!entry.second.isAutoScan)
					{
						continue;
					}

					rapidjson::Value obj;
					obj.SetObject();

					obj.AddMember("hash", rapidjson::Value{ entry.second.hashString, retval.GetAllocator() }, retval.GetAllocator());
					obj.AddMember("rscFlags", rapidjson::Value{ entry.second.rscFlags }, retval.GetAllocator());
					obj.AddMember("rscVersion", rapidjson::Value{ entry.second.rscVersion }, retval.GetAllocator());
					obj.AddMember("size", rapidjson::Value{ entry.second.size }, retval.GetAllocator());

					if (entry.second.isResource)
					{
						obj.AddMember("rscPagesVirtual", rapidjson::Value{ entry.second.rscPagesVirtual }, retval.GetAllocator());
						obj.AddMember("rscPagesPhysical", rapidjson::Value{ entry.second.rscPagesPhysical }, retval.GetAllocator());
					}

					auto key = rapidjson::Value{ entry.first.c_str(), static_cast<rapidjson::SizeType>(entry.first.length()), retval.GetAllocator() };
					resourceStreamFiles.AddMember(std::move(key), std::move(obj), retval.GetAllocator());
				}

				rapidjson::Value obj;
				obj.SetObject();

				obj.AddMember("name", rapidjson::StringRef(resource->GetName().c_str(), resource->GetName().length()), retval.GetAllocator());
				obj.AddMember("files", std::move(resourceFiles), retval.GetAllocator());
				obj.AddMember("streamFiles", std::move(resourceStreamFiles), retval.GetAllocator());

				{
					std::unique_lock<std::shared_mutex> lock(fileServerLock);

					for (const auto& entry : fileServers)
					{
						if (std::regex_match(resource->GetName(), entry.re))
						{
							obj.AddMember("fileServer", rapidjson::Value{ entry.url.c_str(), static_cast<rapidjson::SizeType>(entry.url.length()), retval.GetAllocator() }, retval.GetAllocator());
							break;
						}
					}
				}

				resources.PushBack(std::move(obj), retval.GetAllocator());
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
	}, 5000);
});
