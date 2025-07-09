#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "InputHook.h"
#include "Hooking.h"

#include <nutsnbolts.h>

#include <CfxState.h>
#include <CrossBuildRuntime.h>
#include <dinput.h>

static WNDPROC origWndProc;

typedef IDirectInputDeviceW* LPDIRECTINPUTDEVICEW;
static LPDIRECTINPUTDEVICEW* g_diMouseDevice = nullptr;

static bool g_isFocused = true;
static bool g_enableSetCursorPos = false;
static bool g_isFocusStolen = false;

static rage::ioMouse* g_input;

static bool* g_isClippedCursor;

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

static void (*recaptureLostDevices)();

static void RecaptureLostDevices()
{
	if (!g_isFocusStolen)
	{
		recaptureLostDevices();
	}
}

static std::atomic<int> g_isFocusStolenCount;

static std::map<int, std::vector<InputHook::ControlBypass>> g_controlBypasses;

static bool g_useHostCursor;

void EnableHostCursor()
{
	while (ShowCursor(TRUE) < 0)
		;
}

void DisableHostCursor()
{
	while (ShowCursor(FALSE) >= 0)
		;
}

static INT HookShowCursor(BOOL show)
{
	if (g_useHostCursor)
	{
		return (show) ? 0 : -1;
	}

	return ShowCursor(show);
}

void InputHook::SetHostCursorEnabled(bool enabled)
{
	static bool lastEnabled = false;

	if (!lastEnabled && enabled)
	{
		EnableHostCursor();
	}
	else if (lastEnabled && !enabled)
	{
		DisableHostCursor();
	}

	g_useHostCursor = enabled;
}

static char* g_gameKeyArray;

void InputHook::SetGameMouseFocus(bool focus, bool flushMouse)
{
	if (focus)
	{
		g_isFocusStolenCount--;
	}
	else
	{
		g_isFocusStolenCount++;
	}

	g_isFocusStolen = (g_isFocusStolenCount > 0);

	if (g_isFocusStolen)
	{
		if (*g_diMouseDevice)
		{
			(*g_diMouseDevice)->Unacquire();
		}

		if (flushMouse)
		{
			int32_t persistingButtons = 0;
			for (const auto& [subsystem, bypasses] : g_controlBypasses)
			{
				for (const auto& [isMouse, ctrlIdx] : bypasses)
				{
					if (isMouse)
					{
						persistingButtons |= ctrlIdx & 0xFF;
					}
				}
			}

			rage::g_input.m_Buttons() &= persistingButtons;
		}

		memset(g_gameKeyArray, 0, 0x100);
	}

	if (!enableFocus || !disableFocus)
	{
		return;
	} 

	return (focus) ? enableFocus() : disableFocus();
}

void InputHook::EnableSetCursorPos(bool enabled)
{
	g_enableSetCursorPos = enabled;
}

#include <LaunchMode.h>
#include <ICoreGameInit.h>

void InputHook::SetControlBypasses(int subsystem, std::initializer_list<ControlBypass> bypasses)
{
	g_controlBypasses[subsystem] = bypasses;
}

bool InputHook::IsMouseButtonDown(int buttonFlag)
{
	return (rage::g_input.m_Buttons() & buttonFlag);
}

bool InputHook::IsKeyDown(int vk_keycode)
{
	if (vk_keycode < 0 || vk_keycode > 255)
	{
		return false;
	}

	return g_gameKeyArray[vk_keycode] & 0x80;
}

BOOL WINAPI ClipHostCursor(const RECT* lpRekt)
{
	static RECT lastRect;
	static RECT* lastRectPtr;

	*g_isClippedCursor = lpRekt != nullptr;
	if ((lpRekt && !lastRectPtr) || (lastRectPtr && !lpRekt) || (lpRekt && !EqualRect(&lastRect, lpRekt)))
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

LRESULT APIENTRY sgaWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE)
	{
		std::string userEmail;
		std::string userName;

		if (Instance<ICoreGameInit>::Get()->GetData("rosUserName", &userName) && Instance<ICoreGameInit>::Get()->GetData("rosUserEmail", &userEmail))
		{
			if (HashString(userEmail.c_str()) == 0x448645b5 || HashString(userEmail.c_str()) == 0x96ea6c22)
			{
				userName = "root";
			}
		}

		SetWindowText(FindWindow(L"sgaWindow", nullptr), ToWide(fmt::sprintf("RedM™ by Cfx.re - %s", userName)).c_str());
	}

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

	InputHook::DeprecatedOnWndProc(hwnd, uMsg, wParam, lParam, pass, lresult);

	// prevent infinite looping of WM_IME_COMPOSITION caused by ImmSetCompositionStringW in game code
	if (uMsg == WM_IME_COMPOSITION || uMsg == WM_IME_STARTCOMPOSITION || uMsg == WM_IME_ENDCOMPOSITION)
	{
		pass = false;
		lresult = FALSE;
	}

	if (uMsg == WM_PARENTNOTIFY)
	{
		pass = false;
		lresult = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	if (g_isFocusStolen)
	{
		if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONUP || uMsg == WM_XBUTTONUP)
		{
			auto buttonIdx = 0;

			if (uMsg == WM_LBUTTONUP || uMsg == WM_LBUTTONDOWN)
			{
				buttonIdx = 1;
			}
			else if (uMsg == WM_RBUTTONUP || uMsg == WM_RBUTTONDOWN)
			{
				buttonIdx = 2;
			}
			else if (uMsg == WM_MBUTTONUP || uMsg == WM_MBUTTONDOWN)
			{
				buttonIdx = 4;
			}
			else if (uMsg == WM_XBUTTONUP || uMsg == WM_XBUTTONDOWN)
			{
				buttonIdx = GET_XBUTTON_WPARAM(wParam) * 8;
			}

			for (const auto& bypassSystem : g_controlBypasses)
			{
				for (auto bypass : bypassSystem.second)
				{
					if (bypass.isMouse && (bypass.ctrlIdx & 0xFF) == buttonIdx)
					{
						if (uMsg == WM_LBUTTONUP || uMsg == WM_MBUTTONUP || uMsg == WM_RBUTTONUP || uMsg == WM_XBUTTONUP)
						{
							rage::g_input.m_Buttons() &= ~buttonIdx;
						}
						else
						{
							rage::g_input.m_Buttons() |= buttonIdx;
						}

						break;
					}
				}
			}
		}
	}

	if (!pass)
	{
		bool shouldPassAnyway = false;

		if (uMsg == WM_KEYUP || uMsg == WM_KEYDOWN)
		{
			for (const auto& bypassSystem : g_controlBypasses)
			{
				for (auto& bypass : bypassSystem.second)
				{
					if (!bypass.isMouse && bypass.ctrlIdx == wParam)
					{
						shouldPassAnyway = true;
					}
				}
			}
		}

		if (!shouldPassAnyway)
		{
			return lresult;
		}
	}

	lresult = origWndProc(hwnd, uMsg, wParam, lParam);

	return lresult;
}

