#pragma once

#include <SharedInput.h>

#ifndef COMPILING_RAGE_INPUT_PAYNE
#define INPUT_DECL __declspec(dllimport)
#else
#define INPUT_DECL __declspec(dllexport)
#endif

namespace InputHook
{
	extern INPUT_DECL fwEvent<std::vector<InputTarget*>&> QueryInputTarget;

	extern INPUT_DECL fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> DeprecatedOnWndProc;

	extern INPUT_DECL fwEvent<int&> QueryMayLockCursor;

	INPUT_DECL void SetGameMouseFocus(bool focus);

	INPUT_DECL void EnableSetCursorPos(bool enabled);
}
