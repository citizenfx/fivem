#pragma once

#include <atPool.h>

#ifdef COMPILING_GTA_CORE_FIVE
#define GTA_CORE_EXPORT DLL_EXPORT
#else
#define GTA_CORE_EXPORT DLL_IMPORT
#endif

namespace rage
{
GTA_CORE_EXPORT atPoolBase* GetPoolBase(uint32_t hash);

GTA_CORE_EXPORT void* PoolAllocate(atPoolBase* pool);

GTA_CORE_EXPORT void PoolRelease(atPoolBase* pool, void* entry);

inline atPoolBase* GetPoolBase(const char* hashString)
{
	return GetPoolBase(HashString(hashString));
}

template<typename T>
inline atPool<T>* GetPool(uint32_t hash)
{
	return static_cast<atPool<T>*>(GetPoolBase(hash));
}

template<typename T>
inline atPool<T>* GetPool(const char* hashString)
{
	return static_cast<atPool<T>*>(GetPoolBase(HashString(hashString)));
}
}