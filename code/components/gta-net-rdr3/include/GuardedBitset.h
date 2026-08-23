#pragma once

#include <StdInc.h>

#include <Hooking.h>

inline void ApplyGuardedBitset(void* location, const uint8_t* bytes, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		hook::put<uint8_t>((uintptr_t)location + i, bytes[i]);
	}
}
