#pragma once

#include <CoreConsole.h>

inline bool CfxIsSinglePlayer()
{
#ifdef _WIN32
	static bool isSinglePlayer = false;
	static bool isSinglePlayerSet = false;

	if (!isSinglePlayerSet)
	{
		if (wcsstr(GetCommandLineW(), L" -sp") != nullptr || wcsstr(GetCommandLineW(), L"b372") != nullptr)
		{
			isSinglePlayer = true;
		}

		isSinglePlayerSet = true;
	}

	return isSinglePlayer;
#else
	return false;
#endif
}

inline bool CfxIsWine()
{
#ifdef _WIN32
	static bool isWine = false;
	static bool isWineSet = false;

	if (!isWineSet)
	{
		isWine = GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "wine_get_version") != nullptr;
		isWineSet = true;
	}

	return isWine;
#else
	return false;
#endif
}
