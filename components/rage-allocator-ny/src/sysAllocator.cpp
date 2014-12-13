/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

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