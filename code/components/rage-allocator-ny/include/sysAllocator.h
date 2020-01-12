/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_RAGE_ALLOCATOR_NY
#define ALLOCATOR_EXPORT DLL_EXPORT
#else
#define ALLOCATOR_EXPORT DLL_IMPORT
#endif

namespace rage
{
class sysMemAllocator
{
public:
	virtual ~sysMemAllocator() = 0;

	virtual void m_4() = 0;
	virtual void* allocate(size_t size, size_t align, int subAllocator) = 0;
	virtual void free(void* pointer) = 0;

	// and a lot of other functions below that aren't needed right now
public:
	static inline uint32_t GetAllocatorTlsOffset()
	{
		return 8;
	};
	static ALLOCATOR_EXPORT sysMemAllocator* UpdateAllocatorValue();
};

inline sysMemAllocator* GetAllocator()
{
	sysMemAllocator* allocator = *(sysMemAllocator**)(*(uintptr_t*)(__readfsdword(44)) + sysMemAllocator::GetAllocatorTlsOffset());

	if (!allocator)
	{
		return sysMemAllocator::UpdateAllocatorValue();
	}

	return allocator;
}

class ALLOCATOR_EXPORT sysUseAllocator
{
public:
	void* operator new(size_t size);

	void operator delete(void* memory);
};
}
