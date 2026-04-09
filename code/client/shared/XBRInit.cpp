#include "StdInc.h"

#ifndef IS_FXSERVER
#include "CrossBuildRuntime.h"

#include <HostSharedData.h>
#include <CfxState.h>

#include <shellapi.h>

namespace xbr
{
int GetRequestedGameBuildInit()
{
	constexpr const std::pair<std::wstring_view, int> buildNumbers[] = {
#define EXPAND(_, __, x) \
	{ BOOST_PP_WSTRINGIZE(BOOST_PP_CAT(-b, x)), x },

		BOOST_PP_SEQ_FOR_EACH(EXPAND, , GAME_BUILDS)

#undef EXPAND
	};

	constexpr const std::pair<const wchar_t*, int> deprecatedBuildMappings[] = {
		{ L"-b3717", xbr::Build::Winter_2025 }
	};

	auto sharedData = CfxState::Get();
	std::wstring_view cli = (sharedData->initCommandLine[0]) ? sharedData->initCommandLine : GetCommandLineW();
	auto buildNumber = GetDefaultGameBuild();

	int argc;
	wchar_t** wargv = CommandLineToArgvW(cli.data(), &argc);
	for (int i = 1; i < argc; i++)
	{
		if (buildNumber != GetDefaultGameBuild())
		{
			break;
		}

		std::wstring_view arg = wargv[i];

		for (auto [build, number] : buildNumbers)
		{
			if (arg == build)
			{
				buildNumber = number;
				break;
			}
		}

		for (const auto& [deprecatedBuild, mappedBuild] : deprecatedBuildMappings)
		{
			if (arg == deprecatedBuild)
			{
				buildNumber = mappedBuild;
				break;
			}
		}
	}
	LocalFree(wargv);

	return buildNumber;
}

bool GetReplaceExecutableInit()
{
	bool replaceExecutable = true;

	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		replaceExecutable = (GetPrivateProfileInt(L"Game", L"ReplaceExecutable", 1, fpath.c_str()) != 0);
	}

	return replaceExecutable;
}

}
#endif
