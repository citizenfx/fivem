#include <StdInc.h>

#include <CoreConsole.h>

#include <GameInit.h>
#include <Error.h>

#include <MinHook.h>
#include <Hooking.h>

#include <nutsnbolts.h>

//
// Game reload code flow:
// 1. GameInit::KillNetwork
// 2. Triggers OnKillNetwork, where we'll set loading screen override and set the init state to 'reload save'.
// 3. Game will execute session shutdown, which we intercept with a blocking (busy-ish) loop at the end,
//    and triggering events before and after.
// 4. When GameInit::ReloadGame is called, we'll return and let the game finish reloading.
//

static bool g_setLoadingScreens;
static bool g_shouldKillNetwork;
bool g_isNetworkKilled;

// 1207.80
// instead of setting init state
static hook::cdecl_stub<void(bool)> _newGame([]()
{
	return hook::get_call(hook::get_pattern("33 C9 E8 ? ? ? ? B1 01 E8 ? ? ? ? 83 63 08 00", 9));
});

static hook::cdecl_stub<void(int)> setupLoadingScreens([]()
{
	return hook::get_pattern("83 EA 01 0F 84 99 01 00 00 83 EA 01 0F 84", -0x1E);
});

static hook::cdecl_stub<void()> loadingScreenUpdate([]()
{
	return hook::get_pattern("74 53 48 83 64 24 20 00 4C 8D", -0x31);
});

void SetRenderThreadOverride()
{
	g_setLoadingScreens = true;
}

static void(*g_shutdownSession)();
extern hook::cdecl_stub<void()> _doLookAlive;

void ShutdownSessionWrap()
{
	Instance<ICoreGameInit>::Get()->SetVariable("gameKilled");
	Instance<ICoreGameInit>::Get()->SetVariable("shutdownGame");

	g_isNetworkKilled = true;

	AddCrashometry("kill_network_game", "true");

	OnKillNetworkDone();

	g_shutdownSession();

	Instance<ICoreGameInit>::Get()->OnShutdownSession();

	g_shouldKillNetwork = false;

	while (g_isNetworkKilled)
	{
		// warning screens apparently need to run on main thread
		OnGameFrame();
		OnMainGameFrame();

		Sleep(0);

		_doLookAlive();

		// todo: add critical servicing
	}

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad();
}

static InitFunction initFunction([]()
{
	OnKillNetwork.Connect([=](const char* message)
	{
		AddCrashometry("kill_network", "true");
		AddCrashometry("kill_network_msg", message);

		trace("Killing network: %s\n", message);

		g_shouldKillNetwork = true;

		Instance<ICoreGameInit>::Get()->ClearVariable("networkInited");

		SetRenderThreadOverride();
	}, 500);

	OnLookAliveFrame.Connect([]()
	{
		if (g_setLoadingScreens)
		{
			setupLoadingScreens(1);
			loadingScreenUpdate();

			g_setLoadingScreens = false;
		}

		if (g_shouldKillNetwork)
		{
			trace("Killing network, stage 2...\n");

			_newGame(false);

			g_shouldKillNetwork = false;
		}
	});

	static ConsoleCommand reloadTest("reloadTest", []()
	{
		g_setLoadingScreens = true;
		g_shouldKillNetwork = true;
	});
});

static int Return0()
{
	return 0;
}

static int Return2()
{
	return 2;
}

static HookFunction hookFunction([]()
{
	// never try to reload SP save
	//hook::jump(hook::get_pattern("48 8B 03 48 8B CB FF 50 38 48 8B C8 E8 ? ? ? ? F6 D8", -0x12), Return0);
	hook::jump(hook::get_pattern("84 C0 8D 4B 02 0F 44 D9", -0x10), Return2);

	// don't try the SP transition if we're init type 15
	hook::put<uint8_t>(hook::get_pattern("83 F8 0F 75 58 8B 4B 0C", 3), 0xEB);

	// shutdown session wrapper
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("41 B9 13 00 00 00 45 33 C0 33 D2", -0x51), ShutdownSessionWrap, (void**)&g_shutdownSession);
		MH_EnableHook(MH_ALL_HOOKS);
	}
});
