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

class ComponentRegistry
{
public:
	virtual size_t GetSize() = 0;

	virtual size_t RegisterComponent(const char* key) = 0;
};

#ifdef COMPILING_CORE
extern "C" CORE_EXPORT ComponentRegistry* CoreGetComponentRegistry();
#else
#ifndef _WIN32
#include <dlfcn.h>
#endif

static
#ifdef _MSC_VER
__declspec(noinline)
#endif
auto CoreGetComponentRegistry()
{
	static struct RefSource
	{
		RefSource()
		{
#ifdef _WIN32
			auto func = (ComponentRegistry*(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreGetComponentRegistry");
#else
			auto func = (ComponentRegistry*(*)())dlsym(dlopen("./libCoreRT.so", RTLD_LAZY), "CoreGetComponentRegistry");
#endif

			this->registry = func();
		}

		ComponentRegistry* registry;
	} registryRef;

	return registryRef.registry;
}
#endif

template<typename TContained>
class InstanceRegistryBase : public fwRefCountable
{
private:
	std::vector<TContained> m_instances;

public:
	InstanceRegistryBase()
	{
		EnsureSize();
	}

private:
	inline void EnsureSize()
	{
		m_instances.resize(CoreGetComponentRegistry()->GetSize());
	}

public:
	const TContained& GetInstance(size_t key)
	{
		EnsureSize();

		return m_instances[key];
	}

	void SetInstance(size_t key, const TContained& instance)
	{
		EnsureSize();

		m_instances[key] = instance;
	}
};

using InstanceRegistry = InstanceRegistryBase<void*>;
using RefInstanceRegistry = InstanceRegistryBase<fwRefContainer<fwRefCountable>>;

#ifdef COMPILING_CORE
extern "C" CORE_EXPORT InstanceRegistry* CoreGetGlobalInstanceRegistry();
#else
#ifndef _WIN32
#include <dlfcn.h>
#endif

static
#ifdef _MSC_VER
	__declspec(noinline)
#endif
	auto CoreGetGlobalInstanceRegistry()
{
    static struct RefSource
    {
        RefSource()
        {
#ifdef _WIN32
            auto func = (InstanceRegistry*(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreGetGlobalInstanceRegistry");
#else
			auto func = (InstanceRegistry*(*)())dlsym(dlopen("./libCoreRT.so", RTLD_LAZY), "CoreGetGlobalInstanceRegistry");
#endif
            
            this->registry = func();
        }

        InstanceRegistry* registry;
    } instanceRegistryRef;

    return instanceRegistryRef.registry;
}
#endif

template<class T>
class Instance
{
private:
	static const char* ms_name;
	static T* ms_cachedInstance;

public:
	static size_t ms_id;

public:
	static T* Get(InstanceRegistry* registry)
	{
		T* instance = static_cast<T*>(registry->GetInstance(ms_id));

		assert(instance != nullptr);

		return instance;
	}

	static const fwRefContainer<T>& Get(const fwRefContainer<RefInstanceRegistry>& registry)
	{
		// we're casting here to prevent invoking the fwRefContainer copy constructor
		// and ending up with a const reference to a temporary stack variable (as fwRefContainer<T> isn't fwRefContainer<fwRefCountable>).
		//
		// fwRefContainer of every type should have the exact same layout, and the instance registry will contain
		// instances of fwRefCountable, so any cast to a derived type *should* be safe.
		const fwRefContainer<T>& instance = *reinterpret_cast<const fwRefContainer<T>*>(&registry->GetInstance(ms_id));

		assert(instance.GetRef());

		return instance;
	}

	static T* Get()
	{
		if (!ms_cachedInstance)
		{
			ms_cachedInstance = Get(CoreGetGlobalInstanceRegistry());
		}

		return ms_cachedInstance;
	}

	static void Set(T* instance)
	{
		Set(instance, CoreGetGlobalInstanceRegistry());
	}

	static void Set(T* instance, InstanceRegistry* registry)
	{
		registry->SetInstance(ms_id, instance);
	}

	static void Set(const fwRefContainer<T>& instance, fwRefContainer<RefInstanceRegistry> registry)
	{
		registry->SetInstance(ms_id, instance);
	}

	static const char* GetName()
	{
		return ms_name;
	}
};

#ifdef _MSC_VER
#define SELECT_ANY __declspec(selectany)
#else
#define SELECT_ANY inline
#endif

#define TP_(x, y) x ## y
#define TP(x, y) TP_(x, y)

class ComponentRegistration
{
public:
	template<typename TFn>
	inline ComponentRegistration(TFn fn)
	{
		fn();
	}
};

#define DECLARE_INSTANCE_TYPE(name) \
	template<> SELECT_ANY const char* ::Instance<name>::ms_name = #name; \
	template<> SELECT_ANY name* ::Instance<name>::ms_cachedInstance = nullptr; \
	template<> SELECT_ANY size_t Instance<name>::ms_id; \
	static ComponentRegistration TP(_init_instance_, __COUNTER__)  ([] () { Instance<name>::ms_id = CoreGetComponentRegistry()->RegisterComponent(#name); });
