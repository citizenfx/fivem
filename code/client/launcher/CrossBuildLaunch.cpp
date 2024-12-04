#include <StdInc.h>

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <CfxState.h>

#define XBR_BUILDS_ONLY
#include <CrossBuildRuntime.h>

#include <shellapi.h>

void XBR_EarlySelect()
{
	uint32_t defaultBuild =
#ifdef GTA_FIVE
		1604
#elif defined(IS_RDR3)
		1311
#elif defined(GTA_NY)
		43
#else
		0
#endif
		;

	// specify a different saved build for first runs in release to save download time for first launch
	uint32_t initialBuild = defaultBuild;

#ifndef _DEBUG
#ifdef GTA_FIVE
	initialBuild = 2699;
#elif defined(IS_RDR3)
	initialBuild = 1491;
#endif
#endif

	// we *can't* call xbr:: APIs here since they'll `static`-initialize and break GameCache later
	uint32_t builds[] = {
#define EXPAND(_, __, x) x,
		BOOST_PP_SEQ_FOR_EACH(EXPAND, , GAME_BUILDS)
#undef EXPAND
	};

	bool buildFlagFound = false;

	auto state = CfxState::Get();
	const auto realCli = (state->initCommandLine[0]) ? state->initCommandLine : GetCommandLineW();

	int argc;
	wchar_t** wargv = CommandLineToArgvW(realCli, &argc);
	for (int i = 1; i < argc; i++)
	{
		std::wstring_view arg = wargv[i];

		for (uint32_t build : builds)
		{
			if (arg == fmt::sprintf(L"-b%d", build).c_str())
			{
				buildFlagFound = true;
				break;
			}
		}
	}

	if (!buildFlagFound && state->IsMasterProcess())
	{
		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
		auto retainedBuild = GetPrivateProfileInt(L"Game", L"SavedBuildNumber", initialBuild, fpath.c_str());

		// If there is no explicit build flag and retained build is not default - add flag to command line.
		if (retainedBuild != defaultBuild)
		{
			wcscat(state->initCommandLine, va(L" -b%d", retainedBuild));
		}
	}
}
#endif
