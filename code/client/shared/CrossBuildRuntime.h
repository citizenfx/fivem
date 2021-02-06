#pragma once

#pragma region GTA5_builds
inline bool Is2189()
{
#ifdef GTA_FIVE
	static bool retval = ([]()
	{
		if (wcsstr(GetCommandLine(), L"b2189") != nullptr)
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
		if (wcsstr(GetCommandLine(), L"b2060") != nullptr)
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
		if (wcsstr(GetCommandLine(), L"b372") != nullptr)
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
		if (wcsstr(GetCommandLine(), L"b1311") != nullptr)
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
		if (wcsstr(GetCommandLine(), L"b1355") != nullptr)
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
		if (Is1355())
		{
			return 1355;
		}

		return 1311;
	})();
#else
	build = 0;
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
}
