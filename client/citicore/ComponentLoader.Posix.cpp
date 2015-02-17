/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"

void* fwAlloc(size_t size)
{
	return malloc(size);
}

void fwFree(void* ptr)
{
	return free(ptr);
}