#pragma once
#include "ComponentLoader.h"

class FxGameComponent : public ComponentData
{
public:
	virtual std::vector<ComponentId> GetProvides();

	virtual std::string GetName();

	static fwRefContainer<FxGameComponent> Create();
};