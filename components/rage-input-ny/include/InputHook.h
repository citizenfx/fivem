#pragma once

#ifndef COMPILING_RAGE_INPUT_NY
#define INPUT_DECL __declspec(dllimport)
#else
#define INPUT_DECL __declspec(dllexport)
#endif

namespace InputHook
{
	extern INPUT_DECL fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> OnWndProc;

	extern INPUT_DECL fwEvent<int&> QueryMayLockCursor;
}