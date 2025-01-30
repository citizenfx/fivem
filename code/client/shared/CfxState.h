#pragma once

#include <HostSharedData.h>
#include <windows.h>
#include <cassert>

struct CfxState
{
	enum class ProductID
	{
		INVALID,
		FIVEM,
		REDM,
	};

	int initialLauncherPid;
	int initialGamePid;
	int gamePid;
	int gameBuild;

	bool inJobObject;
	bool running;
	bool ranPastInstaller;
	bool isReverseGame;

	wchar_t initPathGame[1024];
	wchar_t initPathLauncher[1024];
	
	wchar_t gameDirectory[1024];

	wchar_t gameExePath[1024];

	wchar_t initCommandLine[2048];

	// Link protocol name is used in app links like `fivem://connect/asdfgh`
	wchar_t linkProtocol[32];

	ProductID productId;

	CfxState()
	{
		memset(initPathGame, 0, sizeof(initPathGame));
		memset(initPathLauncher, 0, sizeof(initPathLauncher));
		memset(gameExePath, 0, sizeof(gameExePath));
		memset(gameDirectory, 0, sizeof(gameDirectory));
		memset(initCommandLine, 0, sizeof(initCommandLine));
		memset(linkProtocol, 0, sizeof(linkProtocol));

		initialGamePid = 0;
		initialLauncherPid = 0;

		running = true;
		gamePid = 0;
		gameBuild = -1;
		ranPastInstaller = false;
		inJobObject = false;
		isReverseGame = false;

		// initialize the game PID
		gamePid = 0;

		productId = ProductID::INVALID;
	}

	inline void SetLinkProtocol(const wchar_t* protocol)
	{
		wcsncpy(linkProtocol, protocol, std::size(linkProtocol) - 1);
	}

	inline const wchar_t* GetLinkProtocol()
	{
		assert(wcslen(linkProtocol));

		return linkProtocol;
	}
	inline const wchar_t* GetLinkProtocol(const wchar_t* appendix)
	{
		return va(L"%s%s", GetLinkProtocol(), appendix);
	}

	inline void SetProductID(ProductID pid)
	{
		productId = pid;
	}

	inline ProductID GetProductID()
	{
		assert(productId != ProductID::INVALID);

		return productId;
	}

	inline void SetGameBuild(const int build)
	{
		gameBuild = build;
	}

	inline int GetGameBuild()
	{
		assert(gameBuild != -1);
		return gameBuild;
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

	inline bool IsFiveM()
	{
		return GetProductID() == ProductID::FIVEM;
	}

	inline bool IsRedM()
	{
		return GetProductID() == ProductID::REDM;
	}

	inline static auto Get()
	{
		return HostSharedData<CfxState>{ "CfxInitState" };
	}
};

namespace cfx
{
	inline bool IsFiveM()
	{
		return CfxState::Get()->IsFiveM();
	}

	inline bool IsRedM()
	{
		return CfxState::Get()->IsRedM();
	}

	inline int GetGameBuild()
	{
		return CfxState::Get()->GetGameBuild();
	}

	inline bool IsMasterProcess()
	{
		return CfxState::Get()->IsMasterProcess();
	}
}
