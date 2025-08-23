/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <GameInit.h>

#include <Hooking.h>
#include <MinHook.h>

#include <GlobalEvents.h>
#include <nutsnbolts.h>

#include <CoreConsole.h>
#include <console/OptionTokenizer.h>
#include <gameSkeleton.h>

#include <Error.h>
#include <CrossBuildRuntime.h>

RageGameInit g_gameInit;
fwEvent<const char*> OnKillNetwork;
fwEvent<> OnKillNetworkDone;
DLL_EXPORT fwEvent<> OnMsgConfirm;

static bool g_showWarningMessage;
static std::string g_warningMessage;

static bool g_triedLoading = false;
static bool g_shouldSetState;

void RageGameInit::KillNetwork(const wchar_t* errorString)
{
	if (errorString == (wchar_t*)1)
	{
		OnKillNetwork("Reloading game.");
	}
	else
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string smallReason = converter.to_bytes(errorString);

		if (!g_showWarningMessage)
		{
			g_warningMessage = smallReason;
			g_showWarningMessage = true;

			SetData("warningMessage", g_warningMessage);

			OnKillNetwork(smallReason.c_str());
		}
	}
}

bool RageGameInit::GetGameLoaded()
{
	return m_gameLoaded;
}

void RageGameInit::SetGameLoaded()
{
	m_gameLoaded = true;
}

void RageGameInit::SetPreventSavePointer(bool* preventSaveValue)
{

}

extern bool g_setLoadingScreens;
extern bool g_isNetworkKilled;
extern bool g_shouldKillNetwork;
extern int* g_initState;

void SetRenderThreadOverride()
{
	g_setLoadingScreens = true;
}

void RageGameInit::ReloadGame()
{
	AddCrashometry("reload_game", "true");
	
	m_gameLoaded = false;
	g_isNetworkKilled = false;

	ClearVariable("gameKilled");
}

static bool(*g_callBeforeLoad)();
static void (*g_runInitFunctions)(void*, int);
static void (*g_lookAlive)();

static void RunInitFunctionsWrap(void* skel, int type)
{
	if (g_callBeforeLoad)
	{
		while (!g_callBeforeLoad())
		{
			g_lookAlive();
		}
	}
	g_runInitFunctions(skel, type);
}

void RageGameInit::LoadGameFirstLaunch(bool(*callBeforeLoad)())
{
	AddCrashometry("load_game_first_launch", "true");
	g_callBeforeLoad = callBeforeLoad;

	OnGameFrame.Connect([=]()
	{
		if (g_shouldSetState)
		{
			if (*g_initState == 10)
			{
				*g_initState = 21;
				g_triedLoading = true;

				g_shouldSetState = false;
			}
		}

		static bool isLoading = false;

		if (isLoading)
		{
			if (*g_initState == 0)
			{
				trace("^2Game finished loading!\n");

				ClearVariable("shutdownGame");
				ClearVariable("killedGameEarly");

				OnGameFinalizeLoad();
				isLoading = false;
			}
		}
		else
		{
			if (*g_initState != 0)
			{
				isLoading = true;
			}
		}
		//trace("g_initState %i\n", *g_initState);
	});

	OnKillNetwork.Connect([=](const char* message)
	{
		AddCrashometry("kill_network", "true");
		AddCrashometry("kill_network_msg", message);

		trace("Killing network: %s\n", message);

		g_shouldKillNetwork = true;

		Instance<ICoreGameInit>::Get()->ClearVariable("networkInited");

		SetRenderThreadOverride();

		if (!Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			Instance<ICoreGameInit>::Get()->SetVariable("killedGameEarly");
			Instance<ICoreGameInit>::Get()->SetVariable("gameKilled");
			Instance<ICoreGameInit>::Get()->SetVariable("shutdownGame");

			AddCrashometry("kill_network_game_early", "true");

			OnKillNetworkDone();
		}
	}, 500);

	OnGameRequestLoad();

	if (*g_initState == 10)
	{
		*g_initState = 21;
		g_triedLoading = true;
	}
	else
	{
		g_shouldSetState = true;
	}
}

bool RageGameInit::TryDisconnect()
{
	return true;
}

bool RageGameInit::TriggerError(const char* message)
{
	return (!OnTriggerError(message));
}


// share this(!)
static std::vector<ProgramArguments> g_argumentList;

static InitFunction initFunction([]()
{
	// initialize console arguments
	std::vector<std::pair<std::string, std::string>> setList;

	auto commandLine = GetCommandLineW();

	{
		wchar_t* s = commandLine;

		if (*s == L'"')
		{
			++s;
			while (*s)
			{
				if (*s++ == L'"')
				{
					break;
				}
			}
		}
		else
		{
			while (*s && *s != L' ' && *s != L'\t')
			{
				++s;
			}
		}

		while (*s == L' ' || *s == L'\t')
		{
			s++;
		}

		try
		{
			std::tie(g_argumentList, setList) = TokenizeCommandLine(ToNarrow(s));
		}
		catch (std::runtime_error & e)
		{
			trace("couldn't parse command line: %s\n", e.what());
		}
	}

	se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

	for (const auto& set : setList)
	{
		console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "set", set.first, set.second });
	}
});

