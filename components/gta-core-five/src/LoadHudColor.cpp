/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <nutsnbolts.h>

static hook::cdecl_stub<void()> loadHudColor([] ()
{
	return hook::pattern("45 33 F6 41 8D 56 27 44 89").count(1).get(0).get<void>(-0x23);
});

static bool LoadHudColorWrap()
{
	__try
	{
		loadHudColor();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	return true;
}

static InitFunction initFunction([] ()
{
	OnGameFrame.Connect([] ()
	{
		static bool loadedHudColor = false;

		if (!loadedHudColor)
		{
			// really bad way of doing this, but for now it should be fine
			if (LoadHudColorWrap())
			{
				loadedHudColor = true;
			}
		}
	});
});