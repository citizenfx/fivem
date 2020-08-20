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
