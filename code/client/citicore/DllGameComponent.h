/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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