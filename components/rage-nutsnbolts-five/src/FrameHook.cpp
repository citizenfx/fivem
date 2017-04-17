/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <nutsnbolts.h>

#include <Hooking.h>

#include <ICoreGameInit.h>

fwEvent<> OnGameFrame;
fwEvent<> OnMainGameFrame;
fwEvent<> OnFirstLoadCompleted;

static int(*g_appState)(void* fsm, int state, void* unk, int type);

int DoAppState(void* fsm, int state, void* unk, int type)
{
	static bool firstLoadCompleted = false;

	if (!firstLoadCompleted && state == 2 && type == 1)
	{
		OnFirstLoadCompleted();

		firstLoadCompleted = true;
	}

	return g_appState(fsm, state, unk, type);
}

static void WaitThing(int i)
{
	trace("waiting from %p\n", _ReturnAddress());

	Sleep(i);
}

static bool(*g_origLookAlive)();

#include <mutex>
#include <mmsystem.h>

static uint32_t g_lastGameFrame;
static std::mutex g_gameFrameMutex;
static DWORD g_mainThreadId;
static bool g_executedOnMainThread;

static void DoGameFrame()
{
	if (g_gameFrameMutex.try_lock())
	{
		OnGameFrame();

		g_gameFrameMutex.unlock();
	}

	if (GetCurrentThreadId() == g_mainThreadId)
	{
		OnMainGameFrame();

		g_executedOnMainThread = true;
	}

	g_lastGameFrame = timeGetTime();
}

static bool* g_isD3DInvalid;

// actually: 'should exit game' function called by LookAlive
static bool OnLookAlive()
{
	if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
	{
		DoGameFrame();
	}

	return g_origLookAlive();
}

static void(*g_origFrameFunc)();

void DoLoadsFrame()
{
	g_origFrameFunc();

	int timeout = (g_executedOnMainThread) ? 500 : 50;

	if ((timeGetTime() - g_lastGameFrame) > timeout)
	{
		g_executedOnMainThread = false;

		DoGameFrame();
	}
}

static HookFunction hookFunction([] ()
{
	g_mainThreadId = GetCurrentThreadId();

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

	// loading screen render thread function, to 'safely' handle game frames while loading (as a kind of watchdog)
	void* func = hook::pattern("83 FB 0A 0F 85 80 00 00 00 8B").count(1).get(0).get<void>(-17);
	
	hook::set_call(&g_origFrameFunc, func);
	hook::call(func, DoLoadsFrame);

	location = hook::get_pattern<char>("84 C9 74 0D 38 05 ? ? ? ? 75 05 B8 01 00", 6);

	g_isD3DInvalid = (bool*)(location + *(int32_t*)location + 4);

	// allow resizing window in all cases
	hook::nop(hook::get_pattern("45 8D 67 01 74 05 41 8B C4", 4), 2);

	//__debugbreak();
});