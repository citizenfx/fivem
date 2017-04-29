/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ConsoleHostImpl.h"
#include "InputHook.h"
#include <thread>
#include <condition_variable>

static std::thread g_consoleThread;
static std::once_flag g_consoleInitialized;
bool g_consoleFlag;
extern int g_scrollTop;
extern int g_bufferHeight;

static InitFunction initFunction([] ()
{
	InputHook::OnWndProc.Connect([] (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult)
	{
		if (g_consoleFlag)
		{
			static bool g_consoleClosing = false;

			// should the console be closed?
			if (wParam == VK_F8)
			{
				if (msg == WM_KEYDOWN)
				{
					g_consoleClosing = true;
					return;
				}

				if (g_consoleClosing)
				{
					if (msg == WM_KEYUP)
					{
						g_consoleClosing = false;
						g_consoleFlag = false;

						return;
					}
				}
			}
			else if (wParam == VK_PRIOR)
			{
				if (msg == WM_KEYDOWN)
				{
					g_scrollTop -= 10;

					if (g_scrollTop < 0)
					{
						g_scrollTop = 0;
					}
				}
			}
			else if (wParam == VK_NEXT)
			{
				if (msg == WM_KEYDOWN)
				{
					g_scrollTop += 10;

					if (g_scrollTop >= (g_bufferHeight - 25))
					{
						g_scrollTop = (g_bufferHeight - 25);
					}
				}
			}

			// handle keyboard input
			static uint32_t lastVKey;

			if (msg >= WM_KEYFIRST && msg <= WM_KEYLAST)
			{
				if (g_consoleClosing)
				{
					pass = false;
					lresult = 0;
					return;
				}

				uint32_t vKey;
				wchar_t character = 0;

				BYTE keyState[256];
				GetKeyboardState(keyState);

				if (msg == WM_CHAR || msg == WM_SYSCHAR)
				{
					vKey = lastVKey;
					character = wParam;
				}
				else
				{
					uint16_t scanCode = (lParam >> 16) & 0xFF;

					wchar_t characters[2];
					int numChars = ToUnicode(wParam, scanCode, keyState, characters, 2, 0);

					if (numChars == 1)
					{
						character = characters[0];
					}

					vKey = wParam;
					lastVKey = vKey;
				}

				// do we need to ignore the key?
				if (character != 0)
				{
					if (msg != WM_CHAR && msg != WM_SYSCHAR)
					{
						pass = false;
						lresult = 0;

						return;
					}
				}

				if (msg != WM_KEYUP && msg != WM_SYSKEYUP)
				{
					ConsoleModifiers modifiers = ConsoleModifierNone;

					if (keyState[VK_SHIFT] & 0x80)
					{
						modifiers |= ConsoleModifierShift;
					}

					if (keyState[VK_LMENU] & 0x80 || keyState[VK_RMENU] & 0x80)
					{
						modifiers |= ConsoleModifierAlt;
					}

					if (keyState[VK_LCONTROL] & 0x80 || keyState[VK_RCONTROL] & 0x80)
					{
						modifiers |= ConsoleModifierControl;
					}

					ConHost_KeyEnter(vKey, character, modifiers);
				}

				pass = false;
				lresult = 0;
			}
		}
		else
		{
			// check if the console should be opened
			if (msg == WM_KEYUP && wParam == VK_F8)
			{
				g_consoleFlag = true;

				// initialize the console if it hasn't been already
				std::call_once(g_consoleInitialized, [] ()
				{
					g_consoleThread = std::thread([] ()
					{
						SetThreadName(-1, "ConHost Thread");

						ConHost_Run();
					});
				});

				pass = false;
				lresult = 0;

				return;
			}
		}
	}, -10);
});