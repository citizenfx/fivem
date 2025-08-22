#include "StdInc.h"
#include "InputHook.h"
#include "Hooking.h"
#include "CoreConsole.h"

#include <nutsnbolts.h>

#include <CfxState.h>

#include <CrossBuildRuntime.h>
#include <timeapi.h> // timeGetTime()

#include <ICoreGameInit.h>
#include <GlobalInput.h>
#include <dinput.h>

#include "ffx_antilag2_dx11.h"

static WNDPROC origWndProc;

typedef IDirectInputDeviceW* LPDIRECTINPUTDEVICEW;
static LPDIRECTINPUTDEVICEW* g_diMouseDevice = nullptr;

static bool g_isFocused = true;
static bool g_enableSetCursorPos = false;
static bool g_isFocusStolen = false;

static int* g_mouseButtons;
static int* g_inputOffset;
static rage::ioMouse* g_input;

static bool* g_isClippedCursor;

static bool g_useGlobalMouseMovement = true;
static POINT g_mouseMovement{ 0 };

static std::shared_ptr<GlobalInputHandler> GetGlobalInput()
{
	static auto handler = CreateGlobalInputHandler();

	return handler;
}

static void UpdateMouseMovement(const RAWMOUSE& mouse)
{
	if (mouse.usButtonFlags != RI_MOUSE_WHEEL && !(mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
	{
		g_mouseMovement.x += mouse.lLastX;
		g_mouseMovement.y += mouse.lLastY;
	}
}

static void (*disableFocus)();

static void DisableFocus()
{
	if (!g_isFocusStolen)
	{
		disableFocus();
	}
}

static void (*enableFocus)();

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

static BOOL ClipHostCursor(const RECT* lpRekt)
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

static std::atomic<int> g_isFocusStolenCount;

static std::map<int, std::vector<InputHook::ControlBypass>> g_controlBypasses;

void InputHook::SetGameMouseFocus(bool focus, bool flushMouse)
{
	if (focus)
	{
		g_isFocusStolenCount--;
	}
	else if (!focus)
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
				for (const auto [isMouse, ctrlIdx] : bypasses)
				{
					if (isMouse)
					{
						persistingButtons |= ctrlIdx & 0xFF;
					}
				}
			}
			rage::g_input.m_Buttons() &= persistingButtons;
		}

		memset(g_gameKeyArray, 0, 256);
	}

	return (!g_isFocusStolen) ? enableFocus() : disableFocus();
}

void InputHook::EnableSetCursorPos(bool enabled)
{
	g_enableSetCursorPos = enabled;
}

void InputHook::SetControlBypasses(int subsystem, std::initializer_list<ControlBypass> bypasses)
{
	g_controlBypasses[subsystem] = bypasses;
}

bool InputHook::IsMouseButtonDown(int buttonFlag)
{
	return ((*g_mouseButtons) & buttonFlag);
}

static bool GlobalInputIsDown(int vKey)
{
	static bool keys[512];

	static auto gInput = []()
	{
		auto inp = GetGlobalInput();

		inp->OnKey.Connect([](DWORD vKey, bool down)
		{
			keys[vKey] = down;
		});

		return inp;
	}();

	return keys[vKey];
}

bool InputHook::IsKeyDown(int vk_keycode)
{
	if (vk_keycode < 0 || vk_keycode > 255)
	{
		return false;
	}

	if (GlobalInputIsDown(vk_keycode))
	{
		return true;
	}

	return g_gameKeyArray[vk_keycode] & 0x80;
}

