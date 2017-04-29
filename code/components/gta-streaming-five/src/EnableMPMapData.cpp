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
	// replace default loading of GROUP_MAP_SP DLC in some cases with consistent loading of GROUP_MAP (MP) DLC.
	char* location = hook::pattern("75 0D BA E2 99 8F 57").count(1).get(0).get<char>(0);

	hook::nop(location, 2);
	hook::put(location + 3, 0xBCC89179); // GROUP_MAP
});