#pragma once

inline bool IsCL2()
{
#ifndef IS_FXSERVER
	if (wcsstr(GetCommandLineW(), L"cl2") != nullptr)
	{
		return true;
	}
#endif

	return false;
}

namespace launch
{
inline bool IsSDKGuest()
{
#ifndef IS_FXSERVER
	if (wcsstr(GetCommandLineW(), L"dkguest") != nullptr)
	{
		return true;
	}
#endif

	return false;
}
	
inline bool IsSDK()
{
#ifndef IS_FXSERVER
	if (wcsstr(GetCommandLineW(), L"fxdk") != nullptr && !IsSDKGuest())
	{
		return true;
	}
#endif

	return false;
}

inline const std::string& GetLaunchModeKey()
{
	static thread_local std::string launchKey = ([]()
	{
		if (IsSDK() || IsSDKGuest())
		{
			return "fxdk";
		}
		else if (IsCL2())
		{
			return "cl2";
		}

		return "";
	})();

	return launchKey;
}

inline std::string GetPrefixedLaunchModeKey(std::string_view prefix)
{
	const auto& lm = GetLaunchModeKey();

	if (lm.empty())
	{
		return "";
	}

	return std::string{ prefix } + lm;
}

inline const std::string& GetProductKey()
{
	static thread_local std::string launchKey = ([]()
	{
#ifdef IS_FXSERVER
		return "SV";
#elif defined(GTA_FIVE)
		return "Five";
#elif defined(IS_RDR3)
		return "RDR";
#else
		return "CFX";
#endif
	})();

	return launchKey;
}
}
