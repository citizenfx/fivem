/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <CefOverlay.h>

static void ExitSinglePlayerGame(int)
{
	// disable main UI
	nui::SetMainUI(true);
	nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");

	// force loading screen
	((void(*)(bool))0xA6E7E0)(true);

	// reset network flag
	hook::put<uint32_t>(0x10F8070, 1);
}

static HookFunction hookFunction([] ()
{
	hook::nop(0x40DAA8, 5);
	hook::call(0x40DAAF, ExitSinglePlayerGame);

	// saved game protected data validation (from xliveless)
	hook::nop(0x5B06E5, 2);
});