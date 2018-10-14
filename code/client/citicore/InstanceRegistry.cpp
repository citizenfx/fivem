/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Registry.h"

// global registry for use by client-only modules
InstanceRegistry g_instanceRegistry;

InstanceRegistry* CoreGetGlobalInstanceRegistry()
{
    return &g_instanceRegistry;
}

// global component registry
class ComponentRegistryImpl : public ComponentRegistry
{
public:
	virtual size_t RegisterComponent(const char* key) override
	{
		auto it = m_components.find(key);

		if (it == m_components.end())
		{
			it = m_components.insert({ key, m_size++ }).first;
		}

		return it->second;
	}

	virtual size_t GetSize() override
	{
		return m_size;
	}

private:
	std::map<std::string, size_t> m_components;

	std::atomic<size_t> m_size{ 0 };
};

ComponentRegistry* CoreGetComponentRegistry()
{
	static ComponentRegistryImpl componentRegistry;

	return &componentRegistry;
}
