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

#include "CefImeHandler.h"

#include <windowsx.h>

static bool g_hasFocus = false;
bool g_hasCursor = false;
static bool g_hasOverriddenFocus = false;
extern bool g_mainUIFlag;
POINT g_cursorPos;

bool isKeyDown(WPARAM wparam)
{
	return (GetKeyState(wparam) & 0x8000) != 0;
}

static bool HasFocus()
{
	return (g_hasFocus || g_hasOverriddenFocus);
}

namespace nui
{
	void GiveFocus(bool hasFocus, bool hasCursor)
	{
		if (!HasFocus() && hasFocus)
		{
			InputHook::SetGameMouseFocus(false);
		}
		else if (!hasFocus && HasFocus())
		{
			InputHook::SetGameMouseFocus(true);
		}

		g_hasFocus = hasFocus;
		g_hasCursor = hasCursor;
	}

	void OverrideFocus(bool hasFocus)
	{
		if (!HasFocus() && hasFocus)
		{
			InputHook::SetGameMouseFocus(false);
		}
		else if (!hasFocus && HasFocus())
		{
			InputHook::SetGameMouseFocus(true);
		}

		g_hasOverriddenFocus = hasFocus;
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

OsrImeHandlerWin* g_imeHandler;

static HookFunction initFunction([] ()
{
	InputHook::QueryMayLockCursor.Connect([](int& argPtr)
	{
		if (HasFocus())
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
			if (HasFocus() != g_lastFocus)
			{
				browser->GetHost()->SendFocusEvent(HasFocus());
			}

			g_lastFocus = HasFocus();
		}

		if (HasFocus())
		{
			if (!g_imeHandler)
			{
				g_imeHandler = new OsrImeHandlerWin(FindWindow(L"grcWindow", nullptr));
			}

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

				mouseEvent.x = g_cursorPos.x;
				mouseEvent.y = g_cursorPos.y;

				auto browser = nui::GetBrowser();

				if (browser)
				{
					browser->GetHost()->SendMouseWheelEvent(mouseEvent, 0, delta);
				}

				pass = false;
				lresult = TRUE;
				return;
			}
			else if (msg == WM_INPUT && g_hasCursor)
			{
				pass = false;
				lresult = TRUE;
				return;
			}
			else if (msg == WM_IME_STARTCOMPOSITION)
			{
				if (g_imeHandler)
				{
					g_imeHandler->CreateImeWindow();
					g_imeHandler->MoveImeWindow();
					g_imeHandler->ResetComposition();
				}

				pass = false;
				lresult = FALSE;
				return;
			}
			else if (msg == WM_IME_SETCONTEXT)
			{
				// We handle the IME Composition Window ourselves (but let the IME Candidates
				// Window be handled by IME through DefWindowProc()), so clear the
				// ISC_SHOWUICOMPOSITIONWINDOW flag:
				lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
				::DefWindowProc(hWnd, msg, wParam, lParam);

				// Create Caret Window if required
				if (g_imeHandler) {
					g_imeHandler->CreateImeWindow();
					g_imeHandler->MoveImeWindow();
				}

				pass = false;
				lresult = false;
				return;
			}
			else if (msg == WM_IME_COMPOSITION)
			{
				auto browser = nui::GetBrowser();

				if (browser && g_imeHandler) {
					CefString cTextStr;
					if (g_imeHandler->GetResult(lParam, cTextStr)) {
						// Send the text to the browser. The |replacement_range| and
						// |relative_cursor_pos| params are not used on Windows, so provide
						// default invalid values.
						browser->GetHost()->ImeCommitText(cTextStr,
							CefRange(UINT32_MAX, UINT32_MAX), 0);
						g_imeHandler->ResetComposition();
						// Continue reading the composition string - Japanese IMEs send both
						// GCS_RESULTSTR and GCS_COMPSTR.
					}

					std::vector<CefCompositionUnderline> underlines;
					int composition_start = 0;

					if (g_imeHandler->GetComposition(lParam, cTextStr, underlines,
						composition_start)) {
						// Send the composition string to the browser. The |replacement_range|
						// param is not used on Windows, so provide a default invalid value.
						browser->GetHost()->ImeSetComposition(
							cTextStr, underlines, CefRange(UINT32_MAX, UINT32_MAX),
							CefRange(composition_start,
								static_cast<int>(composition_start + cTextStr.length())));

						// Update the Candidate Window position. The cursor is at the end so
						// subtract 1. This is safe because IMM32 does not support non-zero-width
						// in a composition. Also,  negative values are safely ignored in
						// MoveImeWindow
						g_imeHandler->UpdateCaretPosition(composition_start - 1);
					}
					else {
						browser->GetHost()->ImeCancelComposition();
						g_imeHandler->ResetComposition();
						g_imeHandler->DestroyImeWindow();
					}
				}

				pass = false;
				lresult = false;

				return;
			}
			else if (msg == WM_IME_ENDCOMPOSITION)
			{
				browser->GetHost()->ImeCancelComposition();
				g_imeHandler->ResetComposition();
				g_imeHandler->DestroyImeWindow();

				pass = false;
				lresult = false;

				return;
			}
			else if (msg == WM_IME_KEYLAST || msg == WM_IME_KEYDOWN || msg == WM_IME_KEYUP)
			{
				pass = false;
				lresult = false;

				return;
			}
		}
	});
});
