#include <StdInc.h>

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <CfxState.h>

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
	initialBuild = 2545;
#elif defined(IS_RDR3)
	initialBuild = 1436;
#endif
#endif

	// we *can't* call xbr:: APIs here since they'll `static`-initialize and break GameCache later
	uint32_t builds[] = { 372, 1604, 2060, 2189, 2372, 2545, 2612, 1311, 1355, 1436, 43 };
	uint32_t requestedBuild = defaultBuild;

	auto state = CfxState::Get();
	const auto realCli = (state->initCommandLine[0]) ? state->initCommandLine : GetCommandLineW();

	for (uint32_t build : builds)
	{
		if (wcsstr(realCli, fmt::sprintf(L"b%d", build).c_str()) != nullptr)
		{
			requestedBuild = build;
			break;
		}
	}

	if (requestedBuild == defaultBuild && state->IsMasterProcess())
	{
		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

		auto retainedBuild = GetPrivateProfileInt(L"Game", L"SavedBuildNumber", initialBuild, fpath.c_str());

		// wcsstr is in case we have a `b1604` argument e.g. and we therefore want to ignore the saved build
		if (retainedBuild != defaultBuild && !wcsstr(realCli, va(L"b%d", defaultBuild)))
		{
			wcscat(state->initCommandLine, va(L" -b%d", retainedBuild));
		}
	}
}
#endif
