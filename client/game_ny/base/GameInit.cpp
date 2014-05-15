#include "StdInc.h"
#include "GameInit.h"
#include "Text.h"
#include "Hooking.h"
#include "CrossLibraryInterfaces.h"

int hook::baseAddressDifference;

bool GameInit::GetGameLoaded()
{
	return !(*(uint8_t*)0xF22B3C);
}

void GameInit::LoadGameFirstLaunch(bool (*callBeforeLoad)())
{
	assert(!GameInit::GetGameLoaded());

	*(DWORD*)0x10C7F80 = 6;

	if (callBeforeLoad)
	{
		static bool(*preLoadCB)();

		preLoadCB = callBeforeLoad;

		g_hooksDLL->SetHookCallback(StringHash("preGmLoad"), [] (void*)
		{
			while (!preLoadCB())
			{
				*(BYTE*)0x18A825C = 0;

				MSG msg;

				while (PeekMessage(&msg, 0, 0, 0, TRUE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		});
	}
}

struct LoadingTune
{
	void StartLoadingTune();
};

void WRAPPER LoadingTune::StartLoadingTune() { EAXJMP(0x7B9980); }

static LoadingTune& loadingTune = *(LoadingTune*)0x10F85B0;

void GameInit::SetLoadScreens()
{
	((void(*)(int, int, int))0x423CE0)(1, 0, 0);

	//*(BYTE*)0x7BD9F0 = 0xC3;
	*(BYTE*)0x18A823A = 1;

	loadingTune.StartLoadingTune();

	SetEvent(*(HANDLE*)0x10F9AAC);
}

void WRAPPER SetTextForLoadscreen(const char* text, bool a2, bool a3, int neg1) { EAXJMP(0x84F580); }
bool& stopNetwork = *(bool*)0x18A82FE;

void GameInit::KillNetwork(const wchar_t* reason)
{
	TheText.SetCustom("CIT_NET_KILL_REASON", reason);
	SetTextForLoadscreen("CIT_NET_KILL_REASON", true, false, -1);

	static char smallReason[8192];
	wcstombs(smallReason, reason, _countof(smallReason));

	g_netLibrary->Disconnect(smallReason);

	stopNetwork = true;
}

void GameInit::ReloadGame()
{ }

static InitFunction initFunction([] ()
{
	// unused byte which is set to 0 during loading
	hook::put<uint8_t>(0xF22B3C, 1);
});