/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Registry.h"

void* InstanceRegistry::GetInstance(const char* key)
{
	auto it = m_instanceMap.find(key);
	void* instance = nullptr;

	if (it != m_instanceMap.end())
	{
		instance = it->second;
	}
	else
	{
		FatalError("Could not obtain instance from InstanceRegistry of type `%s`.", key);
	}
	
	return instance;
}

void InstanceRegistry::SetInstance(const char* key, void* instance)
{
	m_instanceMap[key] = instance;
}

// global registry for use by client-only modules
CORE_EXPORT InstanceRegistry g_instanceRegistry;