#pragma once

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
