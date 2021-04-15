#pragma once

#include <SharedInput.h>

#ifndef COMPILING_RAGE_INPUT_FIVE
#define INPUT_DECL __declspec(dllimport)
#else
#define INPUT_DECL __declspec(dllexport)
#endif

// static global members in ioMouse - possible that padding may change
namespace rage
{
struct ioMouse
{
	int32_t m_dZ; // mousewheel
	char pad_0028[24];
	int32_t m_X; // Based on your monitor's resolution. Zero when alt-tabbed and Zero in launcher
	int32_t m_Y; // ^^^^^^^^^^^
	int32_t m_dX; // in pixels
	int32_t m_lastDX;
	int32_t m_dY;
	int32_t m_lastDY;
	char pad_0050[84];
	int32_t cursorAbsX; // Based on your monitor's resolution. center of screen when alt-tabbed (ex: 1280/720 on a 1440p screen)
	int32_t cursorAbsY; // ^^^^^^^^^^^^^^
	float cursorDiffX; // cursor pixel diff in float, but is always rounded to a whole number
	float cursorDiffY; // ^^^^^^
	int32_t scrollDiffTicks; // unsure about this
	int32_t m_InternalButtonsState;
	char pad_00BC[4];
	int32_t m_LastButtons;
	int32_t m_Buttons;
	int32_t N00000048; // some sort of frame ticker, only goes up while alt-tabbed
	float mouseDiffDirectionX; // -1.000[Left] -> 1.000[Right]
	float mouseDiffDirectionY; // -1.000[Up] -> 1.000[Down]
};
}
namespace InputHook
{
struct ControlBypass
{
	bool isMouse;
	int ctrlIdx; // (RAGE) VK if keyboard, mouse button bit if not
};

INPUT_DECL void SetControlBypasses(int subsystem, std::initializer_list<ControlBypass> bypasses);

extern INPUT_DECL fwEvent<std::vector<InputTarget*>&> QueryInputTarget;

extern INPUT_DECL fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> DeprecatedOnWndProc;

extern INPUT_DECL fwEvent<int&> QueryMayLockCursor;

INPUT_DECL void SetGameMouseFocus(bool focus);

INPUT_DECL void EnableSetCursorPos(bool enabled);
}
