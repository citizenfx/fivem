/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"
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