LRESULT APIENTRY grcWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// inform the game of being minimized
	static bool minimized = false;

	if (uMsg == WM_SIZE)
	{
		if (wParam == SIZE_MINIMIZED)
		{
			minimized = true;

			Instance<ICoreGameInit>::Get()->SetVariable("gameMinimized");
		}
		else if (minimized)
		{
			Instance<ICoreGameInit>::Get()->ClearVariable("gameMinimized");
			minimized = false;
		}
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

	if (uMsg == WM_INPUT && g_useGlobalMouseMovement)
	{
		pass = false;
	}

	if (!pass)
	{
		bool shouldPassAnyway = false;

		if (uMsg == WM_KEYUP || uMsg == WM_KEYDOWN)
		{
			for (const auto& bypassSystem : g_controlBypasses)
			{
				for (auto bypass : bypassSystem.second)
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

	//return CallWindowProc(origWndProc, hwnd, uMsg, wParam, lParam);
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
	if (!g_isFocusStolen || g_enableSetCursorPos)
	{
		return SetCursorPos(X, Y);
	}

	return TRUE;
}

#include <HostSharedData.h>
#include <ReverseGameData.h>

BOOL WINAPI RegisterRawInputDevicesWrap(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize)
{
	GetGlobalInput()->OnMouse.Connect([](const RAWMOUSE& mouse) { UpdateMouseMovement(mouse); });

	return RegisterRawInputDevices(pRawInputDevices, uiNumDevices, cbSize);
}

static void (*origSetInput)(int, void*, void*, void*);

struct ReverseGameInputState
{
	uint8_t keyboardState[256];
	int mouseDeltaX;
	int mouseDeltaY;
	int mouseWheel;
	int mouseButtons;

	ReverseGameInputState()
	{
		memset(keyboardState, 0, sizeof(keyboardState));
		mouseDeltaX = 0;
		mouseDeltaY = 0;
		mouseWheel = 0;
		mouseButtons = 0;
	}

	explicit ReverseGameInputState(const ReverseGameData& data)
	{
		memcpy(keyboardState, data.keyboardState, sizeof(keyboardState));
		mouseDeltaX = data.mouseDeltaX;
		mouseDeltaY = data.mouseDeltaY;
		mouseWheel = data.mouseWheel;
		mouseButtons = data.mouseButtons;
	}
};

static hook::cdecl_stub<bool(wchar_t, int)> ProcessWMChar([]()
{
	return hook::get_pattern("44 8B 0D ? ? ? ? 41 83 F9 1E 73");
});

static ReverseGameInputState lastInput;
static ReverseGameInputState curInput;

#include <queue>

extern void DoPatchMouseScrollDelta();

static HookFunction setOffsetsHookFunction([]()
{
	g_inputOffset = hook::get_address<int*>(hook::get_pattern("89 3D ? ? ? ? EB 0F 48 8B CB", 2));
	g_mouseButtons = hook::get_address<int*>(hook::get_pattern("FF 15 ? ? ? ? 85 C0 8B 05 ? ? ? ? 74 05", 10));

	rage::g_input.mouseButtons = hook::get_address<int32_t*>(hook::get_pattern("8B 05 ? ? ? ? 89 05 ? ? ? ? E8 ? ? ? ? 48 63"), 2, 6);
	rage::g_input.mouseLastDX = hook::get_address<int32_t*>(hook::get_pattern("83 25 ? ? ? ? 00 83 25 ? ? ? ? 00 48 8D 0D ? ? ? ? E8 ? ? ? ? 83"), 2, 7);
	rage::g_input.mouseLastDY = hook::get_address<int32_t*>(hook::get_pattern("83 25 ? ? ? ? 00 83 25 ? ? ? ? 00 48 8D 0D ? ? ? ? E8 ? ? ? ? 83", 7), 2, 7);
	rage::g_input.mouseDX = hook::get_address<int32_t*>(hook::get_pattern("D1 F8 89 05 ? ? ? ? E8 ? ? ? ? 83 25", 13), 2, 7);
	rage::g_input.mouseDY = hook::get_address<int32_t*>(hook::get_pattern("D1 F8 89 05 ? ? ? ? E8 ? ? ? ? 83 25", 20), 2, 7);
	rage::g_input.mouseDZ = hook::get_address<int32_t*>(hook::get_pattern("C1 E8 1F 03 D0 01 15", 7));
	rage::g_input.mouseAbsX = hook::get_address<int32_t*>(hook::get_pattern("8B 15 ? ? ? ? 8B 0D ? ? ? ? EB"), 2, 6);
	rage::g_input.mouseAbsY = hook::get_address<int32_t*>(hook::get_pattern("8B 15 ? ? ? ? 8B 0D ? ? ? ? EB", 6), 2, 6);
	rage::g_input.cursorAbsX = hook::get_address<int32_t*>(hook::get_pattern("8B 15 ? ? ? ? 8B 0D ? ? ? ? 2B C7"), 2, 6);
	rage::g_input.cursorAbsY = hook::get_address<int32_t*>(hook::get_pattern("8B 15 ? ? ? ? 8B 0D ? ? ? ? 2B C7", 6), 2, 6);
	rage::g_input.mouseDiffDirectionX = hook::get_address<float*>(hook::get_pattern("21 3D ? ? ? ? 21 3D ? ? ? ? 48 8B"), 2, 6);
	rage::g_input.mouseDiffDirectionY = hook::get_address<float*>(hook::get_pattern("21 3D ? ? ? ? 21 3D ? ? ? ? 48 8B", 6), 2, 6);

	// this has to be here as the mouseDZ pattern gets invalidated otherwise
	DoPatchMouseScrollDelta();
});

static AMD::AntiLag2DX11::Context g_antiLagContext = {};
static std::shared_ptr<ConVar<bool>> g_antiLagPresentConVar, g_antiLagEnabledConVar;

static void SetInputWrap(int a1, void* a2, void* a3, void* a4)
{
	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

	g_useGlobalMouseMovement = rgd->useRawMouseCapture;

	WaitForSingleObject(rgd->inputMutex, INFINITE);

	std::vector<InputTarget*> inputTargets;
	static bool lastBlockGameInput;
	bool blockGameInput = !InputHook::QueryInputTarget(inputTargets);

	if (g_antiLagPresentConVar->GetValue())
	{
		AMD::AntiLag2DX11::Update(&g_antiLagContext, g_antiLagEnabledConVar->GetValue(), 0);
	}

	curInput = ReverseGameInputState{ *rgd };

	if (g_useGlobalMouseMovement)
	{
		curInput.mouseDeltaX = g_mouseMovement.x;
		curInput.mouseDeltaY = g_mouseMovement.y;
	}

	g_mouseMovement.x = 0;
	g_mouseMovement.y = 0;
	rgd->mouseWheel = 0;

	rgd->mouseAbsX = std::clamp(rgd->mouseAbsX + curInput.mouseDeltaX, 0, rgd->twidth);
	rgd->mouseAbsY = std::clamp(rgd->mouseAbsY + curInput.mouseDeltaY, 0, rgd->theight);

	// handling switching between game input states
	if (blockGameInput != lastBlockGameInput)
	{
		static POINT mousePos[2];

		mousePos[blockGameInput ? 1 : 0] = { curInput.mouseDeltaX, curInput.mouseDeltaY };

		curInput.mouseDeltaX = mousePos[lastBlockGameInput ? 1 : 0].x;
		curInput.mouseDeltaY = mousePos[lastBlockGameInput ? 1 : 0].y;

		rgd->mouseDeltaX = curInput.mouseDeltaX;
		rgd->mouseDeltaY = curInput.mouseDeltaY;
	}

	lastBlockGameInput = blockGameInput;

	// propagate input to input targets
	if (inputTargets.size())
	{
		auto loopTargets = [&inputTargets](const auto& fn)
		{
			for (InputTarget* t : inputTargets)
			{
				fn(t);
			}
		};

		// attempt input synthesis for standard key up/down and mouse events
		for (size_t i = 0; i < std::size(curInput.keyboardState); i++)
		{
			bool pass = true;
			LRESULT lr;

			// TODO: key repeat
			if (curInput.keyboardState[i] && !lastInput.keyboardState[i])
			{
				loopTargets([i](InputTarget* target)
				{
					target->KeyDown(i, 0);
				});
			}
			else if (!curInput.keyboardState[i] && lastInput.keyboardState[i])
			{
				loopTargets([i](InputTarget* target)
				{
					target->KeyUp(i, 0);
				});
			}
		}

		// mouse buttons
		for (int i = 0; i < 3; i++)
		{
			bool pass = true;
			LRESULT lr;

			int mb = 1 << i;
			int mx = rgd->mouseAbsX;
			int my = rgd->mouseAbsY;

			if ((curInput.mouseButtons & mb) && !(lastInput.mouseButtons & mb))
			{
				loopTargets([i, mx, my](InputTarget* target)
				{
					target->MouseDown(i, mx, my);
				});
			}
			else if (!(curInput.mouseButtons & mb) && (lastInput.mouseButtons & mb))
			{
				loopTargets([i, mx, my](InputTarget* target)
				{
					target->MouseUp(i, mx, my);
				});
			}
		}

		// mouse wheel
		if (curInput.mouseWheel != lastInput.mouseWheel)
		{
			int mw = curInput.mouseWheel;

			loopTargets([mw](InputTarget* target)
			{
				target->MouseWheel(mw);
			});
		}

		// mouse movement
		if (curInput.mouseDeltaX != lastInput.mouseDeltaX || curInput.mouseDeltaY != lastInput.mouseDeltaY)
		{
			int mx = rgd->mouseAbsX;
			int my = rgd->mouseAbsY;

			loopTargets([mx, my](InputTarget* target)
			{
				target->MouseMove(mx, my);
			});
		}
	}

	lastInput = curInput;

	// keys typed
	{
		static uint16_t lastInputChar = 0;
		static uint32_t inputCharChangedAt = timeGetTime();

		auto currentInputChar = rgd->inputChar;
		bool inputCharChanged = currentInputChar != lastInputChar;

		lastInputChar = currentInputChar;

		if (currentInputChar > 0)
		{
			auto vkey = VkKeyScanW(lastInputChar);

			if (inputCharChanged)
			{
				inputCharChangedAt = timeGetTime();
				ProcessWMChar(currentInputChar, vkey);
			}
			else if (timeGetTime() - inputCharChangedAt > 300)
			{
				ProcessWMChar(currentInputChar, vkey);
			}
		}
	}

	if (!a1 && !blockGameInput)
	{
		int off = ((*g_inputOffset - 1) & 1) ? 1 : 0;

		// TODO: handle flush of keyboard
		memcpy(g_gameKeyArray, curInput.keyboardState, 256);
		if (off)
		{
			rage::g_input.m_lastDX() = curInput.mouseDeltaX;
			rage::g_input.m_lastDY() = curInput.mouseDeltaY;
		}
		else
		{
			rage::g_input.m_dX() = curInput.mouseDeltaX;
			rage::g_input.m_dY() = curInput.mouseDeltaY;
		}

		rage::g_input.m_cursorAbsX() = std::clamp(rage::g_input.m_cursorAbsX() + curInput.mouseDeltaX, 0, rgd->twidth);
		rage::g_input.m_cursorAbsY() = std::clamp(rage::g_input.m_cursorAbsY() + curInput.mouseDeltaY, 0, rgd->theight);

		rage::g_input.m_Buttons() = curInput.mouseButtons;
		rage::g_input.m_dZ() = curInput.mouseWheel;

		origSetInput(a1, a2, a3, a4);

		off = 0;

		if (!rgd->skipKeyboardStateCopyback)
		{
			memcpy(rgd->keyboardState, g_gameKeyArray, 256);
		}

		if (off)
		{
			rgd->mouseDeltaX = rage::g_input.m_lastDX();
			rgd->mouseDeltaY = rage::g_input.m_lastDY();
		}
		else
		{
			rgd->mouseDeltaX = rage::g_input.m_dX();
			rgd->mouseDeltaY = rage::g_input.m_dY();
		}
		rgd->mouseButtons = rage::g_input.m_Buttons();
		rgd->mouseWheel = rage::g_input.m_dZ();
	}

	ReleaseMutex(rgd->inputMutex);
}

// testing valid gamepads, similar to how the game does it
#if 0
#include <Xinput.h>
#ifdef _MSC_VER
#pragma comment(lib, "Xinput9_1_0")
#endif
#endif

static void (*origIOPadUpdate)(void*, bool);
static void* ioPadArray = nullptr;

// This hook is used for ReverseGame Gamepad input
static void rage__ioPad__Update(rage::ioPad* thisptr, bool onlyVibrate)
{
	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
	WaitForSingleObject(rgd->inputMutex, INFINITE);

#if 0
	XINPUT_STATE state;
	DWORD dwUserIndex = ((uintptr_t)out_RageIOPadState - (uintptr_t)ioPadArray) / 96/*sizeof(RageIOPad)*/;
	if( !XInputGetState( dwUserIndex, &state ) )
	{
		trace("Pad is Valid\n");
	}
#endif

	origIOPadUpdate(thisptr, onlyVibrate);

	// save this from the original function
	rgd->gamepad.lastButtonFlags = thisptr->lastButtonFlags;

	// apply gamepad struct to the game
	*thisptr = rgd->gamepad;

	ReleaseMutex(rgd->inputMutex);
}

static int Return0()
{
	return 0;
}

static HookFunction hookFunction([]()
{
	g_antiLagPresentConVar = std::make_shared<ConVar<bool>>("game_amdAntiLagPresent", ConVar_Internal | ConVar_ScriptRestricted, false);
	g_antiLagEnabledConVar = std::make_shared<ConVar<bool>>("game_useAmdAntiLag", ConVar_Archive | ConVar_UserPref, true);
	if (const auto antiLagInitResult = AMD::AntiLag2DX11::Initialize(&g_antiLagContext); antiLagInitResult == S_OK)
	{
		g_antiLagPresentConVar->GetHelper()->SetRawValue(true);
	}

	static int* captureCount = hook::get_address<int*>(hook::get_pattern("48 3B 05 ? ? ? ? 0F 45 CA 89 0D ? ? ? ? 48 83 C4 28", 12));

	OnGameFrame.Connect([]()
	{
		SetInputWrap(-1, NULL, NULL, NULL);

		int may = 1;
		InputHook::QueryMayLockCursor(may);

		if (!may)
		{
			ClipHostCursor(nullptr);
			*captureCount = 0;
		}
	});

	// window procedure
	char* location = xbr::IsGameBuildOrGreater<2545>() ? hook::pattern("48 8D 05 ? ? ? ? 33 C9 89 75 20 4C 89 7D").count(1).get(0).get<char>(3) : hook::pattern("48 8D 05 ? ? ? ? 33 C9 44 89 75 20 4C 89 7D").count(1).get(0).get<char>(3);

	origWndProc = (WNDPROC)(location + *(int32_t*)location + 4);

	hook::put<int32_t>(location, (intptr_t)(hook::AllocateFunctionStub(grcWindowProcedure)) - (intptr_t)location - 4);

	// disable mouse focus function
	void* patternMatch = hook::pattern("74 0D 38 1D ? ? ? ? 74 05 E8 ? ? ? ? 48 39").count(1).get(0).get<void>(10);
	hook::set_call(&disableFocus, patternMatch);
	hook::call(patternMatch, DisableFocus);

	patternMatch = hook::pattern("74 0D 38 1D ? ? ? ? 74 05 E8 ? ? ? ? 33 C9 E8").count(1).get(0).get<void>(10);
	hook::set_call(&enableFocus, patternMatch);
	hook::call(patternMatch, EnableFocus);

	patternMatch = hook::pattern("48 83 EC ? 8B 0D ? ? ? ? 85 C9 74 ? FF C9 74 ? FF C9 75").count(1).get(0).get<void>(53);
	hook::set_call(&recaptureLostDevices, patternMatch);
	hook::call(patternMatch, RecaptureLostDevices);

	g_diMouseDevice = hook::get_address<LPDIRECTINPUTDEVICEW*>(hook::get_pattern("48 8B 0D ? ? ? ? 48 8B 01 FF 50 ? 83 F8 ? 7F ? 48 83 C4", 3));

	// game key array
	location = hook::pattern("BF 00 01 00 00 48 8D 1D ? ? ? ? 48 3B 05").count(1).get(0).get<char>(8);

	g_gameKeyArray = (char*)(location + *(int32_t*)location + 4);

	// ClipHostCursor now controls the rage::ioMouse field that signals that
	// ClipCursor has non-null lpRect.
	{
		auto location = hook::get_pattern<char>("FF 15 ? ? ? ? C6 05 ? ? ? ? ? EB 18");
		g_isClippedCursor = hook::get_address<bool*>(location + 0x6, 0x2, 0x7);

		hook::nop(location + 0x6, 0x7);
		hook::nop(location + 0x20, 0x7);
	}

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

	// default international keyboard mode to on
	// (this will always use a US layout to map VKEY scan codes, instead of using the local layout)
	hook::put<uint8_t>(hook::get_pattern("8D 48 EF 41 3B CE 76 0C", 6), 0xEB);

	// fix repeated ClipCursor calls (causing DWM load)
	hook::iat("user32.dll", ClipCursorWrap, "ClipCursor");
	hook::iat("user32.dll", ActivateKeyboardLayoutWrap, "ActivateKeyboardLayout");

	// don't allow SetCursorPos during focus
	hook::iat("user32.dll", SetCursorPosWrap, "SetCursorPos");

	hook::iat("user32.dll", HookShowCursor, "ShowCursor");

	// NOTE: this specific flow should *only* be used if RG!
	// there's a call above in OnGameFrame which handles all other ancillary responsibilities
	static HostSharedData<CfxState> initState("CfxInitState");

	if (initState->isReverseGame)
	{
		if (launch::IsSDKGuest())
		{
			hook::iat("user32.dll", RegisterRawInputDevicesWrap, "RegisterRawInputDevices");
		}

		auto location = hook::get_pattern("45 33 C9 44 8A C7 40 8A D7 33 C9 E8", 11);
		hook::set_call(&origSetInput, location);
		hook::call(location, SetInputWrap);

		// NOP these out in ReverseGame so the cursor will work
		// Cursor X(Raw Input)
		char* loc = hook::pattern("89 0D ? ? ? ? 8B 85").count(1).get(0).get<char>(0);
		hook::nop(loc, 6);
		// Cursor Y(Raw Input)
		loc += 31;
		hook::nop(loc, 6);

		// Hook the GamePad sampling function so we can inject events into it from Reverse Game.
		auto ioPadUpdate = hook::get_pattern("41 8A D6 48 8B CF E8 ? ? FF FF 48 83 C7 60", 6);
		hook::set_call(&origIOPadUpdate, ioPadUpdate);
		hook::call(ioPadUpdate, rage__ioPad__Update);

		// NOP the function that changes mouse Input methods, this forces raw input
		auto changeMouseInput = hook::get_pattern("0F 84 ? ? ? ? 8B CB E8 ? ? ? ? E9", 8);
		hook::nop(changeMouseInput, 5);
	}

	// cancel out ioLogitechLedDevice
	hook::jump(hook::get_pattern("85 C0 0F 85 ? ? 00 00 48 8B CB FF 15", -0x77), Return0);

	// hook up cursor lock to CMousePointer::_bIsVisible
	static auto isPointerVisible = hook::get_address<bool*>(hook::get_pattern("80 3D ? ? ? ? 00 0F 45 C6 88 05 ? ? ? ? 48 8B 5C", 10), 2, 6) + 2;

	InputHook::QueryMayLockCursor.Connect([](int& may)
	{
		if (*isPointerVisible)
		{
			may = FALSE;
		}
	});

	
	{
		char* location = (char*)hook::get_pattern("48 8D 05 ? ? ? ? 48 2B C8 48 B8 AB AA AA AA AA");
		int offset = *(int*)(location + 3);
		ioPadArray = location + offset + 7;
	}
});

fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> InputHook::DeprecatedOnWndProc;

fwEvent<std::vector<InputTarget*>&> InputHook::QueryInputTarget;

fwEvent<int&> InputHook::QueryMayLockCursor;
