/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "InputHook.h"
#include "Hooking.h"
#include <MinHook.h>

WNDPROC origWndProc;

bool g_isFocused = true;
static bool g_enableSetCursorPos = false;
static bool g_isFocusStolen = false;

static void(*disableFocus)();

static void DisableFocus()
{
	if (!g_isFocusStolen)
	{
		if (disableFocus)
		{
			disableFocus();
		}
	}
}

static void(*enableFocus)();

static void EnableFocus()
{
	if (!g_isFocusStolen)
	{
		if (enableFocus)
		{
			enableFocus();
		}
	}
}

void InputHook::SetGameMouseFocus(bool focus)
{
	g_isFocusStolen = !focus;

	if (enableFocus && disableFocus)
	{
		return (focus) ? enableFocus() : disableFocus();
	}
}

void InputHook::EnableSetCursorPos(bool enabled) {
	g_enableSetCursorPos = enabled;
}

LRESULT APIENTRY grcWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ACTIVATEAPP)
	{
		g_isFocused = (wParam) ? true : false;

		/*if (g_isFocused)
		{
			*(BYTE*)(0x1970A21) &= ~1;
		}
		else
		{
			*(BYTE*)(0x1970A21) |= 1;
		}*/
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

	if (!pass)
	{
		return lresult;
	}

	return CallWindowProc(origWndProc, hwnd, uMsg, wParam, lParam);
}

static void RepairInput()
{
	//char spillBuffer[256];

	// function 1 regarding input
	//((void(*)(char*, int))0x6366D0)(spillBuffer, 0);

	// unset XLive overlay showing flag - unblocks input
	/*if (*(uint8_t*)0x1970A21 & 1)
	{
		*(uint8_t*)0x1970A21 &= ~1;
	}*/
}

int LockEnabled()
{
	int retval = 1;

	//HookCallbacks::RunCallback(StringHash("mouseLock"), &retval);
	InputHook::QueryMayLockCursor(retval);

	return retval;
}


#if tenseventy
static void __declspec(naked) LockMouseDeviceHook()
{
	__asm
	{
		call LockEnabled

		test eax, eax
		jnz returnStuff

		mov dword ptr [esp + 4], 0

	returnStuff:
		cmp dword ptr ds:[188AB8Ch], 0
		push 623C37h
		retn
	}
}
#else
void(__cdecl* g_origLockMouseDevice)(BYTE a1);

static void __cdecl LockMouseDeviceHook(BYTE a1)
{
	if (!LockEnabled())
	{
		a1 = 0;
	}

	g_origLockMouseDevice(a1);
}
#endif

static char* fumble0;
static char* fumble1;

static double FumbleMouseStuff(void* a2, int axis)
{
	if (axis == 0)
	{
		return *(int32_t*)(fumble0) * 0.0078125;
	}
	else if (axis == 1)
	{
		return *(int32_t*)(fumble1) * 0.0078125;
	}

	return 0.0;
}

struct IRgscUi
{
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvOut) = 0;
};

static BOOL __stdcall RgscIsUiShown(void* self)
{
	return g_isFocusStolen;
}

static void __stdcall QueryRgscUI(IRgscUi* rgsc, REFIID iid, void** ppvOut)
{
	rgsc->QueryInterface(iid, ppvOut);

	intptr_t* vtbl = *(intptr_t**)(*ppvOut);
	hook::putVP(&vtbl[2], RgscIsUiShown);
}

void InitInputHook()
{
	// d3d fpu preserve, STILL FIXME move it elsewhere
	/*hook::put<uint8_t>(0x6213C7, 0x46);
	hook::put<uint8_t>(0x62140F, 0x16);
	hook::put<uint8_t>(0x621441, 0x46);
	hook::put<uint8_t>(0x621481, 0x86);
	hook::put<uint8_t>(0x6214C1, 0x26);*/

	// window procedure
	{
		//auto location = hook::get_pattern("80 7C 24 04 00 8A 54 24 08 0f 95 C1", 0x12);
		auto location = hook::get_pattern("C7 44 24 2C 08 00 00 00", 12);
		origWndProc = *(WNDPROC*)(location);
		*(WNDPROC*)(location) = grcWindowProcedure;
	}

	// ignore WM_ACTIVATEAPP deactivates (to fix the weird crashes)
	hook::nop(hook::get_pattern("8B DF 8B 7D 08 85 DB", 7), 2);

	// disable dinput
	hook::return_function(hook::get_pattern("8B 0D ? ? ? ? 83 EC 14 85 C9"));

	// some mouse stealer -- removes dinput and doesn't get toggled back (RepairInput instead fixes the mouse)
	//hook::jump(0x623C30, RepairInput);
	//hook::jump(hook::get_pattern("83 3D ? ? ? ? 00 74 7D 80 3D ? ? ? ? 00"), LockMouseDeviceHook);
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("83 3D ? ? ? ? 00 74 7D 80 3D ? ? ? ? 00"), LockMouseDeviceHook, (void**)&g_origLockMouseDevice);
	MH_EnableHook(MH_ALL_HOOKS);

	{
		auto location = hook::get_pattern<char>("50 FF 15 ? ? ? ? 5F 5E 5B C3");

		hook::jump(location + 10, RepairInput); // tail of above function
		hook::nop(location, 7); // ignore ShowCursor calls
	}

	// testing stuff (simplify function that returns mouse values sometimes and garbage other times)
	{
		auto location = hook::get_pattern<char>("51 8B 54 24 0C C7 04 24 00 00 00 00");
		hook::jump(location, FumbleMouseStuff);
		fumble0 = *(char**)(location + 0x12);
		fumble1 = *(char**)(location + 0x28);
	}

	// RGSC UI hook for overlay checking (on QueryInterface)
	{
		auto location = hook::get_pattern("51 FF 10 85 C0 0F 85 41 02 00 00", 1);
		hook::nop(location, 10);
		hook::call(location, QueryRgscUI);
	}
}

fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> InputHook::DeprecatedOnWndProc;

fwEvent<int&> InputHook::QueryMayLockCursor;
fwEvent<std::vector<InputTarget*>&> InputHook::QueryInputTarget;
