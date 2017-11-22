/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include "ComponentLoader.h"
#include <rapidjson/document.h>

class DllGameComponent : public ComponentData
{
private:
	fwPlatformString m_path;

	rapidjson::Document m_document;

private:
	void ReadManifest();

public:
	DllGameComponent(const pchar_t* path);

	virtual std::string GetName() override;

	virtual std::vector<ComponentId> GetProvides() override;

	virtual std::vector<ComponentId> GetDepends() override;

	virtual bool ShouldAutoInstance() override;

	virtual Component* CreateComponent() override;
};