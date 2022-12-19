#include "StdInc.h"

#ifndef IS_FXSERVER
#include "CrossBuildRuntime.h"

#include <HostSharedData.h>
#include <CfxState.h>

namespace xbr
{
int GetGameBuildInit()
{
	constexpr const std::pair<std::wstring_view, int> buildNumbers[] = {
#define EXPAND(_, __, x) \
	{ BOOST_PP_WSTRINGIZE(BOOST_PP_CAT(b, x)), x },

		BOOST_PP_SEQ_FOR_EACH(EXPAND, , GAME_BUILDS)

#undef EXPAND
	};

	auto sharedData = CfxState::Get();
	std::wstring_view cli = (sharedData->initCommandLine[0]) ? sharedData->initCommandLine : GetCommandLineW();
	auto buildNumber = std::get<1>(buildNumbers[std::size(buildNumbers) - 1]);

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
}
#endif
