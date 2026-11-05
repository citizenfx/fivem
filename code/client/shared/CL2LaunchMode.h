#pragma once

#include <string>
#include <string_view>

namespace launch
{
// Whole-argument match, skipping argv[0]: "cl2" in the install path or a connect URL must not count (#3400).
// Splits like CommandLineToArgvW (whitespace separates, quotes group), minus backslash escapes, which can't form a flag.
inline bool HasCommandLineArgument(std::wstring_view commandLine, std::wstring_view argument)
{
	bool isExecutable = true;
	size_t i = 0;

	while (i < commandLine.size())
	{
		if (commandLine[i] == L' ' || commandLine[i] == L'\t')
		{
			i++;
			continue;
		}

		std::wstring current;
		bool inQuotes = false;

		for (; i < commandLine.size() && (inQuotes || (commandLine[i] != L' ' && commandLine[i] != L'\t')); i++)
		{
			if (commandLine[i] == L'"')
			{
				inQuotes = !inQuotes;
			}
			else
			{
				current += commandLine[i];
			}
		}

		if (!isExecutable && current == argument)
		{
			return true;
		}

		isExecutable = false;
	}

	return false;
}

// Subprocesses like CEF or DumpServer don't get `-cl2`; MakeCfxSubProcess names their executable `<Product>_cl2_<type>` instead.
inline bool IsCL2SubProcessImage(std::wstring_view imagePath)
{
	auto imageName = imagePath.substr(imagePath.find_last_of(L"\\/") + 1);
	return imageName.find(L"_cl2_") != std::wstring_view::npos;
}
}

inline bool IsCL2()
{
	static auto isCl2 = ([]()
	{
#ifndef IS_FXSERVER
		if (launch::HasCommandLineArgument(GetCommandLineW(), L"-cl2"))
		{
			return true;
		}

		wchar_t imagePath[MAX_PATH] = { 0 };
		GetModuleFileNameW(nullptr, imagePath, _countof(imagePath));

		if (launch::IsCL2SubProcessImage(imagePath))
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
#elif defined(GTA_FIVE) || defined(IS_RDR3)
		return "CFXGame";
#else
		return "CFX";
#endif
	})();

	return launchKey;
}
}
