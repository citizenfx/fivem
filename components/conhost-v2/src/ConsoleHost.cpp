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
		}
		else
		{
			// check if the console should be opened
			if (msg == WM_KEYUP && wParam == VK_F8)
			{
				g_consoleFlag = true;

				pass = false;
				lresult = 0;

				return;
			}
		}
	}, -10);
});