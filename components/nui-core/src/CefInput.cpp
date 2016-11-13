/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CefOverlay.h"
#include "CrossLibraryInterfaces.h"
#include "InputHook.h"
#include <windowsx.h>

static bool g_hasFocus = false;
extern bool g_mainUIFlag;
POINT g_cursorPos;

bool isKeyDown(WPARAM wparam)
{
	return (GetKeyState(wparam) & 0x8000) != 0;
}

namespace nui
{
	void GiveFocus(bool hasFocus)
	{
#if defined(GTA_NY)
		if (!g_hasFocus && hasFocus)
		{
			// TODO: move this elsewhere

			// release all keys in the game
			uint8_t* keyData = (uint8_t*)0x188B150;

			memset(keyData, 0, 256);

			// disable processing mouse input
			//*(WORD*)0x6239F7 = 0x9090;

			((void(*)(bool))0x623C30)(false);
		}
		else if (!hasFocus && g_hasFocus)
		{
			((void(*)(bool))0x623C30)(true);
		}
#else
		if (!g_hasFocus && hasFocus)
		{
			InputHook::SetGameMouseFocus(false);
		}
		else if (!hasFocus && g_hasFocus)
		{
			InputHook::SetGameMouseFocus(true);
		}
#endif

		g_hasFocus = hasFocus;
	}

	void ProcessInput()
	{
	}
}

int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam)
{
	int modifiers = 0;
	if (isKeyDown(VK_SHIFT))
		modifiers |= EVENTFLAG_SHIFT_DOWN;
	if (isKeyDown(VK_CONTROL))
		modifiers |= EVENTFLAG_CONTROL_DOWN;
	if (isKeyDown(VK_MENU))
		modifiers |= EVENTFLAG_ALT_DOWN;

	// Low bit set from GetKeyState indicates "toggled".
	if (::GetKeyState(VK_NUMLOCK) & 1)
		modifiers |= EVENTFLAG_NUM_LOCK_ON;
	if (::GetKeyState(VK_CAPITAL) & 1)
		modifiers |= EVENTFLAG_CAPS_LOCK_ON;

	switch (wparam)
	{
		case VK_RETURN:
			if ((lparam >> 16) & KF_EXTENDED)
				modifiers |= EVENTFLAG_IS_KEY_PAD;
			break;
		case VK_INSERT:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
			if (!((lparam >> 16) & KF_EXTENDED))
				modifiers |= EVENTFLAG_IS_KEY_PAD;
			break;
		case VK_NUMLOCK:
		case VK_NUMPAD0:
		case VK_NUMPAD1:
		case VK_NUMPAD2:
		case VK_NUMPAD3:
		case VK_NUMPAD4:
		case VK_NUMPAD5:
		case VK_NUMPAD6:
		case VK_NUMPAD7:
		case VK_NUMPAD8:
		case VK_NUMPAD9:
		case VK_DIVIDE:
		case VK_MULTIPLY:
		case VK_SUBTRACT:
		case VK_ADD:
		case VK_DECIMAL:
		case VK_CLEAR:
			modifiers |= EVENTFLAG_IS_KEY_PAD;
			break;
		case VK_SHIFT:
			if (isKeyDown(VK_LSHIFT))
				modifiers |= EVENTFLAG_IS_LEFT;
			else if (isKeyDown(VK_RSHIFT))
				modifiers |= EVENTFLAG_IS_RIGHT;
			break;
		case VK_CONTROL:
			if (isKeyDown(VK_LCONTROL))
				modifiers |= EVENTFLAG_IS_LEFT;
			else if (isKeyDown(VK_RCONTROL))
				modifiers |= EVENTFLAG_IS_RIGHT;
			break;
		case VK_MENU:
			if (isKeyDown(VK_LMENU))
				modifiers |= EVENTFLAG_IS_LEFT;
			else if (isKeyDown(VK_RMENU))
				modifiers |= EVENTFLAG_IS_RIGHT;
			break;
		case VK_LWIN:
			modifiers |= EVENTFLAG_IS_LEFT;
			break;
		case VK_RWIN:
			modifiers |= EVENTFLAG_IS_RIGHT;
			break;
	}
	return modifiers;
}

static HookFunction initFunction([] ()
{
	InputHook::QueryMayLockCursor.Connect([](int& argPtr)
	{
		if (g_hasFocus)
		{
			argPtr = 0;
		}
	});

	//g_hooksDLL->SetHookCallback(StringHash("wndProc"), [] (void* argsPtr)
	InputHook::OnWndProc.Connect([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult)
	{
		if (!pass)
		{
			return;
		}

		static bool g_lastFocus = false;

		// send a focus event to CEF if focus changed
		auto browser = nui::GetBrowser();

		if (browser)
		{
			if (g_hasFocus != g_lastFocus)
			{
				browser->GetHost()->SendFocusEvent(g_hasFocus);
			}

			g_lastFocus = g_hasFocus;
		}

		if (g_hasFocus)
		{
			if (msg == WM_KEYUP || msg == WM_KEYDOWN || msg == WM_CHAR)
			{
				CefKeyEvent keyEvent;

				keyEvent.windows_key_code = wParam;
				keyEvent.native_key_code = lParam;
				keyEvent.modifiers = GetCefKeyboardModifiers(wParam, lParam);

				if (msg != WM_CHAR)
				{
					keyEvent.type = (msg == WM_KEYDOWN) ? KEYEVENT_RAWKEYDOWN : KEYEVENT_KEYUP;
				}
				else
				{
					keyEvent.type = KEYEVENT_CHAR;
				}

				auto browser = nui::GetBrowser();

				if (browser)
				{
					browser->GetHost()->SendKeyEvent(keyEvent);
				}

				pass = false;
				lresult = FALSE;
				return;
			}
			else if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
			{
				CefMouseEvent mouseEvent;

				mouseEvent.x = LOWORD(lParam);
				mouseEvent.y = HIWORD(lParam);

				auto browser = nui::GetBrowser();

				if (browser)
				{
					browser->GetHost()->SendMouseClickEvent(mouseEvent, (msg == WM_LBUTTONUP || msg == WM_LBUTTONDOWN) ? MBT_LEFT : MBT_RIGHT, (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP), 1);
				}

				pass = false;
				lresult = FALSE;
				return;
			}
			else if (msg == WM_MOUSEMOVE)
			{
				CefMouseEvent mouseEvent;

				mouseEvent.x = LOWORD(lParam);
				mouseEvent.y = HIWORD(lParam);

				g_cursorPos.x = mouseEvent.x;
				g_cursorPos.y = mouseEvent.y;

				auto browser = nui::GetBrowser();

				if (browser)
				{
					browser->GetHost()->SendMouseMoveEvent(mouseEvent, false);
				}

				pass = false;
				lresult = FALSE;
				return;
			}
			else if (msg == WM_MOUSEWHEEL)
			{
				int delta = GET_WHEEL_DELTA_WPARAM(wParam);

				CefMouseEvent mouseEvent;

				mouseEvent.x = GET_X_LPARAM(lParam);
				mouseEvent.y = GET_Y_LPARAM(lParam);

				g_cursorPos.x = mouseEvent.x;
				g_cursorPos.y = mouseEvent.y;

				auto browser = nui::GetBrowser();

				if (browser)
				{
					browser->GetHost()->SendMouseWheelEvent(mouseEvent, 0, delta);
				}

				pass = false;
				lresult = TRUE;

				return;
			}
		}
	});
});