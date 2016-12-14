/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <nutsnbolts.h>

static HookFunction hookFunction([] ()
{
	char* location = hook::get_pattern<char>("0F 57 C9 48 2B C1 48 8D 0D", 9);
	uint32_t* legalScreenTime = (uint32_t*)(location + *(int32_t*)location + 4);

	*legalScreenTime = 0;
});