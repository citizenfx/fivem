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

static uint32_t g_tempAllocatorTlsOffset;

static DWORD RageThreadHook(HANDLE hThread)
{
	// store the allocator
	g_gtaTlsEntry = *(sysMemAllocator**)(hook::get_tls() + sysMemAllocator::GetAllocatorTlsOffset());

	return GetThreadId(hThread);
}

static HookFunction hookFunction([] ()
{
	void* location = hook::pattern("48 8B CF FF 15 ? ? ? ? 4C 8B 4D 50 4C 8B 45").count(1).get(0).get<void>(3);

	hook::nop(location, 6);
	hook::call(location, RageThreadHook);

	g_tempAllocatorTlsOffset = *hook::get_pattern<uint32_t>("4A 3B 1C 09 75 ? 49 8B 0C 08 48", -4);
});

sysMemAllocator* sysMemAllocator::UpdateAllocatorValue()
{
	assert(g_gtaTlsEntry);

	*(sysMemAllocator**)(hook::get_tls() + sysMemAllocator::GetAllocatorTlsOffset()) = g_gtaTlsEntry;
	*(sysMemAllocator**)(hook::get_tls() + g_tempAllocatorTlsOffset) = g_gtaTlsEntry;

	return g_gtaTlsEntry;
}

BOOL WINAPI DllMain(HANDLE, DWORD reason, LPVOID)
{
	if (reason == DLL_THREAD_ATTACH)
	{
		if (g_gtaTlsEntry)
		{
			*(sysMemAllocator**)(hook::get_tls() + sysMemAllocator::GetAllocatorTlsOffset()) = g_gtaTlsEntry;
			*(sysMemAllocator**)(hook::get_tls() + g_tempAllocatorTlsOffset) = g_gtaTlsEntry;
		}
	}

	return TRUE;
}
