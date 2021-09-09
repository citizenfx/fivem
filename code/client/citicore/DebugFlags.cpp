/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <CrossBuildRuntime.h>

static bool g_isDebuggerPresent;

bool CoreIsDebuggerPresent()
{
	return g_isDebuggerPresent;
}

void CoreSetDebuggerPresent()
{
#ifdef _WIN32
	g_isDebuggerPresent = IsDebuggerPresent();
#endif
}

#ifdef _WIN32
// #TODO: maybe store this in a HostSharedData?
static HWND g_gameWindow;

void CoreSetGameWindow(HWND hWnd)
{
	g_gameWindow = hWnd;
}

HWND CoreGetGameWindow()
{
	if (!g_gameWindow)
	{
		static HWND g_fallbackGameWindow;

		if (!g_fallbackGameWindow)
		{
			g_fallbackGameWindow = FindWindowW(xbr::GetGameWndClass(), nullptr);
		}
		
		return g_fallbackGameWindow;
	}

	return g_gameWindow;
}
#endif
