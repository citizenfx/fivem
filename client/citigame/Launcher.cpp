#include "StdInc.h"
#include "LauncherInterface.h"
#include "Launcher.h"
#include "CrossLibraryInterfaces.h"
#include "net/HttpClient.h"
#include "ui/CefOverlay.h"
#include <libnp.h>

bool LauncherInterface::PreLoadGame(void* cefSandbox)
{
	bool continueRunning = true;

	HooksDLLInterface::PreGameLoad(&continueRunning, &g_hooksDLL);

	// first give the auth process a chance to run (CEF won't initialize if this has already happened)
	Auth_RunProcess();

	// now let NUI initialize (also do its process stuff if it is invoked, this should be done as early on as possible)
	if (nui::OnPreLoadGame(cefSandbox))
	{
		return false;
	}

	// do NP initialization
	if (!Auth_VerifyIdentityEx("CitizenMP", true))
	{
		return false;
	}

	NP_Init();

	if (!NP_Connect("iv-platform.prod.citizen.re", 3036))
	{
		FatalError("Could not connect to the platform server at iv-platform.prod.citizen.re.");
	}

	auto async = NP_AuthenticateWithToken(Auth_GetSessionID());
	auto result = async->Wait(15000);

	if (!result)
	{
		FatalError("Could not authenticate to the platform server at iv-platform.prod.citizen.re - operation timed out.");
	}

	if (result->result != AuthenticateResultOK)
	{
		FatalError("Could not authenticate to the platform server at iv-platform.prod.citizen.re - error %d.", result->result);
	}

	// and start running the game
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