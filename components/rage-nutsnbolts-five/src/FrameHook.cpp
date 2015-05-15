/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <nutsnbolts.h>

#include <Hooking.h>

fwEvent<> OnGameFrame;

static int(*g_appState)(void* fsm, int state, void* unk, int type);

int DoAppState(void* fsm, int state, void* unk, int type)
{
	trace("App state: %d - %d\n", state, type);

	return g_appState(fsm, state, unk, type);
}

static void WaitThing(int i)
{
	trace("waiting from %p", _ReturnAddress());

	Sleep(i);
}

static bool(*g_origLookAlive)();

static bool OnLookAlive()
{
	OnGameFrame();

	return g_origLookAlive();
}

static HookFunction hookFunction([] ()
{
	void* lookAliveFrameCall = hook::pattern("48 81 EC 60 01 00 00 E8 ? ? ? ? 33 F6 48 8D").count(1).get(0).get<void>(7);

	hook::set_call(&g_origLookAlive, lookAliveFrameCall);
	hook::call(lookAliveFrameCall, OnLookAlive);

	auto waits = hook::pattern("EB 0F B9 21 00 00 00 E8").count(3);

	for (int i = 0; i < waits.size(); i++)
	{
		hook::call(waits.get(i).get<void>(7), WaitThing);
	}

	char* location = hook::pattern("66 89 41 16 83 C8 FF 4C 89 49 0C 44 89").count(1).get(0).get<char>(0xC - 0x2E);

	location = (char*)(location + *(int32_t*)location + 4);

	location += 32;

	void** vt = (void**)location;

	g_appState = (decltype(g_appState))vt[0];
	vt[0] = DoAppState;

	// temp nop of script handler mgr shutdown because of curiosity
	void* fpt = hook::pattern("48 8B CB FF 50 48 84 C0 74 23 48").count(1).get(0).get<void>(-0x24);
	hook::return_function(fpt);

	//__debugbreak();
});