#include "StdInc.h"
#include "Hooking.h"
#include <CrossBuildRuntime.h>

// This increase the harcoded limit of blip groups from 100 to 255 in pause menu map legend
static HookFunction hookFunction([]()
{
	// The limit no longer exists on game build 3258 and above
	if (xbr::IsGameBuildOrGreater<3258>())
	{
		return;
	}

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		auto location = hook::get_pattern("EB 03 45 33 E4 66 41 83 F8 64 73 29", 9);
		hook::put<uint8_t>(location, 0xFF);
	}
	else
	{
		auto location = hook::get_pattern("03 45 33 E4 66 83 3D ? ? ? ? 64 73", 11);
		hook::put<uint8_t>(location, 0xFF);
	}
});
