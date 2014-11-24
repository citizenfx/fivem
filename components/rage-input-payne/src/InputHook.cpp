#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "InputHook.h"
#include "Hooking.h"

WNDPROC origWndProc;

bool g_isFocused = true;

static int AreWeFocused()
{
	return !g_isFocused;
}

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

	return CallWindowProc(origWndProc, hwnd, uMsg, wParam, lParam);
}

static HookFunction hookFunction([] ()
{
	// window procedure
	WNDPROC* wndProcPtr = hook::pattern("66 0F D6 44 24 48 C7 44 24 28 08").count(1).get(0).get<WNDPROC>(18);

	origWndProc = *wndProcPtr;
	hook::put(wndProcPtr, grcWindowProcedure);

	// disable directinput keyboard handling
	hook::return_function(hook::pattern("A1 ? ? ? ? 83 EC 14 53 33 DB").count(1).get(0).get<void>());

	// focus testing
	hook::call(hook::pattern("83 7E 0C 00 0F 84 23 01 00 00 83 7E 08 00 0F 84").count(1).get(0).get<void>(20), AreWeFocused);
});

fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> InputHook::OnWndProc;

fwEvent<int&> InputHook::QueryMayLockCursor;