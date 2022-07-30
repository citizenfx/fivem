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
	g_gtaTlsEntry = *(sysMemAllocator**)(hook::get_tls() + sysMemAllocator::GetAllocatorTlsOffset());

	return ResumeThread(hThread);
}

static HookFunction hookFunction([] ()
{
	void* location = hook::pattern("48 8B CF FF D3 48 8B CF FF 15").count(1).get(0).get<void>(8);

	hook::nop(location, 6);
	hook::call(location, RageThreadHook);
});

sysMemAllocator* sysMemAllocator::UpdateAllocatorValue()
{
	assert(g_gtaTlsEntry);

	*(sysMemAllocator**)(hook::get_tls() + sysMemAllocator::GetAllocatorTlsOffset()) = g_gtaTlsEntry;

	return g_gtaTlsEntry;
}

BOOL WINAPI DllMain(HANDLE, DWORD reason, LPVOID)
{
	if (reason == DLL_THREAD_ATTACH)
	{
		if (g_gtaTlsEntry)
		{
			*(sysMemAllocator**)(hook::get_tls() + sysMemAllocator::GetAllocatorTlsOffset()) = g_gtaTlsEntry;
		}
	}

	return TRUE;
}
