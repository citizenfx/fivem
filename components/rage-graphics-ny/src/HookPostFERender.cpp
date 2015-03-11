/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include "DrawCommands.h"

static void DrawFrontendWrap()
{
	bool dwitf = false;

	DoWeIgnoreTheFrontend(dwitf);

	if (!dwitf)
	{
		OnPostFrontendRender();
	}

	// frontend flag
	if (*(uint8_t*)0x10C7F6F)
	{
		((void(*)())0x44CCD0)();
	}

	if (dwitf)
	{
		OnPostFrontendRender();
	}
}

static HookFunction hookFunction([] ()
{
	// always invoke frontend renderphase
	hook::put<uint8_t>(0x43AF21, 0xEB);

	hook::put(0xE9F1AC, DrawFrontendWrap);
});

DC_EXPORT fwEvent<bool&> DoWeIgnoreTheFrontend;