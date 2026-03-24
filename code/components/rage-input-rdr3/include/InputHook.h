#pragma once

#include <SharedInput.h>

#ifndef COMPILING_RAGE_INPUT_RDR3
#define INPUT_DECL __declspec(dllimport)
#else
#define INPUT_DECL __declspec(dllexport)
#endif

namespace rage
{
static constexpr uint32_t METHOD_RAW_INPUT = 0;
static constexpr uint32_t METHOD_DIRECTINPUT = 1;
static constexpr uint32_t METHOD_WINDOWS = 2;

struct ioMouse
{
	int32_t* mouseButtons;

	int32_t dummy = 0;
	int32_t& m_Buttons() { return mouseButtons ? *mouseButtons : dummy; };
};
inline ioMouse g_input;
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

	INPUT_DECL void SetGameMouseFocus(bool focus, bool flushMouse = true);

	INPUT_DECL void EnableSetCursorPos(bool enabled);

	INPUT_DECL bool IsMouseButtonDown(int buttonFlag);

	INPUT_DECL bool IsKeyDown(int vk_keycode);

	INPUT_DECL void SetHostCursorEnabled(bool enabled);

}
