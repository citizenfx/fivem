#include "StdInc.h"
#include "ResourceUI.h"
#include <include/cef_origin_whitelist.h>

ResourceUI::ResourceUI(Resource* resource)
	: m_resource(resource)
{

}

ResourceUI::~ResourceUI()
{

}

bool ResourceUI::Create()
{
	auto& metaData = m_resource->GetMetaData();
	auto it = metaData.find("uiPage");

	CefRegisterSchemeHandlerFactory("http", m_resource->GetName().c_str(), nui::GetSchemeHandlerFactory());

	if (it == metaData.end())
	{
		return false;
	}

	fwString path = "nui://" + m_resource->GetName() + "/" + it->second;

	nui::CreateFrame(m_resource->GetName(), path);

	CefAddCrossOriginWhitelistEntry(va("nui://%s", m_resource->GetName().c_str()), "http", m_resource->GetName().c_str(), true);

	return true;
}

void ResourceUI::Destroy()
{
	nui::DestroyFrame(m_resource->GetName());
}

void ResourceUI::AddCallback(fwString type, ResUICallback callback)
{
	m_callbacks.insert(std::make_pair(type, callback));
}

bool ResourceUI::InvokeCallback(fwString type, fwString data, ResUIResultCallback resultCB)
{
	auto set = m_callbacks.equal_range(type);

	if (set.first == set.second)
	{
		return false;
	}

	std::for_each(set.first, set.second, [&] (std::pair<fwString, ResUICallback> cb)
	{
		cb.second(data, resultCB);
	});

	return true;
}

void ResourceUI::SignalPoll()
{
	nui::SignalPoll(m_resource->GetName());
}

static fwMap<fwString, fwRefContainer<ResourceUI>> g_resourceUIs;

fwRefContainer<ResourceUI> ResourceUI::GetForResource(fwRefContainer<Resource> resource)
{
	return g_resourceUIs[resource->GetName()];
}

static InitFunction initFunction([] ()
{
	Resource::OnStartingResource.Connect([] (fwRefContainer<Resource> resource)
	{
		auto ui = new ResourceUI(resource.GetRef());
		ui->Create();

		g_resourceUIs[resource->GetName()] = ui;
	});

	Resource::OnStoppingResource.Connect([] (fwRefContainer<Resource> resource)
	{
		auto uiRef = g_resourceUIs.find(resource->GetName());

		if (uiRef != g_resourceUIs.end())
		{
			uiRef->second->Destroy();
			g_resourceUIs.erase(uiRef);
		}
	});
});