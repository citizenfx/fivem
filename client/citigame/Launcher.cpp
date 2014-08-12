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

	HANDLE authDialog = OpenMutex(SYNCHRONIZE, FALSE, L"CitizenAuthDialog");

	if (authDialog)
	{
		// first give the auth process a chance to run (CEF won't initialize if this has already happened)
		Auth_RunProcess();

		CloseHandle(authDialog);
	}

	const wchar_t* mutexName = (wcsstr(GetCommandLine(), L"cl2")) ? L"CitizenFX2" : L"CitizenFX";

	HANDLE gameMutex = OpenMutex(SYNCHRONIZE, FALSE, mutexName);
	authDialog = CreateMutex(nullptr, TRUE, L"CitizenAuthDialog");

	if (!gameMutex)
	{
		// do NP initialization
		if (!Auth_VerifyIdentityEx("CitizenMP", true))
		{
			return false;
		}
	}
	else
	{
		CloseHandle(gameMutex);
	}

	CloseHandle(authDialog);

	gameMutex = CreateMutex(nullptr, true, mutexName);

	// now let NUI initialize (also do its process stuff if it is invoked, this should be done as early on as possible)
	if (nui::OnPreLoadGame(cefSandbox))
	{
		return false;
	}

	NP_Init();

	if (!NP_Connect("iv-platform.prod.citizen.re", 3036))
	{
		FatalError("Could not connect to the platform server at iv-platform.prod.citizen.re.");
	}

	auto async = NP_AuthenticateWithToken(Auth_GetSessionID());
	//auto async = NP_AuthenticateWithToken("mZBmANM/eRTf7qQELe37yLlPnahO2nqLCGyewVkcIVShQF2KYfONq89Pa+kkIGroHORablkhSc5zhL+MA1fEKLOOTl4jYXNauo18hVO1y9Y2sMs7lVGU48NO0YA1Npx7ToDpAaWLQU7aPsnDj/U=&&40366");
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