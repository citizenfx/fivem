/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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