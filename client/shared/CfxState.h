#pragma once

#include <windows.h>

struct CfxState
{
	int initialPid;

	bool inJobObject;
	bool running;

	wchar_t initPath[1024];

	CfxState()
	{
		running = true;
		initialPid = GetCurrentProcessId();

		// get the init path
		wchar_t modulePath[512];
		GetModuleFileName(GetModuleHandle(nullptr), modulePath, sizeof(modulePath) / sizeof(wchar_t));

		// re-root to a real path
		wchar_t realModulePath[512];

		GetFullPathName(modulePath, _countof(realModulePath), realModulePath, nullptr);

		wchar_t* dirPtr = wcsrchr(realModulePath, L'\\');

		// we do not check if dirPtr happens to be 0, as any valid absolute Win32 file path contains at least one backslash
		dirPtr[1] = '\0';

		// copy the init path
		wcscpy_s(initPath, realModulePath);
	}

	inline bool IsMasterProcess()
	{
		return (initialPid == GetCurrentProcessId());
	}
};