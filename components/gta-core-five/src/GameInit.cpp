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

#include <Hooking.h>

FiveGameInit g_gameInit;

fwEvent<const char*> OnKillNetwork;

void AddCustomText(const char* key, const char* value);

static hook::cdecl_stub<void(int unk, uint32_t* titleHash, uint32_t* messageHash, uint32_t* subMessageHash, int flags, bool, int8_t, void*, void*, bool, bool)> setWarningMessage([] ()
{
	return hook::get_call<void*>(hook::get_call(hook::pattern("57 41 56 41 57 48 83 EC 50 4C 63 F2").count(1).get(0).get<char>(0xAC)) + 0x6D);
});

static hook::cdecl_stub<int(bool, int)> getWarningResult([] ()
{
	return hook::get_call(hook::pattern("33 D2 33 C9 E8 ? ? ? ? 48 83 F8 04 0F 84").count(1).get(0).get<void>(4));
});

static bool g_showWarningMessage;

void FiveGameInit::KillNetwork(const wchar_t* errorString)
{
	if (errorString == (wchar_t*)1)
	{
		// TODO: disconnect cleanly
	}
	else
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string smallReason = converter.to_bytes(errorString);

		AddCustomText("CFX_NETERR", smallReason.c_str());
		AddCustomText("CFX_NETERR_TITLE", "\xD0\x9E\xD0\xA8\xD0\x98\xD0\x91\xD0\x9A\xD0\x90"); // Oshibka!

		g_showWarningMessage = true;

		OnKillNetwork(smallReason.c_str());
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

static InitFunction initFunction([] ()
{
	OnGameFrame.Connect([] ()
	{
		if (g_showWarningMessage)
		{
			uint32_t titleHash = HashString("CFX_NETERR_TITLE");
			uint32_t messageHash = HashString("CFX_NETERR");
			uint32_t noneHash = 0;

			setWarningMessage(0, &titleHash, &messageHash, &noneHash, 2,  0, -1, 0, 0, 1, 0);

			if (getWarningResult(0, 0) == 2)
			{
				g_showWarningMessage = false;

				OnMsgConfirm();
			}
		}
	});

	Instance<ICoreGameInit>::Set(&g_gameInit);
});