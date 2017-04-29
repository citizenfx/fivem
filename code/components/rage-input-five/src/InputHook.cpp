#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "InputHook.h"
#include "Hooking.h"

static WNDPROC origWndProc;

static bool g_isFocused = true;
static bool g_isFocusStolen = false;

static void(*disableFocus)();

static void DisableFocus()
{
	if (!g_isFocusStolen)
	{
		disableFocus();
	}
}

static void(*enableFocus)();

static void EnableFocus()
{
	if (!g_isFocusStolen)
	{
		enableFocus();
	}
}

void InputHook::SetGameMouseFocus(bool focus)
{
	g_isFocusStolen = !focus;

	return (focus) ? enableFocus() : disableFocus();
}

static char* g_gameKeyArray;

LRESULT APIENTRY grcWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ACTIVATEAPP)
	{
		g_isFocused = (wParam) ? true : false;
	}

	if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
	{
		if (!g_isFocused)
		{
			return 0;
		}
	}

	bool pass = true;
	LRESULT lresult;

	InputHook::OnWndProc(hwnd, uMsg, wParam, lParam, pass, lresult);

	if (!pass)
	{
		return lresult;
	}

	//return CallWindowProc(origWndProc, hwnd, uMsg, wParam, lParam);
	lresult = origWndProc(hwnd, uMsg, wParam, lParam);

	if (g_isFocusStolen)
	{
		memset(g_gameKeyArray, 0, 256);
	}

	return lresult;
}

BOOL WINAPI ClipCursorWrap(const RECT* lpRekt)
{
	static RECT lastRect;
	static RECT* lastRectPtr;

	if ((lpRekt && !lastRectPtr) ||
		(lastRectPtr && !lpRekt) ||
		!EqualRect(&lastRect, lpRekt))
	{
		// update last rect
		if (lpRekt)
		{
			lastRect = *lpRekt;
			lastRectPtr = &lastRect;
		}
		else
		{
			memset(&lastRect, 0xCC, 0);
			lastRectPtr = nullptr;
		}

		return ClipCursor(lpRekt);
	}

	return TRUE;
}

static HookFunction hookFunction([] ()
{
	// window procedure
	char* location = hook::pattern("48 8D 05 ? ? ? ? 33 C9 44 89 75 20 4C 89 7D").count(1).get(0).get<char>(3);
	
	origWndProc = (WNDPROC)(location + *(int32_t*)location + 4);

	*(int32_t*)location = (intptr_t)(hook::AllocateFunctionStub(grcWindowProcedure)) - (intptr_t)location - 4;

	// disable mouse focus function
	void* patternMatch = hook::pattern("74 0D 38 1D ? ? ? ? 74 05 E8 ? ? ? ? 48 39").count(1).get(0).get<void>(10);
	hook::set_call(&disableFocus, patternMatch);
	hook::call(patternMatch, DisableFocus);

	patternMatch = hook::pattern("74 0D 38 1D ? ? ? ? 74 05 E8 ? ? ? ? 33 C9 E8").count(1).get(0).get<void>(10);
	hook::set_call(&enableFocus, patternMatch);
	hook::call(patternMatch, EnableFocus);

	// game key array
	location = hook::pattern("BF 00 01 00 00 48 8D 1D ? ? ? ? 48 3B 05").count(1).get(0).get<char>(8);

	g_gameKeyArray = (char*)(location + *(int32_t*)location + 4);

	// disable directinput keyboard handling
	// TODO: change for Five
	//hook::return_function(hook::pattern("A1 ? ? ? ? 83 EC 14 53 33 DB").count(1).get(0).get<void>());

	// focus testing
	//hook::call(hook::pattern("83 7E 0C 00 0F 84 23 01 00 00 83 7E 08 00 0F 84").count(1).get(0).get<void>(20), AreWeFocused);
	//hook::jump(hook::pattern("8B 51 08 50 FF D2 84 C0 74 06 B8").count(1).get(0).get<void>(-17), AreWeFocused);

	// force input to be handled using WM_KEYUP/KEYDOWN, not DInput/RawInput

	// disable DInput device creation
	char* dinputCreate = hook::pattern("45 33 C9 FF 50 18 BF 26").count(1).get(0).get<char>(0);
	hook::nop(dinputCreate, 200); // that's a lot of nops!
	hook::nop(dinputCreate + 212, 6);
	hook::nop(dinputCreate + 222, 6);

	// jump over raw input keyboard handling
	hook::put<uint8_t>(hook::pattern("44 39 2E 75 ? B8 FF 00 00 00").count(1).get(0).get<void>(3), 0xEB);

	// fix repeated ClipCursor calls (causing DWM load)
	hook::iat("user32.dll", ClipCursorWrap, "ClipCursor");
});

fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> InputHook::OnWndProc;

fwEvent<int&> InputHook::QueryMayLockCursor;