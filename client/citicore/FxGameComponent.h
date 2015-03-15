/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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