/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "LauncherInterface.h"
#include "Launcher.h"
#include "CrossLibraryInterfaces.h"

#include <ComponentLoader.h>

IGameSpecToHooks* g_hooksDLL;

__declspec(dllexport) void SetHooksDll(IGameSpecToHooks* dll);

bool LauncherInterface::PreLoadGame(void* cefSandbox)
{
	bool continueRunning = true;

	// HooksDLL only exists for GTA_NY
#ifdef GTA_NY
	HooksDLLInterface::PreGameLoad(&continueRunning, &g_hooksDLL);
#endif

	SetHooksDll(g_hooksDLL);

	// make the component loader initialize
	ComponentLoader::GetInstance()->Initialize();

	// and start running the game
	return continueRunning;
}

bool LauncherInterface::PostLoadGame(HMODULE hModule, void(**entryPoint)())
{
	bool continueRunning = true;

	ComponentLoader::GetInstance()->DoGameLoad(hModule);

	// HooksDLL only exists for GTA_NY
#ifdef GTA_NY
	HooksDLLInterface::PostGameLoad(hModule, &continueRunning);
#endif

	InitFunctionBase::RunAll();

#if defined(GTA_NY)
	*entryPoint = (void(*)())0xD0D011;
#elif defined(PAYNE)
	// don't modify the entry point
	//*entryPoint = (void(*)())0;
#elif defined(GTA_FIVE)
#else
#error TODO: define entry point for this title
#endif

	return continueRunning;
}

static LauncherInterface g_launcherInterface;

extern "C" __declspec(dllexport) ILauncherInterface* GetLauncherInterface()
{
	return &g_launcherInterface;
}