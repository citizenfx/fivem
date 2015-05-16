/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

bool CoreIsDebuggerPresent()
{
	static bool(*func)();

	if (!func)
	{
		func = (bool(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreIsDebuggerPresent");
	}

	return (!func) ? false : func();
}

void CoreSetDebuggerPresent()
{
	static void(*func)();

	if (!func)
	{
		func = (void(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreSetDebuggerPresent");
	}

	(func) ? func() : 0;
}