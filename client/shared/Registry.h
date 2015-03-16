/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifndef CORE_EXPORT
#ifdef COMPILING_CORE
#define CORE_EXPORT DLL_EXPORT
#else
#define CORE_EXPORT
#endif
#endif

#ifndef CORE_IMPORT
#ifdef COMPILING_CORE
#define CORE_IMPORT
#else
#define CORE_IMPORT DLL_IMPORT
#endif
#endif

class CORE_EXPORT InstanceRegistry : public fwRefCountable
{
private:
	std::unordered_map<std::string, void*> m_instanceMap;

public:
	void* GetInstance(const char* key);

	void SetInstance(const char* key, void* instance);
};

extern CORE_EXPORT CORE_IMPORT InstanceRegistry g_instanceRegistry;

template<class T>
class Instance
{
private:
	static const char* ms_name;
	static T* ms_cachedInstance;

public:
	static T* Get(InstanceRegistry* registry)
	{
		T* instance = static_cast<T*>(registry->GetInstance(ms_name));

		assert(instance != nullptr);

		return instance;
	}

	static T* Get()
	{
		if (!ms_cachedInstance)
		{
			ms_cachedInstance = Get(&g_instanceRegistry);
		}

		return ms_cachedInstance;
	}

	static void Set(T* instance)
	{
		Set(instance, &g_instanceRegistry);
	}

	static void Set(T* instance, InstanceRegistry* registry)
	{
		registry->SetInstance(ms_name, instance);
	}
};

#define DECLARE_INSTANCE_TYPE(name) \
	template<> __declspec(selectany) const char* ::Instance<name>::ms_name = #name; \
	template<> __declspec(selectany) name* ::Instance<name>::ms_cachedInstance = nullptr;