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
bool& dontProcessTheGame = *(bool*)0x18A8238;

void GameInit::SetLoadScreens()
{
	((void(*)(int, int, int))0x423CE0)(0, 0, 0);
	//((void(*)())0x423E60)();

	if (GetGameLoaded())
	{
		dontProcessTheGame = true;
	}

	//*(BYTE*)0x7BD9F0 = 0xC3;
	*(BYTE*)0x18A823A = 1;

	loadingTune.StartLoadingTune();

	//SetEvent(*(HANDLE*)0x10F9AAC);
}

void WRAPPER SetTextForLoadscreen(const char* text, bool a2, bool a3, int neg1) { EAXJMP(0x84F580); }
bool& stopNetwork = *(bool*)0x18A82FE;

void WRAPPER __fastcall LeaveGame(void* net) { EAXJMP(0x462370); }

static bool g_ignoreNextGracefulDeath;

void GameInit::KillNetwork(const wchar_t* reason)
{
	g_hooksDLL->SetDisconnectSafeguard(false);

	// special case for graceful kill
	if (reason == (wchar_t*)1)
	{
		if (g_ignoreNextGracefulDeath)
		{
			g_ignoreNextGracefulDeath = false;
			return;
		}

		LeaveGame((void*)0x11D6718);
		return;
	}

	if (reason != nullptr)
	{
		TheText.SetCustom("CIT_NET_KILL_REASON", reason);
		SetTextForLoadscreen("CIT_NET_KILL_REASON", true, false, -1);

		static char smallReason[8192];
		wcstombs(smallReason, reason, _countof(smallReason));

		g_ignoreNextGracefulDeath = true;

		g_netLibrary->Disconnect(smallReason);
	}

	stopNetwork = true;
}

bool& reloadGameNextFrame = *(bool*)0x10F8074;

void GameInit::ReloadGame()
{
	reloadGameNextFrame = true;
}

static void ToggleBackGameProcess()
{
	dontProcessTheGame = false;
}

static InitFunction initFunction([] ()
{
	// weird jump because of weird random floats? probably for fading anyway...
	hook::nop(0x42107C, 6);

	// some function that resets to/undoes weird loading text state rather than continuing on our nice loading screens (during the 'faux game process' state)
	hook::nop(0x421426, 5);

	// does loading stuff when the game is set to not process
	hook::return_function(0x421160);

	// hook to reset processing the game after our load caller finishes
	hook::jump(0x420FA2, ToggleBackGameProcess);

	// unused byte which is set to 0 during loading
	hook::put<uint8_t>(0xF22B3C, 1);

	// LoadGameNow argument 'reload game fully, even if episodes didn't change' in one caller, to be specific the one we actually use indirectly above as the script flag uses it
	hook::put<uint8_t>(0x420F91, true);
});