#include "StdInc.h"
#include <ClientHttpHandler.h>
#include <ResourceManager.h>
#include <ResourceFilesComponent.h>

#include <ServerInstanceBase.h>

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		fx::ResourceManager* resman = instance->GetComponent<fx::ResourceManager>().GetRef();

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("getConfiguration", [=](std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request)
		{
			json resources = json::array();

			std::string_view filterValues = postMap["resources"];
			std::set<std::string_view> filters;

			if (!filterValues.empty())
			{
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
				if (!filters.empty() && filters.find(resource->GetName()) == filters.end())
				{
					return;
				}

				if (resource->GetState() != fx::ResourceState::Started)
				{
					return;
				}

				json resourceFiles = json::object();
				fwRefContainer<fx::ResourceFilesComponent> files = resource->GetComponent<fx::ResourceFilesComponent>();

				for (const auto& entry : files->GetFileHashPairs())
				{
					resourceFiles[entry.first] = entry.second;
				}

				resources.push_back(json::object({
					{ "name", resource->GetName() },
					{ "files", resourceFiles },
					{ "streamFiles", json::object() }
				}));
			});

			return json::object({
				{ "fileServer", "http://%s/files" },
				{ "resources", resources }
			});
		});
	}, 5000);
});
