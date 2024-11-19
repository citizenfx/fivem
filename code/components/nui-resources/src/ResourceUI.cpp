/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceUI.h"
#include <include/cef_origin_whitelist.h>

#include <NUISchemeHandlerFactory.h>

#include <ResourceMetaDataComponent.h>
#include <ResourceGameLifetimeEvents.h>

#include <ResourceManager.h>

#include <boost/algorithm/string/predicate.hpp>
#include <mutex>

ResourceUI::ResourceUI(Resource* resource)
	: m_resource(resource), m_hasFrame(false), m_hasCallbacks(false), m_strictModeOptedIn(false)
{

}

ResourceUI::~ResourceUI()
{

}

bool ResourceUI::Create()
{
	m_isDead = false;

	// initialize callback handlers
	auto resourceName = m_resource->GetName();
	std::transform(resourceName.begin(), resourceName.end(), resourceName.begin(), ::ToLower);
	nui::RegisterSchemeHandlerFactory("http", resourceName, Instance<NUISchemeHandlerFactory>::Get());
	nui::RegisterSchemeHandlerFactory("https", resourceName, Instance<NUISchemeHandlerFactory>::Get());
	nui::RegisterSchemeHandlerFactory("https", "cfx-nui-" + resourceName, Instance<NUISchemeHandlerFactory>::Get());

	// get the metadata component
	fwRefContainer<fx::ResourceMetaDataComponent> metaData = m_resource->GetComponent<fx::ResourceMetaDataComponent>();

	// get the UI page list and a number of pages
	auto uiPageData = metaData->GetEntries("ui_page");
	int pageCount = std::distance(uiPageData.begin(), uiPageData.end());

	// if no page exists, return
	if (pageCount == 0)
	{
		return false;
	}

	// if more than one, warn
	if (pageCount > 1)
	{
		trace(__FUNCTION__ ": more than one ui_page in resource %s\n", m_resource->GetName().c_str());
		return false;
	}

	// mark us as having a frame
	m_hasFrame = true;

	// get the page name from the iterator
	std::string pageName = uiPageData.begin()->second;

	// create the NUI frame
	auto rmvRes = metaData->IsManifestVersionBetween("cerulean", "");
	auto uiPrefix = (!rmvRes || !*rmvRes) ? "nui://" : "https://cfx-nui-";

	std::string path = uiPrefix + m_resource->GetName() + "/" + pageName;

	// allow direct mentions of absolute URIs
	if (pageName.find("://") != std::string::npos)
	{
		path = pageName;
	}

	auto uiPagePreloadData = metaData->GetEntries("ui_page_preload");
	bool uiPagePreload = std::distance(uiPagePreloadData.begin(), uiPagePreloadData.end()) > 0;

	if (uiPagePreload)
	{
		nui::CreateFrame(m_resource->GetName(), path);
	}
	else
	{
		nui::PrepareFrame(m_resource->GetName(), path);
	}

	// check if the resource opts into strict mode
	auto strictModeData = metaData->GetEntries("nui_callback_strict_mode");
	if (std::distance(uiPageData.begin(), uiPageData.end()) == 1 && strictModeData.begin()->second == "true")
	{
		m_strictModeOptedIn = true;
	}

	// add a cross-origin entry to allow fetching the callback handler
	CefAddCrossOriginWhitelistEntry(va("nui://%s", m_resource->GetName().c_str()), "http", m_resource->GetName(), true);
	CefAddCrossOriginWhitelistEntry(va("nui://%s", m_resource->GetName().c_str()), "https", m_resource->GetName(), true);
	
	CefAddCrossOriginWhitelistEntry(va("https://cfx-nui-%s", m_resource->GetName().c_str()), "https", m_resource->GetName(), true);
	CefAddCrossOriginWhitelistEntry(va("https://cfx-nui-%s", m_resource->GetName().c_str()), "nui", m_resource->GetName(), true);

	return true;
}

void ResourceUI::Destroy()
{
	m_isDead = true;

	if (m_hasFrame)
	{
		// destroy the target frame
		nui::DestroyFrame(m_resource->GetName());

		// mark as no frame
		m_hasFrame = false;
	}
}

void ResourceUI::AddCallback(const std::string& type, ResUICallback callback)
{
	m_callbacks.insert({ type, callback });
}

void ResourceUI::RemoveCallback(const std::string& type)
{
	// Note: This is called by UNREGISTER_RAW_NUI_CALLBACK but
	// can still technically target event based NUI Callbacks
	m_callbacks.erase(type);
}

static std::mutex g_nuiCallbackMutex;
static std::queue<std::function<void()>> g_nuiCallbackQueue;

