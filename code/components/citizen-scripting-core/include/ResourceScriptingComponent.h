/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include <fxScripting.h>

#include <mutex>

namespace fx
{
class Resource;

class ResourceScriptingComponent : public fwRefCountable
{
private:
	Resource* m_resource;

	fx::OMPtr<IScriptHost> m_scriptHost;

	std::map<int32_t, fx::OMPtr<IScriptRuntime>> m_scriptRuntimes;

	std::recursive_mutex m_scriptRuntimesLock;

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
		std::unique_lock<std::recursive_mutex> lock(m_scriptRuntimesLock);

		m_scriptRuntimes.insert({ runtime->GetInstanceId(), runtime });
	}

	inline OMPtr<IScriptRuntime> GetRuntimeById(int32_t instanceId)
	{
		std::unique_lock<std::recursive_mutex> lock(m_scriptRuntimesLock);

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

	inline std::forward_list<fx::OMPtr<IScriptRuntime>> CollectScriptRuntimes()
	{
		std::unique_lock<std::recursive_mutex> lock(m_scriptRuntimesLock);

		std::forward_list<fx::OMPtr<IScriptRuntime>> runtimes;

		for (auto& runtime : m_scriptRuntimes)
		{
			runtimes.push_front(runtime.second);
		}

		return runtimes;
	}
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceScriptingComponent);
