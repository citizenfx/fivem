/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

bool CoreIsDebuggerPresent()
{
	static auto func = (bool(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreIsDebuggerPresent");

	return func();
}

void CoreSetDebuggerPresent()
{
	static auto func = (void(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreSetDebuggerPresent");

	return func();
}