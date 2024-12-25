#include <StdInc.h>

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <CfxState.h>

#define XBR_BUILDS_ONLY
#include <CrossBuildRuntime.h>

#include <shellapi.h>
#include <string>
#include <sstream>

void XBR_EarlySelect()
{
	// we *can't* call xbr:: APIs here since they'll `static`-initialize and break GameCache later
	std::wstring_view builds[] = {
#define EXPAND(_, __, x) \
	BOOST_PP_WSTRINGIZE(BOOST_PP_CAT(-b, x)),

		BOOST_PP_SEQ_FOR_EACH(EXPAND, , GAME_BUILDS)
#undef EXPAND
	};

	auto state = CfxState::Get();
	const auto realCli = (state->initCommandLine[0]) ? state->initCommandLine : GetCommandLineW();

	bool buildFlagFound = false;
	std::wstring connectionBuildArg;

	int argc;
	wchar_t** wargv = CommandLineToArgvW(realCli, &argc);
	for (int i = 1; i < argc; i++)
	{
		std::wstring_view arg = wargv[i];

		if (arg.find(L"://connect/") != std::wstring_view::npos)
		{
			std::wstringstream ss(arg.data());
			std::wstring connectionArg;
			while (std::getline(ss, connectionArg, L'?'))
			{
				if (connectionArg.find(L"-b") == 0)
				{
					connectionBuildArg = connectionArg;
				}
			}
		}

		for (auto build : builds)
		{
			if (arg == build)
			{
				buildFlagFound = true;
				break;
			}
		}
	}
	LocalFree(wargv);

	if (buildFlagFound || !state->IsMasterProcess())
	{
		return;
	}

	if (!connectionBuildArg.empty())
	{
		wcscat(state->initCommandLine, L" ");
		wcscat(state->initCommandLine, connectionBuildArg.c_str());
		return;
	}

	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	auto retainedBuild = GetPrivateProfileInt(L"Game", L"SavedBuildNumber", xbr::GetDefaultGameBuild(), fpath.c_str());

	// If there is no explicit build flag and retained build is not default - add flag to command line.
	if (retainedBuild != xbr::GetDefaultGameBuild())
	{
		wcscat(state->initCommandLine, va(L" -b%d", retainedBuild));
	}
}
#endif
