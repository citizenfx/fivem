#pragma once

inline bool Is1868()
{
#ifdef GTA_FIVE
	if (wcsstr(GetCommandLine(), L"b1868") != nullptr)
	{
		return true;
	}
#endif

	return false;
}
