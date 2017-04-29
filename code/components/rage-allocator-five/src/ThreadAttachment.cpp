/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include "sysAllocator.h"

using rage::sysMemAllocator;

// the global allocator entry
static sysMemAllocator* g_gtaTlsEntry;

static DWORD RageThreadHook(HANDLE hThread)
{
	// store the allocator
	g_gtaTlsEntry = *(sysMemAllocator**)(*(uintptr_t*)(__readgsqword(88)) + sysMemAllocator::GetAllocatorTlsOffset());

	return GetThreadId(hThread);
}

static HookFunction hookFunction([] ()
{
	void* location = hook::pattern("48 8B CF FF 15 ? ? ? ? 4C 8B 4D 50 4C 8B 45").count(1).get(0).get<void>(3);

	hook::nop(location, 6);
	hook::call(location, RageThreadHook);
});

sysMemAllocator* sysMemAllocator::UpdateAllocatorValue()
{
	assert(g_gtaTlsEntry);

	*(sysMemAllocator**)(*(uintptr_t*)(__readgsqword(88)) + sysMemAllocator::GetAllocatorTlsOffset()) = g_gtaTlsEntry;
	*(sysMemAllocator**)(*(uintptr_t*)(__readgsqword(88)) + sysMemAllocator::GetAllocatorTlsOffset() - 8) = g_gtaTlsEntry;

	return g_gtaTlsEntry;
}

BOOL WINAPI DllMain(HANDLE, DWORD reason, LPVOID)
{
	if (reason == DLL_THREAD_ATTACH)
	{
		if (g_gtaTlsEntry)
		{
			*(sysMemAllocator**)(*(uintptr_t*)(__readgsqword(88)) + sysMemAllocator::GetAllocatorTlsOffset()) = g_gtaTlsEntry;
			*(sysMemAllocator**)(*(uintptr_t*)(__readgsqword(88)) + sysMemAllocator::GetAllocatorTlsOffset() - 8) = g_gtaTlsEntry; // temp allocator is at 192
		}
	}

	return TRUE;
}