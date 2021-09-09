/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <ICoreGameInit.h>
#include <LaunchMode.h>

uint32_t GetCurrentMapGroup()
{
	bool isSP = (CfxIsSinglePlayer() || Instance<ICoreGameInit>::Get()->HasVariable("storyMode"));
	return (isSP) ? HashString("GROUP_MAP_SP") : HashString("GROUP_MAP");
}

static void (*origEnableGroup)(void*, uint32_t);

static void _enableGroupHook(void* self, uint32_t group)
{
	origEnableGroup(self, GetCurrentMapGroup());
}

static HookFunction hookFunction([] ()
{
	{
		// replace default loading of GROUP_MAP_SP DLC with loading of GROUP_MAP (MP) DLC.
		auto location = hook::get_pattern<char>("75 0D BA E2 99 8F 57", 0);

		hook::nop(location, 2);
		hook::set_call(&origEnableGroup, location + 10);
		hook::call(location + 10, _enableGroupHook);
	}
});
