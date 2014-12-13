/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

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
};

inline sysMemAllocator* GetAllocator()
{
	sysMemAllocator* allocator = *(sysMemAllocator**)(*(uintptr_t*)(__readfsdword(44)) + 8);

	return allocator;
}

class GAMESPEC_EXPORT sysUseAllocator
{
public:
	void* operator new(size_t size);

	void operator delete(void* memory);
};
}