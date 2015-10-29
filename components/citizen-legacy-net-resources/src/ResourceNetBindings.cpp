/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include <StdInc.h>
#include <ResourceManager.h>

#include <CachedResourceMounter.h>
#include <HttpClient.h>

#include <NetLibrary.h>

#include <nutsnbolts.h>

#include <rapidjson/document.h>

static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* netLibrary)
	{
		static std::mutex executeNextGameFrameMutex;
		static std::vector<std::function<void()>> executeNextGameFrame;

		// download trigger
		netLibrary->OnInitReceived.Connect([=] (NetAddress address)
		{
			// initialize mounter if needed
			static std::once_flag onceFlag;
			static fwRefContainer<fx::CachedResourceMounter> mounter;

			std::call_once(onceFlag, [=] ()
			{
				fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();
				mounter = fx::GetCachedResourceMounter(manager, "rescache:/");

				manager->AddMounter(mounter);
			});

			// fetch configuration
			std::shared_ptr<HttpClient> httpClient = std::make_shared<HttpClient>();

			// build request
			std::map<std::string, std::string> postMap;
			postMap["method"] = "getConfiguration";

			httpClient->DoPostRequest(address.GetWAddress(), address.GetPort(), L"/client", postMap, [=] (bool result, const char* data, size_t size)
			{
				// keep a reference to the HTTP client
				auto httpClientRef = httpClient;
				auto addressClone = address; // due to non-const-safety

				// handle failure
				if (!result)
				{
					GlobalError("Obtaining configuration from server (%s) failed.", addressClone.GetAddress().c_str());

					return;
				}

				// 'get' the server host
				std::string serverHost = addressClone.GetAddress() + va(":%d", addressClone.GetPort());

				// start parsing the result
				rapidjson::Document node;
				node.Parse(data);

				if (node.HasParseError())
				{
					auto err = node.GetParseError();
					GlobalError("parse error %d", err);

					return;
				}

				// more stuff from downloadmgr
				bool hasValidResources = true;

				if (!node.HasMember("resources") || !node["resources"].IsArray())
				{
					hasValidResources = false;
				}

				if (hasValidResources)
				{
					if (!node.HasMember("fileServer") || !node["fileServer"].IsString())
					{
						hasValidResources = false;
					}
				}

				std::vector<std::string> requiredResources;

				if (hasValidResources)
				{
					auto& resources = node["resources"];

					std::string origBaseUrl = node["fileServer"].GetString();

					for (auto it = resources.Begin(); it != resources.End(); it++)
					{
						auto& resource = *it;

						std::string baseUrl = origBaseUrl;

						if (it->HasMember("fileServer") && (*it)["fileServer"].IsString())
						{
							baseUrl = (*it)["fileServer"].GetString();
						}

						// define the resource in the mounter
						std::string resourceName = resource["name"].GetString();

						// this is old NY stuff
						if (resourceName == "mapmanager")
						{
							continue;
						}

						std::string resourceBaseUrl = va("%s/%s/", va(baseUrl.c_str(), serverHost.c_str()), resourceName.c_str());

						auto& files = resource["files"];
						for (auto i = files.MemberBegin(); i != files.MemberEnd(); i++)
						{
							fwString filename = i->name.GetString();

							mounter->AddResourceEntry(resourceName, filename, i->value.GetString(), resourceBaseUrl + filename);
						}

						trace("[%s]\n", resourceName.c_str());

						requiredResources.push_back(resourceName);
					}
				}

				fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();

				for (auto& resourceName : requiredResources)
				{
					manager->AddResource("global://" + resourceName).then([=] (fwRefContainer<fx::Resource> resource)
					{
						if (!resource.GetRef())
						{
							GlobalError("Couldn't load resource %s. :(", resourceName.c_str());

							return;
						}

						std::unique_lock<std::mutex> lock(executeNextGameFrameMutex);
						executeNextGameFrame.push_back([=] ()
						{
							if (!resource->Start())
							{
								GlobalError("Couldn't start resource %s. :(", resourceName.c_str());
							}
						});
					});
				}
			});
		});

		OnGameFrame.Connect([] ()
		{
			std::unique_lock<std::mutex> lock(executeNextGameFrameMutex);

			for (auto& func : executeNextGameFrame)
			{
				func();
			}

			executeNextGameFrame.clear();
		});
	});
});
/*


manager->AddMounter(fx::GetCachedResourceMounter(manager.GetRef(), "rescache:/"));
*/