static InitFunction initFunctionTwo([]()
{
	// early init command stuff
	rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::InitFunctionType::INIT_CORE)
		{
			// run command-line initialization
			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

			for (const auto& bit : g_argumentList)
			{
				console::GetDefaultContext()->ExecuteSingleCommandDirect(bit);
			}
		}
	});

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			g_gameInit.SetGameLoaded();
		}
	});

	OnGameFrame.Connect([]()
	{
		if (g_showWarningMessage)
		{
			g_showWarningMessage = false;

			OnMsgConfirm();
		}
	});

	static ConsoleCommand assertCmd("_assert", []()
	{
		assert(!"_assert command used");
	});

	static ConsoleCommand crashCmd("_crash", []()
	{
		*(volatile int*)0 = 0;
	});

	Instance<ICoreGameInit>::Set(&g_gameInit);
});

static int Return1()
{
	return 1;
}

static bool ReturnTrueAndForcePedMPFlag(char* playerObj)
{
	char* ped = *(char**)(playerObj + 88);
	char* task = *(char**)(ped + 32); // why is this in fwEntity anyway?

	task[861] &= ~0x40;

	return true;
}

static void LogStubLog1(void* stub, const char* type, const char* format, ...)
{
	if (type && format)
	{
		char buffer[4096];
		va_list ap;
		va_start(ap, format);
		vsnprintf(buffer, 4096, format, ap);
		va_end(ap);

		trace("%s: %s\n", type, buffer);
	}
}

static bool (*g_fiAssetManagerExists)(void*, const char*, const char*);
static bool fiAssetManagerExists(void* self, const char* name, const char* extension)
{
	if ((extension && strcmp(extension, "meta") == 0) && strcmp(name, "platformcrc:/data/startup") == 0)
	{
		return false;
	}

	return g_fiAssetManagerExists(self, name, extension);
}

static HookFunction hookFunctionNet([]()
{
	// tunable privilege check
	// arxan
	hook::jump(hook::get_call(hook::get_pattern("74 07 B8 80 9C D1 EB EB 0E", -0xC)), Return1);

	// player can-clone SP model skip
	hook::jump(hook::get_pattern("84 C0 74 04 32 C0 EB 0E 4C 8B C7 48 8B D6", -0x1D), ReturnTrueAndForcePedMPFlag);
	hook::jump(hook::get_pattern("40 8A F2 48 8B F9 E8 ? ? ? ? 84 C0 74", -0x12), ReturnTrueAndForcePedMPFlag);

	// nop checks used for not syncing some "unwanted" metaped components
	hook::nop(hook::get_pattern("8B 40 18 3D CC E2 69 9D"), 0x2F);

	// skip tunable checks for explosion/fire related natives
	hook::jump(hook::get_pattern("B9 BD C5 AF E3 BA B2 A0 A7 92", -0x14), Return1);

	//hook::jump(0x1406B50E8, LogStubLog1);

	// skip use of SC session based queryfunctions
	// #TODORDR: can we safely disable this? (2019-12-11)
	hook::nop(hook::get_pattern("48 0F 44 C1 48 8D 0D ? ? ? ? 48 89 44"), 4);

	// loading screen thread FPS -> 180 max
	{
		auto location = hook::get_pattern<char>("0F 2F 05 ? ? ? ? 73 0F B9 0F 00 00 00");
		*hook::get_address<float*>(location + 3) = 1000 / 180.0;
		hook::put<int32_t>(location + 10, 0);
	}

	// don't block ped loco for MP peds if not in MP mode (or SP peds if not in SP mode)
	hook::jump(hook::get_pattern("75 05 83 FB 01 EB 03 83 FB 02", -0x1C), Return1);

	// ignore collision-related archetype flag in /CREATE_OBJECT(_NO_OFFSET)?/
	hook::nop(hook::get_pattern("8B 48 50 48 C1 E9 11 F6 C1 01 0F 84 ? ? 00 00 45", 10), 6);

	// Block loading of custom startup.meta file. Completely breaks game loading
	MH_Initialize();
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 3C ? 75 ? 48 8B 0D")), fiAssetManagerExists, (void**)&g_fiAssetManagerExists);
	MH_EnableHook(MH_ALL_HOOKS);

	// block loading until conditions succeed
	{
		char* loadStarter = hook::get_pattern<char>("C6 05 ? ? ? ? 00 E8 ? ? ? ? E8 ? ? ? ? 8B CB E8", 7);
		hook::set_call(&g_runInitFunctions, loadStarter);
		hook::set_call(&g_lookAlive, loadStarter + 5);
		hook::call(loadStarter, RunInitFunctionsWrap);
	}
});
