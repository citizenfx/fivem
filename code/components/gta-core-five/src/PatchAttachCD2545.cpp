#include <StdInc.h>
#include <Hooking.h>
#include "CrossBuildRuntime.h"

//
// This patch forces a tunable (seen starting at b2545) preventing applying
// of ped-to-ped attachment data to networked peds to be 'off', returning the
// old behavior for compatibility.
//

static bool ReturnFalse()
{
	return false;
}

static HookFunction hookFunction([]()
{
	if (xbr::IsGameBuildOrGreater<2545>())
	{
		auto location = hook::get_pattern<char>("BA F4 39 90 CA B9 BD C5 AF");
		hook::call(location - 12, ReturnFalse);
		hook::call(location + 26, ReturnFalse);
	}
});