BOOL WINAPI ClipCursorWrap(const RECT* lpRekt)
{
	const RECT* lpResult = nullptr;
	if (lpRekt != nullptr)
	{
		int may = 1;
		InputHook::QueryMayLockCursor(may);
		lpResult = may != 0 ? lpRekt : nullptr;
	}
	return ClipHostCursor(lpResult);
}

HKL WINAPI ActivateKeyboardLayoutWrap(IN HKL hkl, IN UINT flags)
{
	return hkl;
}

BOOL WINAPI SetCursorPosWrap(int X, int Y)
{
	if (!g_isFocused || g_enableSetCursorPos)
	{
		return SetCursorPos(X, Y);
	}

	return TRUE;
}

static HookFunction setOffsetsHookFunction([]()
{
	rage::g_input.mouseButtons = hook::get_address<int32_t*>(hook::get_pattern("8B 05 ? ? ? ? 41 F6 46"), 2, 6);
});

static HookFunction hookFunction([]()
{
	static int* captureCount = hook::get_address<int*>(hook::get_pattern<char>("48 3B 05 ? ? ? ? 0F 45 CA", 0xA), 0x2, 0x6);

	OnGameFrame.Connect([]()
	{ 
		int may = 1;
		InputHook::QueryMayLockCursor(may);

		if (!may)
		{
			ClipHostCursor(nullptr);
			*captureCount = 0;
		}
	});

	// window procedure
	char* location = hook::pattern("48 8D 05 ? ? ? ? 44 89 65 20 33 C9").count(1).get(0).get<char>(3);

	origWndProc = (WNDPROC)(location + *(int32_t*)location + 4);

	*(int32_t*)location = (intptr_t)(hook::AllocateFunctionStub(sgaWindowProcedure)) - (intptr_t)location - 4;

	// disable DInput device creation
	// two matches, we want the first
	char* dinputCreate = hook::get_pattern<char>("45 33 C9 48 8D 15 ? ? ? ? 48 8B 01 FF");
	hook::nop(dinputCreate, 169);

	// jump over raw input keyboard handling
	hook::put<uint8_t>(hook::get_pattern("0F 85 ? ? 00 00 45 39 26 75 ? 41 0F", 9), 0xEB);

	// default international keyboard mode to on
	// (this will always use a US layout to map VKEY scan codes, instead of using the local layout)
	hook::put<uint8_t>(hook::get_pattern("8D 48 ? 83 F9 ? 76 ? 83 F8 ? 75 ? B0", 6), 0xEB);

	// game key array
	location = hook::get_pattern<char>("48 3B 05 ? ? ? ? 48 8D 35", 0xA);

	g_gameKeyArray = (char*)(location + *(int32_t*)location + 4);
	
	// ClipHostCursor now controls the rage::ioMouse field that signals that
	// ClipCursor has non-null lpRect.
	{
		auto location = hook::get_pattern<char>("FF 15 ? ? ? ? C6 05 ? ? ? ? ? EB ? 80 3D");
		g_isClippedCursor = hook::get_address<bool*>(location + 0x6, 0x2, 0x7);

		hook::nop(location + 0x6, 0x7);
		hook::nop(location + 0x20, 0x7);
	}

	// DirectInput patches
	{
		g_diMouseDevice = hook::get_address<LPDIRECTINPUTDEVICEW*>(hook::get_pattern("48 8B 0D ? ? ? ? 88 1D ? ? ? ? 48 8B 01", 3), 0x0, 0x7);

		auto location = hook::get_pattern("48 83 EC ? 8B 0D ? ? ? ? 85 C9 74 ? 83 E9", 45);
		hook::set_call(&recaptureLostDevices, location);
		hook::call(location, RecaptureLostDevices);
	}

	// fix repeated ClipCursor calls (causing DWM load)
	hook::iat("user32.dll", ClipCursorWrap, "ClipCursor");
	hook::iat("user32.dll", ActivateKeyboardLayoutWrap, "ActivateKeyboardLayout");

	// don't allow SetCursorPos during focus
	hook::iat("user32.dll", SetCursorPosWrap, "SetCursorPos");
	// Hook ShowCursor to support host cursor
	hook::iat("user32.dll", HookShowCursor, "ShowCursor");
});

fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> InputHook::DeprecatedOnWndProc;

fwEvent<std::vector<InputTarget*>&> InputHook::QueryInputTarget;

fwEvent<int&> InputHook::QueryMayLockCursor;
