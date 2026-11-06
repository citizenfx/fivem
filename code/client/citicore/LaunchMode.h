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

// Returns true when running under Proton (Steam's Wine fork).
// Proton injects STEAM_COMPAT_DATA_PATH and/or PROTON_VERSION into the
// Wine environment, so we check those alongside the base Wine detection.
inline bool CfxIsProton()
{
#ifdef _WIN32
	static bool isProton = false;
	static bool isProtonSet = false;

	if (!isProtonSet)
	{
		isProton = CfxIsWine() &&
		           (getenv("STEAM_COMPAT_DATA_PATH") != nullptr ||
		            getenv("PROTON_VERSION") != nullptr);
		isProtonSet = true;
	}

	return isProton;
#else
	return false;
#endif
}
