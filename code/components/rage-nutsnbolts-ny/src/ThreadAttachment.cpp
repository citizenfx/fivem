/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

// the global allocator entry
static void* g_gtaTlsEntry;

static void RageThreadHook(void* param)
{
	// call the original function
	((void(*)(void*))0x524F30)(param);

	// store the allocator
	void** gtaTLS = *(void***)(__readfsdword(44) + (4 * *(uint32_t*)0x17237B0));

	g_gtaTlsEntry = gtaTLS[2];
}

static HookFunction hookFunction([] ()
{
	hook::call(0x5A87D8, RageThreadHook);
});

BOOL WINAPI DllMain(HANDLE, DWORD reason, LPVOID)
{
	if (reason == DLL_THREAD_ATTACH)
	{
		void** gtaTLS = *(void***)(__readfsdword(44) + (4 * *(uint32_t*)0x17237B0));

		if (g_gtaTlsEntry)
		{
			gtaTLS[2] = g_gtaTlsEntry;
			gtaTLS[6] = g_gtaTlsEntry;
		}
	}

	return TRUE;
}