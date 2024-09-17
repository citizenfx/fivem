/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <GameInit.h>

#include <GlobalEvents.h>
#include <nutsnbolts.h>

#include <gameSkeleton.h>

#include <Hooking.h>

#include <CoreConsole.h>
#include <scrEngine.h>

#include <KnownFolders.h>
#include <ShlObj.h>
#include <Error.h>

#include <CrossBuildRuntime.h>
#include <CL2LaunchMode.h>

FiveGameInit g_gameInit;

fwEvent<const char*> OnKillNetwork;
fwEvent<> OnKillNetworkDone;

void AddCustomText(const char* key, const char* value);

static hook::cdecl_stub<void(int unk, uint32_t* titleHash, uint32_t* messageHash, uint32_t* subMessageHash, int flags, bool, int8_t, void*, void*, bool, bool)> setWarningMessage([] ()
{
	if (xbr::IsGameBuildOrGreater<3258>())
	{
		return hook::get_pattern("48 89 5C 24 ? 4C 89 44 24 ? 89 4C 24");
	}
	else if (xbr::IsGameBuildOrGreater<2699>())
	{
		return hook::get_pattern("44 38 ? ? ? ? ? 0F 85 C5 02 00 00 E8", -0x38);
	}

	return hook::get_pattern("44 38 ? ? ? ? ? 0F 85 C2 02 00 00 E8", -0x3A);
});

static hook::cdecl_stub<int(bool, int)> getWarningResult([] ()
{
	return hook::get_call(hook::pattern("33 D2 33 C9 E8 ? ? ? ? 48 83 F8 04 0F 84").count(1).get(0).get<void>(4));
});

extern volatile bool g_isNetworkKilled;

void FiveGameInit::KillNetwork(const wchar_t* errorString)
{
	if (g_isNetworkKilled)
	{
		return;
	}

	if (errorString == (wchar_t*)1)
	{
		OnKillNetwork("Reloading game.");
	}
	else
	{
		auto narrowReason = ToNarrow(errorString);
		SetData("warningMessage", narrowReason);

		OnKillNetwork(narrowReason.c_str());
		OnMsgConfirm();
	}
}

bool FiveGameInit::GetGameLoaded()
{
	return m_gameLoaded;
}

void FiveGameInit::SetGameLoaded()
{
	m_gameLoaded = true;
}

void FiveGameInit::SetPreventSavePointer(bool* preventSaveValue)
{

}

bool FiveGameInit::TryDisconnect()
{
	return true;
}

bool FiveGameInit::TriggerError(const char* message)
{
	return (!OnTriggerError(message));
}

static hook::cdecl_stub<void(rage::InitFunctionType)> gamerInfoMenu_init([]()
{
	return (xbr::IsGameBuildOrGreater<2802>()) ? hook::get_pattern("E9 ? ? ? ? 53 48 83  EC 20 48 83 3D") : hook::get_pattern("83 F9 08 75 3F 53 48 83 EC 20 48 83 3D");
});

static hook::cdecl_stub<void(rage::InitFunctionType)> gamerInfoMenu__shutdown([]()
{
	return hook::get_pattern("83 F9 08 75 46 53 48 83 EC 20 48 83", 0);
});

static void** g_textChat;
static void(*g_origLoadMultiplayerTextChat)(void*);

static void** g_textInputBox;

#include <MinHook.h>

void Void()
{
}

static HookFunction hookFunction([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("74 07 B0 01 E9 ? ? ? ? 83 65", (xbr::IsGameBuildOrGreater<2372>() ? -0x23 : -0x26)), Void, (void**)&g_origLoadMultiplayerTextChat);
	MH_EnableHook(MH_ALL_HOOKS);

	g_textChat = hook::get_address<void**>(hook::get_pattern("74 04 C6 40 01 01 48 8B 0D", 9));
	g_textInputBox = hook::get_address<void**>(hook::get_pattern("C7 45 D4 07 00 00 00 48", xbr::IsGameBuildOrGreater<2802>() ? 36 : 10));

	// disable text input box gfx unload
	hook::nop(hook::get_pattern("E8 ? ? ? ? 83 8B A0 04 00 00 FF"), 5);

	// disable gamer info menu shutdown (testing/temp dbg for blocking loads on host/join)
	//hook::return_function(hook::get_pattern("83 F9 08 75 46 53 48 83 EC 20 48 83", 0));

	// force SET_GAME_PAUSES_FOR_STREAMING (which makes collision loading, uh, blocking) to be off
	hook::put<uint8_t>(hook::get_pattern("74 20 84 C9 74 1C 84 DB 74 18", 0), 0xEB);
});

static void __declspec(noinline) CrashCommand()
{
	*(volatile int*)0 = 0;
}

static hook::cdecl_stub<void(void*)> _textInputBox_loadGfx([]()
{
	return hook::get_call(hook::get_pattern("38 59 59 75 05 E8", 5));
});

static bool (*g_isScWaitingForInit)();

