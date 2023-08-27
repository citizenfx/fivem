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
		auto location = hook::get_pattern<char>("BA E7 8F A5 1E B9 BD C5 AF E3 E8");
		hook::call(location + 23, ReturnFalse);

		// This call was removed in 2802.0
		if (!xbr::IsGameBuildOrGreater<2802>())
		{
			hook::call(location + 61, ReturnFalse);
		}

		auto attachEntityToEntityTunable = hook::get_pattern<char>("BA 37 89 3D 8A B9 BD C5 AF E3");
		hook::call(attachEntityToEntityTunable + 23, ReturnFalse);
	}
});
