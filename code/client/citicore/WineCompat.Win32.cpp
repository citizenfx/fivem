/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

/*
 * WineCompat.Win32.cpp
 *
 * Installs a GetProcAddress hook that hides "wine_get_version" from any DLL
 * that queries it.  legitimacy.dll (and adhesive.dll) call GetProcAddress on
 * ntdll to detect Wine and alter their behaviour; by returning NULL we let
 * them run as they would on real Windows.
 *
 * Our own CfxIsWine() caches its result on first call, so it is unaffected
 * as long as it has been called before this hook is installed.
 *
 * The technique is identical to the Wine AppDB compatibility patches and the
 * Proton game-specific patches Valve ships for titles that denylist Wine.
 * Game-ownership validation is not affected: it proceeds via the Steam SDK
 * (which Proton implements natively) and Rockstar's ROS HTTP endpoints.
 */

#include "StdInc.h"

#ifndef IS_FXSERVER
#include "LaunchMode.h"
#include <MinHook.h>
#include <Hooking.Aux.h>

static FARPROC(WINAPI* g_origGetProcAddress)(HMODULE hModule, LPCSTR lpProcName);

static FARPROC WINAPI GetProcAddressWineHide(HMODULE hModule, LPCSTR lpProcName)
{
	// Ordinal imports use the high bits of the pointer rather than a name.
	if (reinterpret_cast<uintptr_t>(lpProcName) > 0xFFFF)
	{
		if (strcmp(lpProcName, "wine_get_version") == 0 ||
		    strcmp(lpProcName, "wine_get_build_id") == 0 ||
		    strcmp(lpProcName, "wine_get_host_version") == 0)
		{
			// Hide all Wine identity exports from every caller.
			// CfxIsWine() already cached its answer before this hook ran.
			return nullptr;
		}
	}

	return g_origGetProcAddress(hModule, lpProcName);
}

void WineCompat_InstallHooks()
{
	if (!CfxIsWine())
	{
		return;
	}

	// CfxIsWine() must have been called (and cached) before we install the
	// hook, otherwise our own detection breaks.  Assert that here in debug.
#if defined(_DEBUG)
	assert(CfxIsWine()); // ensures the cache is populated
#endif

	DisableToolHelpScope thScope;

	void* target = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetProcAddress");

	if (!target)
	{
		// kernel32 should always have GetProcAddress; fall back to kernelbase
		target = GetProcAddress(GetModuleHandleW(L"kernelbase.dll"), "GetProcAddress");
	}

	if (target)
	{
		MH_Initialize();
		MH_CreateHook(target, GetProcAddressWineHide, (void**)&g_origGetProcAddress);
		MH_EnableHook(target);
	}
}
#endif
