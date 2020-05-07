#include "StdInc.h"
#include <ClientHttpHandler.h>
#include <ResourceManager.h>
#include <ResourceFilesComponent.h>
#include <ResourceStreamComponent.h>
#include <ResourceMetaDataComponent.h>

#include <ServerInstanceBase.h>

#include <regex>

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

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("getConfiguration", [=](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const fx::ClientMethodRegistry::TCallbackFast& cb)
		{
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

			resman->ForAllResources([&](fwRefContainer<fx::Resource> resource)
			{
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
			});

			retval.AddMember("fileServer", "https://%s/files", retval.GetAllocator());
			retval.AddMember("resources", std::move(resources), retval.GetAllocator());

			cb(retval);

			rapidjson::Document nil;
			nil.SetNull();

			cb(nil);
		});
	}, 5000);
});
