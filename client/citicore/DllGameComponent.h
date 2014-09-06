#pragma once

#include "ComponentLoader.h"
#include <rapidjson/document.h>

class DllGameComponent : public ComponentData
{
private:
	std::wstring m_path;

	rapidjson::Document m_document;

private:
	void ReadManifest();

public:
	DllGameComponent(const wchar_t* path);

	virtual std::string GetName();

	virtual std::vector<ComponentId> GetProvides();

	virtual std::vector<ComponentId> GetDepends();

	virtual Component* CreateComponent();
};