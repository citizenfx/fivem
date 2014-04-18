#include "StdInc.h"
#include "sysAllocator.h"

namespace rage
{
inline sysMemAllocator* GetAllocator()
{
	sysMemAllocator* allocator = *(sysMemAllocator**)(*(uintptr_t*)(__readfsdword(44)) + 8);

	return allocator;
}

void* sysUseAllocator::operator new(size_t size)
{
	return GetAllocator()->allocate(size, 16, 0);
}

void sysUseAllocator::operator delete(void* memory)
{
	GetAllocator()->free(memory);
}
}