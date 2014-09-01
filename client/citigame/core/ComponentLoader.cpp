#include "StdInc.h"
#include "ComponentLoader.h"
#include "FxGameComponent.h"

void ComponentLoader::Initialize()
{
	// set up the root component
	m_rootComponent = FxGameComponent::Create();

	AddComponent(m_rootComponent);
}

void ComponentLoader::AddComponent(fwRefContainer<ComponentData> component)
{
	std::string name = component->GetName();

	m_knownComponents.insert(std::make_pair(name, component));
	m_loadedComponents.push_back(name);
}