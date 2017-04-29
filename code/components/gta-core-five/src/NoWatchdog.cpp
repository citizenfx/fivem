/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

static HookFunction hookFunction([] ()
{
	// don't initialize some odd timer watchdog thread that crashes a *lot* of people (its only purpose seems to be to crash - and it ended up being the top
	// crasher on day one of game crash reporting being fixed)
	hook::return_function(hook::pattern("48 83 64 24 30 00 83 4C 24 28 FF 33 D2 48").count(1).get(0).get<void>(-4));
});