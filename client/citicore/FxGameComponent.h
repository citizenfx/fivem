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
	virtual Component* CreateComponent();

	virtual std::vector<ComponentId> GetDepends();

	virtual std::vector<ComponentId> GetProvides();

	virtual std::string GetName();

	static fwRefContainer<FxGameComponent> Create();
};