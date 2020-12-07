#pragma once

#include <SharedInput.h>

#ifndef COMPILING_RAGE_INPUT_FIVE
#define INPUT_DECL __declspec(dllimport)
#else
#define INPUT_DECL __declspec(dllexport)
#endif

namespace InputHook
{
	struct ControlBypass
	{
		bool isMouse;
		int ctrlIdx; // (RAGE) VK if keyboard, mouse button bit if not
	};

	INPUT_DECL void SetControlBypasses(std::initializer_list<ControlBypass> bypasses);

	extern INPUT_DECL fwEvent<std::vector<InputTarget*>&> QueryInputTarget;

	extern INPUT_DECL fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> DeprecatedOnWndProc;

	extern INPUT_DECL fwEvent<int&> QueryMayLockCursor;

	INPUT_DECL void SetGameMouseFocus(bool focus);

	INPUT_DECL void EnableSetCursorPos(bool enabled);
}
