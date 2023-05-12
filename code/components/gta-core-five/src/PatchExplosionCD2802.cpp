#include <StdInc.h>
#include <Hooking.h>
#include "CrossBuildRuntime.h"

//
// This patch forces a tunable (seen starting at b2802) preventing creating
// explosions using native functions, returning the old behavior for compatibility.
//

static bool ReturnFalse()
{
	return false;
}

static HookFunction hookFunction([]()
{
	if (xbr::IsGameBuildOrGreater<2802>())
	{
		auto location = hook::get_pattern<char>("BA 04 09 18 CD B9");
		hook::call(location + 23, ReturnFalse);
	}
});
