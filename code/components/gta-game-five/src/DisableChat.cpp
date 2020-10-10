/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <DisableChat.h>

#include <MinHook.h>

#include <Hooking.h>

static void(*g_origTextChatShutdown)(void*);
static void** g_textChat;
static void* g_textChatBackup;

static void TextChatShutdownWrap(void* textChat)
{
	if (textChat)
	{
		g_origTextChatShutdown(textChat);
	}
}

static void(*g_origInputCheck)();

static void WrapInputCheck()
{
	if (g_textChat && *g_textChat)
	{
		g_origInputCheck();
	}
}

namespace game
{
	void SetTextChatEnabled(bool enabled)
	{
		if (g_textChat && *g_textChat)
		{
			if (!enabled)
			{
				g_textChatBackup = *g_textChat;
				*g_textChat = nullptr;
			}
		}
		else
		{
			if (enabled)
			{
				//*g_textChat = g_textChatBackup;
			}
		}
	}
}

static HookFunction hookFunction([] ()
{
	// find the text chat shutdown function (to patch, and to get the text chat pointer)
#ifdef PRE_PATCH_EXECUTABLE
	auto matches = hook::pattern("48 8B CB C7 45 10 EB 8F 56 B8").count(2);

	for (int i = 0; i < matches.size(); i++)
	{
		char* location = matches.get(i).get<char>(-4);
		char* testFunc = (char*)(location + *(int32_t*)location + 4);

		// proper function starts with 'mov rcx, ...'
		if (testFunc[0] == 0x48)
		{
			// patch the caller
			hook::set_call(&g_origTextChatShutdown, testFunc + 7);
			hook::jump(testFunc + 7, TextChatShutdownWrap);

			// and get the global text chat pointer
			location = testFunc + 3;

			g_textChat = (void**)(location + *(int32_t*)location + 4);
		}
	}
#else
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("32 DB 84 C0 74 2D 48 8B", -0x22), TextChatShutdownWrap, (void**)&g_origTextChatShutdown);
		MH_EnableHook(MH_ALL_HOOKS);

		g_textChat = hook::get_address<void**>(hook::get_pattern("75 5D 48 8B 05 ? ? ? ? 44 38 60 14", 5));
	}
#endif

	// patch another function depending on text chat
	void* func = hook::pattern("B1 01 E8 ? ? ? ? 84 C0 75 05 E8").count(1).get(0).get<void>(21);
	hook::set_call(&g_origInputCheck, func);
	hook::call(func, WrapInputCheck);

	// some task checks for text chat that shouldn't *really* be needed... we hope.
	auto p = hook::pattern("44 38 60 14 75 06 44 39 60 04 74").count(2);

	for (int i = 0; i < p.size(); i++)
	{
		auto loc = p.get(i).get<char>(0);

		hook::nop(loc, 10);
		hook::put<uint8_t>(loc + 10, 0xEB);
	}

	auto loc = hook::pattern("38 59 14 75 05 39 59 04 74").count(1).get(0).get<char>(0);

	hook::nop(loc, 8);
	hook::put<uint8_t>(loc + 8, 0xEB);

	loc = hook::pattern("38 58 14 75 05 39 58 04 74").count(1).get(0).get<char>(0);

	hook::nop(loc, 8);
	hook::put<uint8_t>(loc + 8, 0xEB);

	/*loc = hook::pattern("38 58 14 75 05 39 58 04 74").count(1).get(0).get<char>(0);

	hook::nop(loc, 8);
	hook::put<uint8_t>(loc + 8, 0xEB);*/

	/*loc = hook::pattern("44 38 60 14 75 06 44 39 68 04 74").count(1).get(0).get<char>(0);

	hook::nop(loc, 10);
	hook::put<uint8_t>(loc + 10, 0xEB);*/
});
