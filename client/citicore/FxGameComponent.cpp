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

	return componentIds;
}

fwRefContainer<FxGameComponent> FxGameComponent::Create()
{
	return new FxGameComponent();
}