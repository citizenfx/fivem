/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"

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
