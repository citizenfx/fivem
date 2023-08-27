/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <CrossBuildRuntime.h>
#include <GlobalEvents.h>
#include <GameInit.h>

static void DoKill()
{
	OnKillNetwork("Disconnected.");

	OnMsgConfirm();
}

static void(*g_origTrigger)(uint32_t, uint32_t, uint32_t);

static void CustomTrigger(uint32_t trigger, uint32_t a2, uint32_t a3)
{
	// account picker
	if (trigger == 145)
	{
		DoKill();

		return;
	}

	g_origTrigger(trigger, a2, a3);
}

static void ReplayEditorExit()
{
	DoKill();
}

static HookFunction hookFunction([]()
{
	// pausemenu triggers
	{
		char* location;

		if (xbr::IsGameBuildOrGreater<2802>())
		{
			location = hook::pattern("48 8D 8D 18 01 00 00 41 BE 74 26 B5 9F").count(1).get(0).get<char>(-5);
		}
		else
		{
			location = hook::pattern("48 8D 8D 18 01 00 00 BE 74 26 B5 9F").count(1).get(0).get<char>(-5);
		}

		hook::set_call(&g_origTrigger, location);
		hook::call(location, CustomTrigger);
	
	}

	// same for R* editor exit
	if (!Is372())
	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 83 3D ? ? ? ? 08 75 05 E8 ? ? ? ? 83", 0x1A);
		hook::call(location, ReplayEditorExit);
	}
});
