#pragma once

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
