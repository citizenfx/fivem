/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Registry.h"

static std::unordered_map<std::string, void*> g_instanceMap;

void* InstanceRegistry::GetInstance(const char* key)
{
	auto it = g_instanceMap.find(key);
	void* instance = nullptr;

	if (it != g_instanceMap.end())
	{
		instance = it->second;
	}
	
	return instance;
}

void InstanceRegistry::SetInstance(const char* key, void* instance)
{
	assert(!GetInstance(key));

	g_instanceMap[key] = instance;
}