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

	struct MouseInputStruct 
	{
		int32_t mouseWheel;
		char pad_0028[24];
		int32_t mouseAbsX; // Based on your monitor's resolution. Zero when alt-tabbed and Zero in launcher
		int32_t mouseAbsY; // ^^^^^^^^^^^
		int32_t mouseDiffX; // in pixels
		int32_t lastMouseDiffX;
		int32_t mouseDiffY; 
		int32_t lastMouseDiffY; 
		char pad_0050[84]; 
		int32_t cursorAbsX; // Based on your monitor's resolution. center of screen when alt-tabbed (ex: 1280/720 on a 1440p screen)
		int32_t cursorAbsY; // ^^^^^^^^^^^^^^
		float cursorDiffX; // cursor pixel diff in float, but is always rounded to an even number
		float cursorDiffY; // ^^^^^^
		int32_t scrollDiffTicks; // unsure about this
		int32_t mouseButtons1; // these 3 mouse buttons show the same.
		char pad_00BC[4]; 
		int32_t mouseButtons2;
		int32_t mouseButtons3; // <---- This one was the one the original code was using.
		int32_t N00000048; // some sort of frame ticker, only goes up while alt-tabbed
		float mouseDiffDirectionX; //0x00CC -1.000[Left] -> 1.000[Right]
		float mouseDiffDirectionY; //0x00D0 -1.000[Up] -> 1.000[Down]
	};

	INPUT_DECL void SetControlBypasses(std::initializer_list<ControlBypass> bypasses);

	extern INPUT_DECL fwEvent<std::vector<InputTarget*>&> QueryInputTarget;

	extern INPUT_DECL fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> DeprecatedOnWndProc;

	extern INPUT_DECL fwEvent<int&> QueryMayLockCursor;

	INPUT_DECL void SetGameMouseFocus(bool focus);

	INPUT_DECL void EnableSetCursorPos(bool enabled);
}
