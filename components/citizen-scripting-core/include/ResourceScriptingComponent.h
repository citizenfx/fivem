/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <fxScripting.h>

namespace fx
{
class Resource;

class ResourceScriptingComponent : public fwRefCountable
{
private:
	Resource* m_resource;

	fx::OMPtr<IScriptHost> m_scriptHost;

	std::map<int32_t, fx::OMPtr<IScriptRuntime>> m_scriptRuntimes;

private:
	void CreateEnvironments();

public:
	ResourceScriptingComponent(Resource* resource);

	inline Resource* GetResource()
	{
		return m_resource;
	}
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceScriptingComponent);