#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "HookCallbacks.h"

WNDPROC origWndProc;

bool g_isFocused = true;

LRESULT APIENTRY grcWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ACTIVATEAPP)
	{
		g_isFocused = (wParam) ? true : false;

		if (g_isFocused)
		{
			*(BYTE*)(0x1970A21) &= ~1;
		}
		else
		{
			*(BYTE*)(0x1970A21) |= 1;
		}
	}

	if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
	{
		if (!g_isFocused)
		{
			return 0;
		}
	}

	WNDPROCARGS procArgs;
	procArgs.hwnd = hwnd;
	procArgs.uMsg = uMsg;
	procArgs.wParam = wParam;
	procArgs.lParam = lParam;
	procArgs.pass = true;

	HookCallbacks::RunCallback(StringHash("wndProc"), &procArgs);

	if (!procArgs.pass)
	{
		return procArgs.lresult;
	}

	return CallWindowProc(origWndProc, hwnd, uMsg, wParam, lParam);
}

static void RepairInput()
{
	char spillBuffer[256];

	// function 1 regarding input
	((void(*)(char*, int))0x6366D0)(spillBuffer, 0);

	// unset XLive overlay showing flag - unblocks input
	if (*(uint8_t*)0x1970A21 & 1)
	{
		*(uint8_t*)0x1970A21 &= ~1;
	}
}

static HookFunction hookFunction([] ()
{
	// d3d fpu preserve, FIXME move it elsewhere
	hook::put<uint8_t>(0x6213C7, 0x46);
	hook::put<uint8_t>(0x62140F, 0x16);
	hook::put<uint8_t>(0x621441, 0x46);
	hook::put<uint8_t>(0x621481, 0x86);
	hook::put<uint8_t>(0x6214C1, 0x26);

	// window procedure
	origWndProc = *(WNDPROC*)(0x637082);
	*(WNDPROC*)(0x637082) = grcWindowProcedure;

	// ignore WM_ACTIVATEAPP deactivates (to fix the weird crashes)
	*(WORD*)(0x61CCF1) = 0x9090;

	// experimental: disable dinput
	*(BYTE*)0x636490 = 0xC3; // keyboard

	// some mouse stealer -- removes dinput and doesn't get toggled back (RepairInput instead fixes the mouse)
	hook::jump(0x623C30, RepairInput);
});