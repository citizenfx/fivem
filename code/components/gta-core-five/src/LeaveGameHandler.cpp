/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "Hooking.h"

#include <GlobalEvents.h>
#include <GameInit.h>

static void(*g_origTrigger)(uint32_t, uint32_t, uint32_t);

static void CustomTrigger(uint32_t trigger, uint32_t a2, uint32_t a3)
{
	// account picker
	if (trigger == 145)
	{
		OnKillNetwork("Disconnected.");

		OnMsgConfirm();

		return;
	}

	g_origTrigger(trigger, a2, a3);
}

static HookFunction hookFunction([] ()
{
	// pausemenu triggers
	void* call = hook::pattern("48 8D 8D 18 01 00 00 BE 74 26 B5 9F").count(1).get(0).get<void>(-5);

	hook::set_call(&g_origTrigger, call);
	hook::call(call, CustomTrigger);
});