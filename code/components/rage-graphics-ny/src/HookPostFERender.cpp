/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include "DrawCommands.h"
#include <MinHook.h>

static void(*g_origDrawFrontend)();

static uint8_t* shouldDrawFrontend;

static void DrawFrontendWrap()
{
	bool dwitf = true;

	DoWeIgnoreTheFrontend(dwitf);

	if (!dwitf)
	{
		OnPostFrontendRender();
	}

	// frontend flag
	if (*shouldDrawFrontend)
	{
		g_origDrawFrontend();
	}

	if (dwitf)
	{
		OnPostFrontendRender();
	}
}

static void(__thiscall* g_origGrcSetup_Present)(void*);

static void __fastcall grcSetup_PresentWrap(void* self)
{
	OnPostFrontendRender();

	g_origGrcSetup_Present(self);
}

static HookFunction hookFunction([] ()
{
	// always invoke frontend renderphase
	hook::put<uint8_t>(hook::get_pattern("6A 00 6A 10 68 50 09", -2), 0xEB);

	shouldDrawFrontend = *hook::get_pattern<uint8_t*>("55 8B EC 83 E4 F8 80 3D", 8);

	//hook::put(0xE9F1AC, DrawFrontendWrap);
	MH_Initialize();
	//MH_CreateHook(hook::get_pattern("CC 6A 00 6A 0C E8", 1), DrawFrontendWrap, (void**)&g_origDrawFrontend);
	MH_CreateHook(hook::get_pattern("8B 73 10 8B 7B 14 F3 0F 10 4B 24 2B C6", -0x13), grcSetup_PresentWrap, (void**)&g_origGrcSetup_Present);
	MH_EnableHook(MH_ALL_HOOKS);
});

DC_EXPORT fwEvent<bool&> DoWeIgnoreTheFrontend;
