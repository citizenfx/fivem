/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include <StdInc.h>
#include <Hooking.h>

#include <LaunchMode.h>

static HookFunction hookFunction([] ()
{
	if (CfxIsSinglePlayer())
	{
		return;
	}

	// replace default loading of GROUP_MAP_SP DLC in some cases with consistent loading of GROUP_MAP (MP) DLC.
	char* location = hook::pattern("75 0D BA E2 99 8F 57").count(1).get(0).get<char>(0);

	hook::nop(location, 2);
	hook::put(location + 3, 0xBCC89179); // GROUP_MAP
});
