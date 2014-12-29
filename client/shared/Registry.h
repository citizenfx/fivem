/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifndef CORE_EXPORT
#ifdef COMPILING_CORE
#define CORE_EXPORT __declspec(dllexport)
#else
#define CORE_EXPORT
#endif
#endif

class CORE_EXPORT InstanceRegistry
{
public:
	static void* GetInstance(const char* key);

	static void SetInstance(const char* key, void* instance);
};

template<class T>
class Instance
{
private:
	static const char* ms_name;
	static T* ms_cachedInstance;

public:
	static T* Get()
	{
		if (ms_cachedInstance)
		{
			return ms_cachedInstance;
		}

		T* instance = static_cast<T*>(InstanceRegistry::GetInstance(ms_name));
		ms_cachedInstance = instance;

		assert(instance != nullptr);

		return instance;
	}

	static void Set(T* instance)
	{
		InstanceRegistry::SetInstance(ms_name, instance);
	}
};

#define DECLARE_INSTANCE_TYPE(name) \
	__declspec(selectany) const char* Instance<name>::ms_name = #name; \
	__declspec(selectany) name* Instance<name>::ms_cachedInstance = nullptr;