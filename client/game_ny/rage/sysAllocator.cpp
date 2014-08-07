#include "StdInc.h"
#include "sysAllocator.h"

namespace rage
{
void* sysUseAllocator::operator new(size_t size)
{
	return GetAllocator()->allocate(size, 16, 0);
}

void sysUseAllocator::operator delete(void* memory)
{
	GetAllocator()->free(memory);
}
}