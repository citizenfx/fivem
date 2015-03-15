/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "FxGameComponent.h"

std::string FxGameComponent::GetName()
{
	return "fx";
}

bool FxGameComponent::ComponentInstance::Initialize()
{
	return true;
}

bool FxGameComponent::ComponentInstance::Shutdown()
{
	return true;
}

Component* FxGameComponent::CreateComponent()
{
	return new ComponentInstance();
}

std::vector<ComponentId> FxGameComponent::GetDepends()
{
	return std::vector<ComponentId>();
}

std::vector<ComponentId> FxGameComponent::GetProvides()
{
	std::vector<ComponentId> componentIds;
	componentIds.push_back(ComponentId::Parse("game:gta_ny[1.0.7.0]"));
	componentIds.push_back(ComponentId::Parse("fx[2]"));
	
#if _WIN32
	componentIds.push_back(ComponentId::Parse("platform:windows"));
#endif

	return componentIds;
}

bool FxGameComponent::ShouldAutoInstance()
{
	return false;
}

fwRefContainer<FxGameComponent> FxGameComponent::Create()
{
	return new FxGameComponent();
}