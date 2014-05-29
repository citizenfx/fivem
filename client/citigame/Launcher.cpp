#include "StdInc.h"
#include "LauncherInterface.h"
#include "Launcher.h"
#include "CrossLibraryInterfaces.h"
#include "net/HttpClient.h"
#include "ui/CefOverlay.h"

bool LauncherInterface::PreLoadGame(void* cefSandbox)
{
	bool continueRunning = true;

	HooksDLLInterface::PreGameLoad(&continueRunning, &g_hooksDLL);

	if (nui::OnPreLoadGame(cefSandbox))
	{
		return false;
	}

	return continueRunning;
}

IGameSpecToHooks* g_hooksDLL;

bool LauncherInterface::PostLoadGame(HMODULE hModule, void(**entryPoint)())
{
	bool continueRunning = true;

	HooksDLLInterface::PostGameLoad(hModule, &continueRunning);
	InitFunctionBase::RunAll();

	*entryPoint = (void(*)())0xD0D011;

	return continueRunning;
}

static LauncherInterface g_launcherInterface;

extern "C" __declspec(dllexport) ILauncherInterface* GetLauncherInterface()
{
	return &g_launcherInterface;
}