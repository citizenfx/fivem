#pragma once

#include <SharedInput.h>
#include <CrossBuildRuntime.h>

#ifndef COMPILING_RAGE_INPUT_FIVE
#define INPUT_DECL __declspec(dllimport)
#else
#define INPUT_DECL __declspec(dllexport)
#endif

// static global members in ioMouse - possible that padding may change
namespace rage
{
static constexpr uint32_t METHOD_RAW_INPUT = 0;
static constexpr uint32_t METHOD_DIRECTINPUT = 1;
static constexpr uint32_t METHOD_WINDOWS = 2;

struct ioMouse
{
	int32_t* mouseButtons;
	int32_t* mouseLastDX;
	int32_t* mouseLastDY;
	int32_t* mouseDX;
	int32_t* mouseDY;
	int32_t* mouseDZ;
	int32_t* mouseAbsX;
	int32_t* mouseAbsY;
	int32_t* cursorAbsX;
	int32_t* cursorAbsY;
	float*   mouseDiffDirectionX;
	float*   mouseDiffDirectionY;

	int32_t dummy = 0;
	float dummyf = 0;

	int32_t& m_Buttons(){ return mouseButtons ? *mouseButtons : dummy; }
	int32_t& m_lastDX(){ return mouseLastDX ? *mouseLastDX : dummy; }
	int32_t& m_lastDY(){ return mouseLastDY ? *mouseLastDY : dummy; }
	int32_t& m_dX(){ return mouseDX ? *mouseDX : dummy; }
	int32_t& m_dY(){ return mouseDY ? *mouseDY : dummy; }
	int32_t& m_dZ(){ return mouseDZ ? *mouseDZ : dummy; }
	int32_t& m_X(){ return mouseAbsX ? *mouseAbsX : dummy; }
	int32_t& m_Y(){ return mouseAbsY ? *mouseAbsY : dummy; }
	int32_t& m_cursorAbsX(){ return cursorAbsX ? *cursorAbsX : dummy; }
	int32_t& m_cursorAbsY(){ return cursorAbsY ? *cursorAbsY : dummy; }
	float& m_mouseDiffDirectionX(){ return mouseDiffDirectionX ? *mouseDiffDirectionX : dummyf; }
	float& m_mouseDiffDirectionY(){ return mouseDiffDirectionY ? *mouseDiffDirectionY : dummyf; }
};
inline ioMouse g_input;
}

#define INPUT_HOOK_HOST_CURSOR_SUPPORT

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

INPUT_DECL bool IsMouseButtonDown(int buttonFlag);

INPUT_DECL bool IsKeyDown(int vk_keycode);

INPUT_DECL void SetHostCursorEnabled(bool enabled);
}
