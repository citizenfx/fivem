/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

// is done in sysAllocator.cpp
#if 0
#include "Hooking.h"

// the global allocator entry
static void* g_gtaTlsEntry;
static uint32_t* tlsIndex;

static void(*g_origThreadFunc)(void*);

static void RageThreadHook(void* param)
{
	// call the original function
	g_origThreadFunc(param);

	// store the allocator
	void** gtaTLS = *(void***)(__readfsdword(44) + (4 * *tlsIndex));

	g_gtaTlsEntry = gtaTLS[2];
}

static HookFunction hookFunction([] ()
{
	tlsIndex = *hook::get_pattern<uint32_t*>("89 91 08 17 00 00", 8);

	auto location = hook::get_pattern("0F 42 F0 E8 ? ? ? ? 83 C4 04", 3);
	hook::set_call(&g_origThreadFunc, location);
	hook::call(location, RageThreadHook);
});

BOOL WINAPI DllMain(HANDLE, DWORD reason, LPVOID)
{
	if (reason == DLL_THREAD_ATTACH)
	{
		if (tlsIndex)
		{
			void** gtaTLS = *(void***)(__readfsdword(44) + (4 * *tlsIndex));

			if (g_gtaTlsEntry)
			{
				gtaTLS[2] = g_gtaTlsEntry;
				gtaTLS[6] = g_gtaTlsEntry;
			}
		}
	}

	return TRUE;
}
#endif
