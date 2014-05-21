#include "StdInc.h"
#include "ResourceUI.h"

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

	if (it == metaData.end())
	{
		return false;
	}

	std::string path = "nui://" + m_resource->GetName() + "/" + it->second;

	m_window = NUIWindow::Create(true, 2560, 1440, path);
	m_window->SetPaintType(NUIPaintTypePostRender);

	CefRegisterSchemeHandlerFactory("http", m_resource->GetName(), nui::GetSchemeHandlerFactory());

	return true;
}

void ResourceUI::Destroy()
{
	m_window = nullptr;
}

void ResourceUI::AddCallback(std::string type, ResUICallback callback)
{
	m_callbacks.insert(std::make_pair(type, callback));
}

bool ResourceUI::InvokeCallback(std::string type, std::string data, ResUIResultCallback resultCB)
{
	auto set = m_callbacks.equal_range(type);

	if (set.first == set.second)
	{
		return false;
	}

	std::for_each(set.first, set.second, [&] (std::pair<std::string, ResUICallback> cb)
	{
		cb.second(data, resultCB);
	});

	return true;
}