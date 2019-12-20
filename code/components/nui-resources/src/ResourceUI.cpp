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

#include <mutex>

ResourceUI::ResourceUI(Resource* resource)
	: m_resource(resource), m_hasFrame(false), m_hasCallbacks(false)
{

}

ResourceUI::~ResourceUI()
{

}

bool ResourceUI::Create()
{
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

	// initialize the page
	auto resourceName = m_resource->GetName();
	std::transform(resourceName.begin(), resourceName.end(), resourceName.begin(), ::ToLower);
	CefRegisterSchemeHandlerFactory("http", resourceName, Instance<NUISchemeHandlerFactory>::Get());
	CefRegisterSchemeHandlerFactory("https", resourceName, Instance<NUISchemeHandlerFactory>::Get());

	// create the NUI frame
	std::string path = "nui://" + m_resource->GetName() + "/" + pageName;
	nui::CreateFrame(m_resource->GetName(), path);

	// add a cross-origin entry to allow fetching the callback handler
	CefAddCrossOriginWhitelistEntry(va("nui://%s", m_resource->GetName().c_str()), "http", m_resource->GetName(), true);
	CefAddCrossOriginWhitelistEntry(va("nui://%s", m_resource->GetName().c_str()), "https", m_resource->GetName(), true);

	return true;
}

void ResourceUI::Destroy()
{
	// destroy the target frame
	nui::DestroyFrame(m_resource->GetName());
}

void ResourceUI::AddCallback(const std::string& type, ResUICallback callback)
{
	m_callbacks.insert({ type, callback });
}

bool ResourceUI::InvokeCallback(const std::string& type, const std::string& data, ResUIResultCallback resultCB)
{
	auto set = fx::GetIteratorView(m_callbacks.equal_range(type));

	if (set.begin() == set.end())
	{
		return false;
	}

	for (auto& cb : set)
	{
		cb.second(data, resultCB);
	}

	return true;
}

void ResourceUI::SignalPoll()
{
	nui::SignalPoll(m_resource->GetName());
}

static std::map<std::string, fwRefContainer<ResourceUI>> g_resourceUIs;
static std::mutex g_resourceUIMutex;

static InitFunction initFunction([] ()
{
	Resource::OnInitializeInstance.Connect([] (Resource* resource)
	{
		// create the UI instance
		fwRefContainer<ResourceUI> resourceUI(new ResourceUI(resource));

		// start event
		resource->OnCreate.Connect([=] ()
		{
			std::unique_lock<std::mutex> lock(g_resourceUIMutex);

			resourceUI->Create();
			g_resourceUIs[resource->GetName()] = resourceUI;
		});

		// stop event
		resource->OnStop.Connect([=] ()
		{
			std::unique_lock<std::mutex> lock(g_resourceUIMutex);

			if (g_resourceUIs.find(resource->GetName()) != g_resourceUIs.end())
			{
				resourceUI->Destroy();
				g_resourceUIs.erase(resource->GetName());
			}
		});

#ifdef GTA_FIVE
		// pre-disconnect handling
		resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnBeforeGameShutdown.Connect([=]()
		{
			std::unique_lock<std::mutex> lock(g_resourceUIMutex);

			if (g_resourceUIs.find(resource->GetName()) != g_resourceUIs.end())
			{
				resourceUI->Destroy();
				g_resourceUIs.erase(resource->GetName());
			}
		});
#endif

		// add component
		resource->SetComponent(resourceUI);
	});
});
