/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <fxScripting.h>

#include <tbb/concurrent_unordered_map.h>

namespace fx
{
class Resource;

class ResourceScriptingComponent : public fwRefCountable
{
private:
	Resource* m_resource;

	fx::OMPtr<IScriptHost> m_scriptHost;

	tbb::concurrent_unordered_map<int32_t, fx::OMPtr<IScriptRuntime>> m_scriptRuntimes;

private:
	void CreateEnvironments();

public:
	ResourceScriptingComponent(Resource* resource);

	inline fx::OMPtr<IScriptHost> GetScriptHost()
	{
		return m_scriptHost;
	}

	inline void AddRuntime(OMPtr<IScriptRuntime> runtime)
	{
		m_scriptRuntimes.insert({ runtime->GetInstanceId(), runtime });
	}

	inline OMPtr<IScriptRuntime> GetRuntimeById(int32_t instanceId)
	{
		auto it = m_scriptRuntimes.find(instanceId);

		if (it == m_scriptRuntimes.end())
		{
			return nullptr;
		}

		return it->second;
	}

	inline Resource* GetResource()
	{
		return m_resource;
	}
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceScriptingComponent);
