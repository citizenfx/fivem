#include "StdInc.h"
#include "GameInit.h"
#include "Text.h"
#include "Hooking.h"
#include "CrossLibraryInterfaces.h"

#include <ICoreGameInit.h>

static bool* g_preventSaveLoading;

bool GameInit::GetGameLoaded()
{
	auto ptr = *hook::get_pattern<BYTE*>("8B 4C 24 04 33 C0 85 C9 0F 94 C0", 13);
	return !*ptr;
	//return !(*(uint8_t*)0xF22B3C);
}

void GameInit::LoadGameFirstLaunch(bool (*callBeforeLoad)())
{
	assert(!GameInit::GetGameLoaded());

	static auto skipLoadscreenFrame = *hook::get_pattern<BYTE*>("55 8B EC 83 E4 F8 83 EC 20 80 3D ? ? ? ? 00 53 55", 0x1F);
	static auto loadscreenState = *hook::get_pattern<DWORD*>("83 f8 31 74 ? 83 f8 3e 75 1a", 25);

	*(DWORD*)loadscreenState = 6;

	if (callBeforeLoad)
	{
		static bool(*preLoadCB)();

		preLoadCB = callBeforeLoad;

		OnPreGameLoad.Connect([]()
		{
			while (!preLoadCB())
			{
				*(BYTE*)skipLoadscreenFrame = 0;

				MSG msg;

				while (PeekMessage(&msg, 0, 0, 0, TRUE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		});
	}

	OnGameRequestLoad();
}

#if we_do_want_loading_tune
struct LoadingTune
{
	void StartLoadingTune();
};

void WRAPPER LoadingTune::StartLoadingTune()
{ 
	EAXJMP(0x7B9980);
}

static LoadingTune& loadingTune = *(LoadingTune*)0x10F85B0;
#endif

GameInit::GameInit()
{

}

static hook::cdecl_stub<void* (int, int, int)> _initLoadingScreens([]()
{
	return hook::get_pattern("83 EC 10 80 3D ? ? ? ? 00 53 56 8A FA");
});

static bool* dontProcessTheGame;

void GameInit::SetLoadScreens()
{
	static auto loadScreensEnabled = *hook::get_pattern<uint8_t*>("75 11 85 F6 6A 00", -5);

	// disable load screens if they're already ongoing
	hook::put<uint8_t>(loadScreensEnabled, 0);

	_initLoadingScreens(0, 0, 0);
	if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
	{
		*dontProcessTheGame = true;
		*g_preventSaveLoading = true;
	}

	hook::put<uint8_t>(loadScreensEnabled, 1);

#if we_do_want_loading_tune
	loadingTune.StartLoadingTune();
#endif
}

static bool g_shouldSwitchToCustomLoad;
static rage::grcTexture* g_customLoad;

rage::grcTexture* GameInit::GetLastCustomLoadTexture()
{
	return g_customLoad;
}

void GameInit::PrepareSwitchToCustomLoad(rage::grcTexture* texture)
{
	// set our local flag too
	g_shouldSwitchToCustomLoad = true;

	g_customLoad = texture;
}

bool GameInit::ShouldSwitchToCustomLoad()
{
	if (g_shouldSwitchToCustomLoad)
	{
		g_shouldSwitchToCustomLoad = false;

		return true;
	}

	return false;
}

static hook::cdecl_stub<void(const char*, bool, bool, int)> _setTextForLoadscreen([]()
{
	return hook::get_pattern("CC 0F B6 ? ? ? ? ? FF 74 24 04 33 C9 38", 1);
});

bool* stopNetwork;

static hook::cdecl_stub<void()> _leaveGame([]()
{
	return hook::get_pattern("A1 ? ? ? ? 83 F8 0C 74 34");
});

static bool g_ignoreNextGracefulDeath;

void GameInit::KillNetwork(const wchar_t* reason)
{
	// no overkill please
	if (*stopNetwork)
	{
		return;
	}

	// #TODOLIBERTY: ?
	//g_hooksDLL->SetDisconnectSafeguard(false);

	// special case for graceful kill
	if (reason == (wchar_t*)1)
	{
		if (g_ignoreNextGracefulDeath)
		{
			g_ignoreNextGracefulDeath = false;
			return;
		}

		_leaveGame();
		return;
	}

	if (reason != nullptr)
	{
		TheText->SetCustom("CIT_NET_KILL_REASON", reason);
		_setTextForLoadscreen("CIT_NET_KILL_REASON", true, false, -1);

		static char smallReason[8192];
		wcstombs(smallReason, reason, _countof(smallReason));

		g_ignoreNextGracefulDeath = true;

		OnKillNetwork(smallReason);
	}

	*stopNetwork = true;
}

bool* reloadGameNextFrame;

void GameInit::ReloadGame()
{
	//((void(*)())0x40ACE0)();
	//((void(*)())0x40B180)();

	*reloadGameNextFrame = true;

	tryDisconnectFlag = 2;
}

void GameInit::SetPreventSavePointer(bool* preventSaveValue)
{
	g_preventSaveLoading = preventSaveValue;
}

static void ToggleBackGameProcess()
{
	dontProcessTheGame = false;
	g_preventSaveLoading = false;
}

static uintptr_t origToggleBack;

static void __declspec(naked) ToggleBackGameProcessStub()
{
	__asm
	{
		// preserve ecx and call
		push ecx
		call ToggleBackGameProcess
		pop ecx

		// jump back to original
		push origToggleBack
		retn
	}
}

static hook::cdecl_stub<void* ()> _unloadStreamedFonts([]()
{
	return hook::get_pattern("83 3D ? ? ? ? FF 74 ? 6A FF");
});

void GameInit::MurderGame()
{
	// unload streamed fonts
	_unloadStreamedFonts();
	//((void(*)())0x7F9260)();
}

#include <NetLibrary.h>
#include <GameFlags.h>
#include <nutsnbolts.h>

NetLibrary* g_netLibrary;
GameInit* g_gameInit;

static InitFunction initFunction([]()
{
	g_gameInit = new GameInit();

	// create ICoreGameInit instance
	Instance<ICoreGameInit>::Set(g_gameInit);

	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		g_netLibrary = netLibrary;

		g_netLibrary->OnBuildMessage.Connect([](const std::function<void(uint32_t, const char*, int)>& writeReliable)
		{
			//if (*(BYTE*)0x18A82FD) // is server running
			if (*(BYTE*)hook::get_pattern<char>("0F 86 ? ? ? ? 6A FF 6A 00 6A 01", -33))
			{
				auto base = g_netLibrary->GetServerBase();
				writeReliable(0xB3EA30DE, (char*)&base, sizeof(base));
			}
		});

		g_netLibrary->OnInitReceived.Connect([](NetAddress server)
		{
			GameFlags::ResetFlags();

			/*nui::SetMainUI(false);

			nui::DestroyFrame("mpMenu");*/

			//m_connectionState = CS_DOWNLOADING;

			if (!g_gameInit->GetGameLoaded())
			{
				g_gameInit->SetLoadScreens();
			}
			else
			{
				g_gameInit->SetLoadScreens();

				// unlock the mutex as we'll reenter here
				//g_netFrameMutex.unlock();
				g_netLibrary->Death();

				//g_netFrameMutex.lock();
				g_netLibrary->Resurrection();

				g_gameInit->ReloadGame();
			}
		});

		g_netLibrary->OnAttemptDisconnect.Connect([](const char*)
		{
			g_gameInit->KillNetwork((const wchar_t*)1);
		});

		g_netLibrary->OnFinalizeDisconnect.Connect([](NetAddress address)
		{
			GameFlags::ResetFlags();
			GameInit::MurderGame();
		});

		OnGameFrame.Connect([]()
		{
			g_netLibrary->RunFrame();
		});
	});
});

static HookFunction hookFunction([]()
{
	reloadGameNextFrame = *(bool**)(hook::get_call(hook::get_pattern<char>("6a 00 6a 20 6a 01 50 6a 00", 50)) + 0x16);
	stopNetwork = *hook::get_pattern<bool*>("83 f8 0e 74 34", -9);
	dontProcessTheGame = *hook::get_pattern<bool*>("EB 26 6A 01 6A 00 6A 00 B9", 19);

	// weird jump because of weird random floats? probably for fading anyway...
	hook::nop(hook::pattern("F6 C4 44 0F 8A ? ? ? ? 80 3D ? ? ? ? 00").count(2).get(1).get<void*>(3), 6);

	// some function that resets to/undoes weird loading text state rather than continuing on our nice loading screens (during the 'faux game process' state)
	hook::nop(hook::get_pattern("83 3D ? ? ? ? 01 C6 05 ? ? ? ? 00", -71), 5);

	// hook to reset processing the game after our load caller finishes
	//hook::jump(0x420FA2, ToggleBackGameProcess);
	{
		auto location = hook::get_pattern("6A 00 68 FF 00 00 00 68 E8 03 00 00 B9", 17);
		hook::set_call(&origToggleBack, location);
		hook::call(location, ToggleBackGameProcessStub);
	}

	// unused byte which is set to 0 during loading
	hook::put<uint8_t>(*hook::get_pattern<BYTE*>("8B 4C 24 04 33 C0 85 C9 0F 94 C0", 13), 1);

	// LoadGameNow argument 'reload game fully, even if episodes didn't change' in one caller, to be specific the one we actually use indirectly above as the script flag uses it
	/*hook::put<uint8_t>(0x420F91, true);

	// other callers for this stuff
	hook::put<uint8_t>(0x420FD9, 1);
	hook::put<uint8_t>(0x420EAB, 1);
	hook::put<uint8_t>(0x420F33, 1);*/

	// don't load loadscreens at the start
	hook::nop(hook::get_pattern("6A 00 32 D2 B1 01", 6), 5);

	// don't wait for loadscreens at the start
	hook::put<uint8_t>(hook::get_pattern("80 3D ? ? ? ? 00 B9 01 00 00 00 0F 45 C1 80 3D", -23), 0xEB);

	// always redo game object variables
	//hook::nop(0x4205C5, 2);

	// silly people doing silly things (related to loadGameNow call types)
	{
		auto location = hook::get_pattern<char>("6A 00 68 FF 00 00 00 68 E8 03 00 00 B9");
		hook::nop(location - 0x45, 2);
		hook::nop(location - 0x3B, 2);
	}

	hook::nop(hook::get_pattern("6A 00 6A 00 0F 84 ? ? ? ? 8B", 20), 6);
	hook::nop(hook::get_pattern("6A 00 6A 00 68 C8 00 00 00 E8 ? ? ? ? 5E 5B", 24), 2);
});

fwEvent<const char*> OnKillNetwork;
fwEvent<> OnPreGameLoad;
fwEvent<> OnKillNetworkDone;
