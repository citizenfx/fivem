#pragma once

#include <windows.h>

struct CfxState
{
	int initialLauncherPid;
	int initialGamePid;
	int gamePid;

	bool inJobObject;
	bool running;
	bool ranPastInstaller;
	bool isReverseGame;

	wchar_t initPathGame[1024];
	wchar_t initPathLauncher[1024];

	CfxState()
	{
		memset(initPathGame, 0, sizeof(initPathGame));
		memset(initPathLauncher, 0, sizeof(initPathLauncher));

		initialGamePid = 0;
		initialLauncherPid = 0;

		running = true;
		gamePid = 0;
		ranPastInstaller = false;
		inJobObject = false;
		isReverseGame = false;

		// initialize the game PID
		gamePid = 0;
	}

	inline std::wstring GetInitPath()
	{
		wchar_t* pathRef =
#ifdef IS_LAUNCHER
			initPathLauncher;
#else
			initPathGame;
#endif

		if (!pathRef[0])
		{
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
			wcscpy_s(pathRef, std::size(initPathGame), realModulePath);
		}

		return pathRef;
	}

	inline void SetInitialPid(int pid)
	{
#ifdef IS_LAUNCHER
		initialLauncherPid = pid;
#else
		initialGamePid = pid;
#endif
	}

	inline int GetInitialPid()
	{
#ifdef IS_LAUNCHER
		if (!initialLauncherPid)
		{
			initialLauncherPid = GetCurrentProcessId();
		}
#else
		if (!initialGamePid)
		{
			initialGamePid = GetCurrentProcessId();
		}
#endif

#ifdef IS_LAUNCHER
		return initialLauncherPid;
#else
		return initialGamePid;
#endif
	}

	inline bool IsMasterProcess()
	{
#ifdef IS_LAUNCHER
		if (!initialLauncherPid)
		{
			initialLauncherPid = GetCurrentProcessId();
		}
#else
		if (!initialGamePid)
		{
			initialGamePid = GetCurrentProcessId();
		}
#endif

#ifdef IS_LAUNCHER
		return (initialLauncherPid == GetCurrentProcessId());
#else
		return (initialGamePid == GetCurrentProcessId());
#endif
	}

	inline bool IsGameProcess()
	{
		return (gamePid == GetCurrentProcessId());
	}
};
