#include "StdInc.h"

#ifndef IS_FXSERVER
#include "CrossBuildRuntime.h"

#include <HostSharedData.h>
#include <CfxState.h>

namespace xbr
{
int GetRequestedGameBuildInit()
{
	constexpr const std::pair<std::wstring_view, int> buildNumbers[] = {
#define EXPAND(_, __, x) \
	{ BOOST_PP_WSTRINGIZE(BOOST_PP_CAT(b, x)), x },

		BOOST_PP_SEQ_FOR_EACH(EXPAND, , GAME_BUILDS)

#undef EXPAND
	};

	auto sharedData = CfxState::Get();
	std::wstring_view cli = (sharedData->initCommandLine[0]) ? sharedData->initCommandLine : GetCommandLineW();
	// TODO: replace with default game build defined in CrossBuildRuntime.h.
	auto buildNumber = 1604;

	for (auto [build, number] : buildNumbers)
	{
		if (cli.find(build) != std::string_view::npos)
		{
			buildNumber = number;
			break;
		}
	}

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
