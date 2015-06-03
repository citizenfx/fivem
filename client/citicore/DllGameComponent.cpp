/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "DllGameComponent.h"

DllGameComponent::DllGameComponent(const pchar_t* path)
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

bool DllGameComponent::ShouldAutoInstance()
{
	auto iterator = m_document.FindMember("shouldAutoInstance");

	// we don't auto-instance in tool mode
	if (getenv("CitizenFX_ToolMode") != nullptr)
	{
		return false;
	}

	return (iterator != m_document.MemberEnd()) ? iterator->value.GetBool() : true;
}