void RunRlInitServicing()
{
	using dummyVoidFunc = void(*)();
	using dummyVoidFunc2 = void(*)(void*);

	dummyVoidFunc rlInitFunc1 = (dummyVoidFunc)hook::get_pattern<void*>("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? E8");
	dummyVoidFunc rlInitFunc2 = (dummyVoidFunc)hook::get_call(hook::get_pattern<void*>("E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 80 3D ? ? ? ? ? 74 ? 33 C9"));
	dummyVoidFunc rlInitFunc3 = (dummyVoidFunc)hook::get_pattern<void*>("48 83 EC ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 8D 0D");
	dummyVoidFunc2 rlInitFunc4 = (dummyVoidFunc2)hook::get_pattern<void*>("48 83 EC ? 48 8D 0D ? ? ? ? 33 D2 E8 ? ? ? ? E8");
	void* argToRlInitFunc4 = hook::get_by_offset<void, int32_t>(hook::get_pattern<uint8_t>("48 8D 0D ? ? ? ? C6 05 ? ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? B2"), 3);

	rlInitFunc1();
	rlInitFunc2();
	rlInitFunc3();
	rlInitFunc4(argToRlInitFunc4);
}

void WaitForRlInit()
{
	assert(g_isScWaitingForInit);

	auto waitForRlInitStart = GetTickCount64();

	while (g_isScWaitingForInit())
	{
		// if stuck waiting for over a minute, likely this errored out
		if ((GetTickCount64() - waitForRlInitStart) > 60000)
		{
			{
				PWSTR appdataPath = nullptr;
				SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdataPath);

				_wunlink(va(L"%s\\CitizenFX\\ros_id%s.dat", appdataPath, IsCL2() ? L"CL2" : L""));
			}

			FatalError("Took too long in WaitForRlInit\nWaiting for R* SC SDK initialization took too long. Please restart your game and try again.\n\nIf this issue reoccurs, there might be a problem with cached entitlement tickets.");
		}

		RunRlInitServicing();

		Sleep(50);
	}
}

void SetScInitWaitCallback(bool (*cb)())
{
	g_isScWaitingForInit = cb;
}

static InitFunction initFunction([] ()
{
	OnKillNetworkDone.Connect([]()
	{
		gamerInfoMenu__shutdown(rage::INIT_SESSION);
	});

	rage::OnInitFunctionEnd.Connect([] (rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			// early-init the mp_gamer_info menu (doing this when switching will cause blocking LoadObjectsNow calls)
			gamerInfoMenu_init(rage::INIT_SESSION);

			// also early-load MULTIPLAYER_TEXT_CHAT gfx, this changed sometime between 323 and 505
			// and also causes a blocking load.
			if (g_origLoadMultiplayerTextChat && g_textChat && *g_textChat)
			{
				g_origLoadMultiplayerTextChat(*g_textChat);
			}

			// temp hook bits to prevent *opening* the gfx
			{
				auto func = hook::get_call(hook::get_pattern<char>("38 59 59 75 05 E8", 5));

				uint8_t oldCode[128];
				memcpy(oldCode, func + 0x6E, sizeof(oldCode));

				hook::nop(func + 0x6E, 40);

				// early-load CTextInputBox gfx as this blocking-loads too
				_textInputBox_loadGfx(*g_textInputBox);

				// unhook
				DWORD oldProtect;
				VirtualProtect(func + 0x6E, sizeof(oldCode), PAGE_EXECUTE_READWRITE, &oldProtect);
				memcpy(func + 0x6E, oldCode, sizeof(oldCode));
				VirtualProtect(func + 0x6E, sizeof(oldCode), oldProtect, &oldProtect);
			}

			g_gameInit.SetGameLoaded();
		}
	});

	static ConsoleCommand assertCmd("_assert", []()
	{
#ifndef _DEBUG
		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			return;
		}
#endif

		assert(!"_assert command used");
	});

	static ConsoleCommand crashGameCmd("_crash", [](bool game)
	{
#ifndef _DEBUG
		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			return;
		}
#endif

		if (game)
		{
			auto gameCrashPattern = hook::pattern("45 33 C9 49 8B D2 48 8B 01 48 FF 60").count_hint(2).get(1).get<void>();
			hook::put<uint8_t>(gameCrashPattern, 0xCC);
		}
		else
		{
			CrashCommand();
		}
	});

	static ConsoleCommand crashCmd("_crash", []()
	{
#ifndef _DEBUG
		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			return;
		}
#endif

		CrashCommand();
	});

	static ConsoleCommand exceptCmd("_except", []()
	{
#ifndef _DEBUG
		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			return;
		}
#endif

		throw std::runtime_error("_except command used");
	});

	static int warningMessageActive = false;
	static ConVar<int> warningMessageResult("warningMessageResult", ConVar_None, 0);
	static int wmButtons;

	static ConsoleCommand warningMessageCmd("warningMessage", [](const std::string& heading, const std::string& label, const std::string& label2, int buttons)
	{
		AddCustomText("CUST_WARN_HEADING", heading.c_str());
		AddCustomText("CUST_WARN_LABEL", label.c_str());
		AddCustomText("CUST_WARN_LABEL2", label2.c_str());

		wmButtons = buttons;
		warningMessageActive = true;
	});

	OnGameFrame.Connect([]()
	{
		if (warningMessageActive)
		{
			uint32_t headingHash = HashString("CUST_WARN_HEADING");
			uint32_t labelHash = HashString("CUST_WARN_LABEL");
			uint32_t label2Hash = HashString("CUST_WARN_LABEL2");

			NativeInvoke::Invoke<0xDC38CC1E35B6A5D7, uint32_t>("CUST_WARN_HEADING", "CUST_WARN_LABEL", wmButtons, "CUST_WARN_LABEL2", 0, -1, 0, 0, 1);

			int result = getWarningResult(true, 0);

			if (result != 0)
			{
				warningMessageResult.GetHelper()->SetRawValue(result);
				warningMessageActive = false;
			}
		}
	});

	Instance<ICoreGameInit>::Set(&g_gameInit);
});
