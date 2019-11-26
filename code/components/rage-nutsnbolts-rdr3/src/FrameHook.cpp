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

fwEvent<> OnLookAliveFrame;
fwEvent<> OnGameFrame;
fwEvent<> OnMainGameFrame;
fwEvent<> OnCriticalGameFrame;
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
static uint32_t g_lastCriticalFrame;
static std::mutex* g_gameFrameMutex = new std::mutex();
static std::mutex* g_criticalFrameMutex = new std::mutex();
static DWORD g_mainThreadId;
static bool g_executedOnMainThread;

static void DoGameFrame()
{
	if (g_gameFrameMutex->try_lock())
	{
		OnGameFrame();

		g_gameFrameMutex->unlock();
	}

	if (g_criticalFrameMutex->try_lock())
	{
		OnCriticalGameFrame();

		g_criticalFrameMutex->unlock();
	}

	if (GetCurrentThreadId() == g_mainThreadId)
	{
		OnMainGameFrame();

		g_executedOnMainThread = true;
	}

	g_lastGameFrame = timeGetTime();
	g_lastCriticalFrame = timeGetTime();
}

// actually: 'should exit game' function called by LookAlive
static bool OnLookAlive()
{
/*	if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
	{
		DoGameFrame();
	}*/
	DoGameFrame();

	OnLookAliveFrame();

	return g_origLookAlive();
}

static bool(*g_origFrameFunc)(void*);

bool DoLoadsFrame(void* a)
{
	auto rv = g_origFrameFunc(a);

	int timeout = (g_executedOnMainThread) ? 500 : 50;

	if ((timeGetTime() - g_lastGameFrame) > timeout)
	{
		g_executedOnMainThread = false;

		DoGameFrame();
	}

	return rv;
}

static void RunCriticalGameLoop()
{
	while (true)
	{
		Sleep(50);

		int timeout = (g_executedOnMainThread) ? 500 : 50;

		if ((timeGetTime() - g_lastCriticalFrame) > timeout)
		{
			g_executedOnMainThread = false;

			if (g_criticalFrameMutex->try_lock())
			{
				OnCriticalGameFrame();

				g_criticalFrameMutex->unlock();
			}

			g_lastCriticalFrame = timeGetTime();
		}
	}
}

#include <MinHook.h>

static HookFunction hookFunction([]()
{
	g_mainThreadId = GetCurrentThreadId();

	//void* lookAliveFrameCall = hook::pattern("84 C0 75 05 40 84 FF 74 22 E8").count(1).get(0).get<void>(-5);
	void* lookAliveFrameCall = hook::pattern("84 C0 75 15 40 84 FF 75 10 48 8B").count(1).get(0).get<void>(-5);

	hook::set_call(&g_origLookAlive, lookAliveFrameCall);
	hook::call(lookAliveFrameCall, OnLookAlive);

	auto runState = hook::pattern("8B CA 8B F2 E8").count(1).get(0).get<char>(-0x15);
	MH_Initialize();
	MH_CreateHook(runState, DoAppState, (void**)&g_appState);
	MH_EnableHook(MH_ALL_HOOKS);

	// loading screen render thread function, to 'safely' handle game frames while loading (as a kind of watchdog)
	void* func = hook::pattern("B9 01 00 00 00 48 89 05 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84").count(1).get(0).get<void>(12);

	hook::set_call(&g_origFrameFunc, func);
	hook::call(func, DoLoadsFrame);

	std::thread(RunCriticalGameLoop).detach();

	//__debugbreak();
});
