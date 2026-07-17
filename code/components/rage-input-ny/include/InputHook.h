/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <SharedInput.h>

#ifndef COMPILING_RAGE_INPUT_NY
#define INPUT_DECL __declspec(dllimport)
#else
#define INPUT_DECL __declspec(dllexport)
#endif

namespace InputHook
{
	extern INPUT_DECL fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> DeprecatedOnWndProc;

	extern INPUT_DECL fwEvent<int&> QueryMayLockCursor;

	extern INPUT_DECL fwEvent<std::vector<InputTarget*>&> QueryInputTarget;

	INPUT_DECL void SetGameMouseFocus(bool focus);

	INPUT_DECL void EnableSetCursorPos(bool enabled);
}
