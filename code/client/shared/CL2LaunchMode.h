#pragma once

inline bool IsCL2()
{
	static auto isCl2 = ([]()
	{
#ifndef IS_FXSERVER
		if (wcsstr(GetCommandLineW(), L"cl2") != nullptr)
		{
			return true;
		}
#endif

		return false;
	})();

	return isCl2;
}

namespace launch
{
inline bool IsSDKGuest()
{
	static auto isSdkGuest = ([]()
	{
#ifndef IS_FXSERVER
		if (getenv("CitizenFX_SDK_Guest"))
		{
			return true;
		}
#endif

		return false;
	})();

	return isSdkGuest;
}

inline bool IsFXNode()
{
	static auto isFXNode = ([]()
	{
#ifndef IS_FXSERVER
		if (wcsstr(GetCommandLineW(), L"--start-node") != nullptr)
		{
			return true;
		}
#endif

		return false;
	})();

	return isFXNode;
}

inline bool IsSDK()
{
	static auto isSdk = ([]()
	{
#ifndef IS_FXSERVER
		if (wcsstr(GetCommandLineW(), L"fxdk") != nullptr && !IsSDKGuest())
		{
			return true;
		}
#endif

		return false;
	})();

	return isSdk;
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
