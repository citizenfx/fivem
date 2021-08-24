#pragma once

#pragma region GTA5_builds
inline bool Is2372()
{
#ifdef GTA_FIVE
	static bool retval = ([]()
	{
		if (wcsstr(GetCommandLineW(), L"b2372") != nullptr)
		{
			return true;
		}

		return false;
	})();

	return retval;
#endif

	return false;
}

inline bool Is2189()
{
#ifdef GTA_FIVE
	static bool retval = ([]()
	{
		if (wcsstr(GetCommandLineW(), L"b2189") != nullptr)
		{
			return true;
		}

		return false;
	})();

	return retval;
#endif

	return false;
}

inline bool Is2060()
{
#ifdef GTA_FIVE
	static bool retval = ([]()
	{
		if (wcsstr(GetCommandLineW(), L"b2060") != nullptr)
		{
			return true;
		}

		return false;
	})();

	return retval;
#endif

	return false;
}

inline bool Is372()
{
#ifdef GTA_FIVE
	static bool retval = ([]()
	{
		if (wcsstr(GetCommandLineW(), L"b372") != nullptr)
		{
			return true;
		}

		return false;
	})();

	return retval;
#endif

	return false;
}
#pragma endregion

#pragma region RDR3_builds
inline bool Is1311()
{
#ifdef IS_RDR3
	static bool retval = ([]()
	{
		if (wcsstr(GetCommandLineW(), L"b1311") != nullptr)
		{
			return true;
		}

		return false;
	})();

	return retval;
#endif

	return false;
}

inline bool Is1355()
{
#ifdef IS_RDR3
	static bool retval = ([]()
	{
		if (wcsstr(GetCommandLineW(), L"b1355") != nullptr)
		{
			return true;
		}

		return false;
	})();

	return retval;
#endif

	return false;
}

inline bool Is1436()
{
#ifdef IS_RDR3
	static bool retval = ([]()
	{
		if (wcsstr(GetCommandLineW(), L"b1436") != nullptr)
		{
			return true;
		}

		return false;
	})();

	return retval;
#endif

	return false;
}
#pragma endregion

namespace xbr
{
inline int GetGameBuild()
{
#ifdef GTA_FIVE
	static int build = ([]()
	{
		if (Is2372())
		{
			return 2372;
		}

		if (Is2189())
		{
			return 2189;
		}

		if (Is2060())
		{
			return 2060;
		}

		if (Is372())
		{
			return 372;
		}

		return 1604;
	})();
#elif IS_RDR3
	static int build = ([]()
	{
		if (Is1436())
		{
			return 1436;
		}

		if (Is1355())
		{
			return 1355;
		}

		return 1311;
	})();
#elif GTA_NY
	static int build = 43;
#else
	static int build = 0;
#endif

	return build;
}

template<int Build>
inline bool IsGameBuildOrGreater()
{
	return GetGameBuild() >= Build;
}

template<int Build>
inline bool IsGameBuild()
{
	return GetGameBuild() == Build;
}

#ifdef _WIN32
inline const wchar_t* GetGameWndClass()
{
	return
#if defined(IS_RDR3)
	L"sgaWindow"
#elif defined(GTA_FIVE) || defined(GTA_NY)
	L"grcWindow"
#else
	L"cfxUnkClass"
#endif
	;
}
#endif
}
