/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "sysAllocator.h"

#include <Hooking.h>

namespace rage
{
static sysMemAllocator* g_gtaTlsEntry;

void* sysUseAllocator::operator new(size_t size)
{
	return GetAllocator()->allocate(size, 16, 0);
}

void sysUseAllocator::operator delete(void* memory)
{
	GetAllocator()->free(memory);
}

sysMemAllocator* sysMemAllocator::UpdateAllocatorValue()
{
	if (g_gtaTlsEntry)
	{
		*(sysMemAllocator**)(*(uintptr_t*)(__readfsdword(44)) + sysMemAllocator::GetAllocatorTlsOffset()) = g_gtaTlsEntry;
		//*(sysMemAllocator**)(*(uintptr_t*)(__readfsdword(44)) + sysMemAllocator::GetAllocatorTlsOffset() - 4) = g_gtaTlsEntry;

		return g_gtaTlsEntry;
	}

	return GetAllocator();
}
}

using rage::sysMemAllocator;

static DWORD WINAPI RageThreadHook(HANDLE hThread)
{
	// store the allocator
	rage::g_gtaTlsEntry = *(sysMemAllocator**)(*(uintptr_t*)(__readfsdword(44)) + sysMemAllocator::GetAllocatorTlsOffset());

	return ResumeThread(hThread);
}

static HookFunction hookFunction([]()
{
	auto location = hook::get_pattern("80 ? ? ? ? 74 07 56 FF 15", 8);
	hook::nop(location, 6);
	hook::call(location, RageThreadHook);
});

BOOL WINAPI DllMain(HANDLE, DWORD reason, LPVOID)
{
	if (reason == DLL_THREAD_ATTACH)
	{
		if (rage::g_gtaTlsEntry)
		{
			*(sysMemAllocator**)(*(uintptr_t*)(__readfsdword(44)) + sysMemAllocator::GetAllocatorTlsOffset()) = rage::g_gtaTlsEntry;
		}
	}

	return TRUE;
}
