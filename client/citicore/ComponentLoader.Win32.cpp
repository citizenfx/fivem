/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
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