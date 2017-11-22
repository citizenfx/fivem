/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

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
	virtual Component* CreateComponent() override;

	virtual std::vector<ComponentId> GetDepends() override;

	virtual std::vector<ComponentId> GetProvides() override;

	virtual std::string GetName() override;

	virtual bool ShouldAutoInstance() override;

	static fwRefContainer<FxGameComponent> Create();
};