bool ResourceUI::InvokeCallback(const std::string& type, const std::string& query, const std::multimap<std::string, std::string>& headers, const std::string& data, ResUIResultCallback resultCB)
{
	auto set = fx::GetIteratorView(m_callbacks.equal_range(type));

	if (set.begin() == set.end())
	{
		// try mapping only the first part
		auto firstType = type.substr(0, type.find_first_of('/'));

		set = fx::GetIteratorView(m_callbacks.equal_range(firstType));

		if (set.begin() == set.end())
		{
			return false;
		}
	}

	// origin lookup
	auto originIts = headers.equal_range("Origin");
	std::optional<std::string> originResource;
	if (originIts.first != originIts.second)
	{
		// there should only be one, so take the first
		const std::string& originPath = originIts.first->second;

		constexpr char prefixNui[] = "nui://";
		constexpr char prefixHttp[] = "https://cfx-nui-";

		if (originPath.rfind(prefixNui, 0) == 0)
		{
			originResource = originPath.substr(std::size(prefixNui) - 1);
		}
		else if (originPath.rfind(prefixHttp, 0) == 0)
		{
			originResource = originPath.substr(std::size(prefixHttp) - 1);
		}
	}

	if (m_strictModeOptedIn && (!originResource.has_value() || m_resource->GetName() != originResource.value()))
	{
		trace(__FUNCTION__ ": call to '%s/%s' from '%s' has been blocked by NUI Callback Strict Mode\n",
		m_resource->GetName(),
		type,
		originResource.has_value() ? originResource.value() : "(null)");
		return false;
	}


	std::vector<ResUICallback> cbSet;

	for (auto& cb : set)
	{
		cbSet.push_back(cb.second);
	}

	fwRefContainer selfRef = this;

	std::function<void()> cb = [selfRef, cbSet = std::move(cbSet), type, query, headers, data, originResource, resultCB = std::move(resultCB)]()
	{
		if (selfRef->IsDead())
		{
			return;
		}

		for (const auto& cb : cbSet)
		{
			cb(type, query, headers, data, originResource, resultCB);
		}
	};

	std::unique_lock _(g_nuiCallbackMutex);
	g_nuiCallbackQueue.push(std::move(cb));

	return true;
}

void ResourceUI::SignalPoll()
{
	nui::SignalPoll(m_resource->GetName());
}

#include <boost/algorithm/string.hpp>

static bool NameMatches(std::string_view name, std::string_view match)
{
	if (name == match)
	{
		return true;
	}

	if (boost::algorithm::starts_with(name, match))
	{
		if (name[match.length()] == '/')
		{
			return true;
		}
	}

	return false;
}

static InitFunction initFunction([] ()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		manager->OnTick.Connect([]()
		{
			auto pop = []() -> std::function<void()>
			{
				std::unique_lock _(g_nuiCallbackMutex);
				if (!g_nuiCallbackQueue.empty())
				{
					auto fn = std::move(g_nuiCallbackQueue.front());
					g_nuiCallbackQueue.pop();

					return std::move(fn);
				}

				return {};
			};

			while (auto fn = pop())
			{
				fn();
			}
		}, INT32_MAX);

		nui::SetResourceLookupFunction([manager](const std::string& resourceName, const std::string& fileName) -> std::string
		{
			fwRefContainer<fx::Resource> resource;

			fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
			resourceManager->ForAllResources([&resourceName, &resource](const fwRefContainer<fx::Resource>& resourceRef)
			{
				if (_stricmp(resourceRef->GetName().c_str(), resourceName.c_str()) == 0)
				{
					resource = resourceRef;
				}
			});

			if (resource.GetRef())
			{
				auto path = resource->GetPath();

				if (!boost::algorithm::ends_with(path, "/"))
				{
					path += "/";
				}

				// check if it's a client script of any sorts
				std::stringstream normalFileName;
				char lastC = '/';

				for (size_t i = 0; i < fileName.length(); i++)
				{
					char c = fileName[i];

					if (c != '/' || lastC != '/')
					{
						normalFileName << c;
					}

					lastC = c;
				}

				auto nfn = normalFileName.str();

				auto mdComponent = resource->GetComponent<fx::ResourceMetaDataComponent>();
				bool valid = false;

				if (NameMatches(nfn, "__resource.lua") || NameMatches(nfn, "fxmanifest.lua"))
				{
					return "common:/data/gameconfig.xml";
				}
				
				for (auto& entry : mdComponent->GlobEntriesVector("client_script"))
				{
					if (NameMatches(nfn, entry))
					{
						auto files = mdComponent->GlobEntriesVector("file");
						bool isFile = false;

						for (auto& fileEntry : files)
						{
							if (NameMatches(nfn, fileEntry))
							{
								isFile = true;
								break;
							}
						}

						if (!isFile)
						{
							return "common:/data/gameconfig.xml";
						}
					}
				}

				return path + fileName;
			}

			return fmt::sprintf("resources:/%s/%s", resourceName, fileName);
		});
	});

	fx::Resource::OnInitializeInstance.Connect([] (Resource* resource)
	{
		// create the UI instance
		fwRefContainer<ResourceUI> resourceUI(new ResourceUI(resource));
		resource->SetComponent(resourceUI);

		// start event
		resource->OnCreate.Connect([resource]()
		{
			resource->GetComponent<ResourceUI>()->Create();
		});

		// stop event
		resource->OnStop.Connect([resource] ()
		{
			resource->GetComponent<ResourceUI>()->Destroy();
		});

		// pre-disconnect handling
		resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnBeforeGameShutdown.Connect([resource]()
		{
			resource->GetComponent<ResourceUI>()->Destroy();
		});
	});
});

#include <CefOverlay.h>
#include <HttpClient.h>

static InitFunction httpInitFunction([]()
{
	nui::RequestNUIBlocklist.Connect([](std::function<void(bool, const char*, size_t)> cb)
	{
		auto httpClient = Instance<HttpClient>::Get();
		httpClient->DoGetRequest("https://api.vmp.ir/nui-blacklist.json", cb);
	});
});
