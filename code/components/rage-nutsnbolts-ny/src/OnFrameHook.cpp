/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include "nutsnbolts.h"
#include <Hooking.h>
#include <mmsystem.h>

DLL_EXPORT fwEvent<> OnGameFrame;
DLL_EXPORT fwEvent<> OnCriticalGameFrame;
DLL_EXPORT fwEvent<> OnMainGameFrame;

static uint32_t g_lastGameFrame;
static uint32_t g_lastCriticalFrame;
static std::mutex g_gameFrameMutex;
static std::mutex g_criticalFrameMutex;
static DWORD g_mainThreadId;
static bool g_executedOnMainThread;

static void runFrame()
{
	if (g_gameFrameMutex.try_lock())
	{
		OnGameFrame();

		g_gameFrameMutex.unlock();
	}

	if (g_criticalFrameMutex.try_lock())
	{
		OnCriticalGameFrame();

		g_criticalFrameMutex.unlock();
	}

	if (GetCurrentThreadId() == g_mainThreadId)
	{
		OnMainGameFrame();

		g_executedOnMainThread = true;
	}

	g_lastGameFrame = timeGetTime();
	g_lastCriticalFrame = timeGetTime();
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

			if (g_criticalFrameMutex.try_lock())
			{
				OnCriticalGameFrame();

				g_criticalFrameMutex.unlock();
			}

			g_lastCriticalFrame = timeGetTime();
		}
	}
}

static HookFunction hookFunction([] ()
{
	g_mainThreadId = GetCurrentThreadId();

	hook::call(hook::get_pattern("E8 ? ? ? ? B9 ? ? ? ? 5E E9 ? ? ? ? CC CC"), runFrame);
	hook::call(hook::pattern("83 3F 00 76 ? FF D5 39").count(2).get(1).get<intptr_t>(-48), runFrame);

	if (wcsstr(GetCommandLine(), L"cl2"))
	{
		// don't whine about game already running for cl2
		hook::put(hook::get_pattern("47 54 41 4E 59 2D 30 38 38"), 'ATGN');
	}

	std::thread(RunCriticalGameLoop).detach();
});
