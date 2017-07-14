#pragma once

#include <CoreConsole.h>

inline bool CfxIsSinglePlayer()
{
#ifdef _WIN32
	static bool isSinglePlayer = false;
	static bool isSinglePlayerSet = false;

	if (!isSinglePlayerSet)
	{
		if (wcsstr(GetCommandLineW(), L"-sp") != nullptr)
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
