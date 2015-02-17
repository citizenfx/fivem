/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"

static HANDLE hHeap;

#define BYTES_PER_PARA      16
#define PARAS_PER_PAGE      256     /*  tunable value */
#define BYTES_PER_PAGE      (BYTES_PER_PARA * PARAS_PER_PAGE)

__declspec(dllexport) void* fwAlloc(size_t size)
{
	if (!hHeap)
	{
		hHeap = HeapCreate(0, BYTES_PER_PAGE, 0);
	}

	return HeapAlloc(hHeap, 0, size);
}

__declspec(dllexport) void fwFree(void* ptr)
{
	if (ptr == 0)
	{
		return;
	}

	HeapFree(hHeap, 0, ptr);
}

#include "CrossLibraryInterfaces.h"

static IGameSpecToHooks* g_hooksDLL;

__declspec(dllexport) IGameSpecToHooks* GetHooksDll()
{
	return g_hooksDLL;
}

__declspec(dllexport) void SetHooksDll(IGameSpecToHooks* dll)
{
	g_hooksDLL = dll;
}