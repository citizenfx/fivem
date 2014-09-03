#pragma once
#include "ComponentLoader.h"

class FxGameComponent : public ComponentData
{
private:
	class ComponentInstance : public Component
	{
		virtual bool Initialize();

		virtual bool Shutdown();
	};

public:
	virtual Component* CreateComponent();

	virtual std::vector<ComponentId> GetDepends();

	virtual std::vector<ComponentId> GetProvides();

	virtual std::string GetName();

	static fwRefContainer<FxGameComponent> Create();
};