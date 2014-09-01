#include "StdInc.h"
#include "FxGameComponent.h"

std::string FxGameComponent::GetName()
{
	return "fx";
}

std::vector<ComponentId> FxGameComponent::GetProvides()
{
	std::vector<ComponentId> componentIds;
	componentIds.push_back(ComponentId::Parse("game:gta_ny[1.0.7.0]"));
	componentIds.push_back(ComponentId::Parse("fx[2]"));

	return componentIds;
}

fwRefContainer<FxGameComponent> FxGameComponent::Create()
{
	return new FxGameComponent();
}