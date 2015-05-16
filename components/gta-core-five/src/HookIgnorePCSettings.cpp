/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

static HookFunction hookFunction([] ()
{
	auto matches = hook::pattern("E8 ? ? ? ? EB 17 48 8D 15").count(2);

	for (int i = 0; i < matches.size(); i++)
	{
		char* location = matches.get(i).get<char>(-15);

		strcpy((char*)(location + *(int32_t*)location + 4), "fivem_set.bin");
	}
});