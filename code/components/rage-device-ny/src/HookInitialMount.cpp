/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

// hook for the initial mountpoint of filesystem stuff
#include "StdInc.h"
#include <fiDevice.h>
#include <Hooking.h>

static HookFunction hookFunction([] ()
{
	static hook::inject_call<void, int> injectCall((ptrdiff_t)hook::get_pattern("8A 08 88 0C 02 8D 40 01 84 C9 75 ? 68 ? ? ? ? B9", 37));

	injectCall.inject([] (int)
	{
		static bool didInitialMount = false;

		injectCall.call();

		if (!didInitialMount)
		{
			rage::fiDevice::OnInitialMount();

			didInitialMount = true;
		}
	});
});
