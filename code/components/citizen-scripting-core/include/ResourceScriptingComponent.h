/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <fxScripting.h>

#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>

namespace fx
{
class Resource;

class ResourceScriptingComponent : public fwRefCountable
{
private:
	Resource* m_resource;

	fx::OMPtr<IScriptHost> m_scriptHost;

	tbb::concurrent_unordered_map<int32_t, fx::OMPtr<IScriptRuntime>> m_scriptRuntimes;

	tbb::concurrent_unordered_map<int32_t, fx::OMPtr<IScriptTickRuntime>> m_tickRuntimes;

	tbb::concurrent_unordered_set<std::string> m_eventsHandled;

private:
	void CreateEnvironments();

public:
	ResourceScriptingComponent(Resource* resource);

	fwEvent<> OnCreatedRuntimes;

	fwEvent<const std::string&, const std::string&> OnOpenScript;

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

	template<typename TFn>
	inline void ForAllRuntimes(const TFn&& fn)
	{
		for (auto& runtime : m_scriptRuntimes)
		{
			fn(runtime.second);
		}
	}

	inline Resource* GetResource()
	{
		return m_resource;
	}

	inline void AddHandledEvent(const std::string& eventName)
	{
		m_eventsHandled.insert(eventName);
	}
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceScriptingComponent);
