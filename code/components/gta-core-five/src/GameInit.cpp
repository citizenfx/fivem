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

#include <CrossBuildRuntime.h>

FiveGameInit g_gameInit;

fwEvent<const char*> OnKillNetwork;
fwEvent<> OnKillNetworkDone;

void AddCustomText(const char* key, const char* value);

static hook::cdecl_stub<void(int unk, uint32_t* titleHash, uint32_t* messageHash, uint32_t* subMessageHash, int flags, bool, int8_t, void*, void*, bool, bool)> setWarningMessage([] ()
{
	if (Is372())
	{
		return hook::get_call<void*>(hook::get_call(hook::pattern("57 41 56 41 57 48 83 EC 50 4C 63 F2").count(1).get(0).get<char>(0xAC)) + 0x6D);
	}

	return hook::get_pattern("44 38 ? ? ? ? ? 0F 85 C2 02 00 00 E8", -0x3A);
});

static hook::cdecl_stub<int(bool, int)> getWarningResult([] ()
{
	return hook::get_call(hook::pattern("33 D2 33 C9 E8 ? ? ? ? 48 83 F8 04 0F 84").count(1).get(0).get<void>(4));
});

static bool g_showWarningMessage;
static std::string g_warningMessage;

void FiveGameInit::KillNetwork(const wchar_t* errorString)
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
	return hook::get_pattern("83 F9 08 75 3F 53 48 83 EC 20 48 83 3D", 0);
});

static hook::cdecl_stub<void(rage::InitFunctionType)> gamerInfoMenu__shutdown([]()
{
	return hook::get_pattern("83 F9 08 75 46 53 48 83 EC 20 48 83", 0);
});

static void(*g_origLoadMultiplayerTextChat)();

static void** g_textInputBox;

static HookFunction hookFunction([]()
{
	// really bad pattern pointing to switch-to-netgame
	if (!Is2060() && !Is372())
	{
		auto location = hook::get_pattern("E8 ? ? ? ? B9 08 00 00 00 E8 ? ? ? ? E8", 15);

		hook::set_call(&g_origLoadMultiplayerTextChat, location);
		hook::nop(location, 5);

		// 1737: screwed by Arxan
		// #TODO1737
	}

	if (!Is372())
	{
		g_textInputBox = hook::get_address<void**>(hook::get_pattern("C7 45 D4 07 00 00 00 48 8B 0D", 10));

		// disable text input box gfx unload
		hook::nop(hook::get_pattern("E8 ? ? ? ? 83 8B A0 04 00 00 FF"), 5);
	}

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

static InitFunction initFunction([] ()
{
	OnGameFrame.Connect([] ()
	{
		if (g_showWarningMessage)
		{
			g_showWarningMessage = false;

			OnMsgConfirm();
		}
	});

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
			if (!Is2060() && g_origLoadMultiplayerTextChat)
			{
				g_origLoadMultiplayerTextChat();
			}

			// 1737: screwed by Arxan
			// #TODO1737

			if (!Is372())
			{
				// temp hook bits to prevent *opening* the gfx
				auto func = hook::get_call(hook::get_pattern<char>("38 59 59 75 05 E8", 5));

				uint8_t oldCode[128];
				memcpy(oldCode, func + 0x6E, sizeof(oldCode));

				hook::nop(func + 0x6E, 40);

				// early-load CTextInputBox gfx as this blocking-loads too
				_textInputBox_loadGfx(*g_textInputBox);

				// unhook
				memcpy(func + 0x6E, oldCode, sizeof(oldCode));
			}

			g_gameInit.SetGameLoaded();
		}
	});

	static ConsoleCommand assertCmd("_assert", []()
	{
		assert(!"_assert command used");
	});

	static ConsoleCommand crashCmd("_crash", []()
	{
		CrashCommand();
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
