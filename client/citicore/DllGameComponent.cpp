/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "DllGameComponent.h"

DllGameComponent::DllGameComponent(const wchar_t* path)
	: m_path(path)
{
	ReadManifest();
}

std::string DllGameComponent::GetName()
{
	return m_document["name"].GetString();
}

std::vector<ComponentId> DllGameComponent::GetProvides()
{
	auto& provides = m_document["provides"];
	std::vector<ComponentId> componentIds;

	componentIds.push_back(ComponentId::Parse(va("%s[%s]", GetName().c_str(), m_document["version"].GetString())));
	
	for (auto it = provides.Begin(); it != provides.End(); it++)
	{
		componentIds.push_back(ComponentId::Parse(it->GetString()));
	}

	return componentIds;
}

std::vector<ComponentId> DllGameComponent::GetDepends()
{
	auto& depends = m_document["dependencies"];
	std::vector<ComponentId> componentIds;

	for (auto it = depends.Begin(); it != depends.End(); it++)
	{
		componentIds.push_back(ComponentId::Parse(it->GetString()));
	}

	return componentIds;
}

Component* DllGameComponent::CreateComponent()
{
	HMODULE hModule = LoadLibrary(m_path.c_str());

	if (!hModule)
	{
		return nullptr;
	}

	auto createComponent = (Component*(__cdecl*)())GetProcAddress(hModule, "CreateComponent");

	return createComponent ? createComponent() : nullptr;
}

void DllGameComponent::ReadManifest()
{
	HMODULE hModule = LoadLibraryEx(m_path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);

	if (!hModule)
	{
		return;
	}

	HRSRC hResource = FindResource(hModule, L"FXCOMPONENT", MAKEINTRESOURCE(935));
	
	if (hResource)
	{
		auto resSize = SizeofResource(hModule, hResource);
		auto resData = LoadResource(hModule, hResource);

		auto resPtr = static_cast<const char*>(LockResource(resData));

		// copy into a zero-terminated std::string
		std::string resourceString(resPtr, resSize);

		m_document.Parse(resourceString.c_str());
	}

	FreeLibrary(hModule);
}