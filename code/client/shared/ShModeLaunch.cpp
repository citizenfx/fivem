#include <StdInc.h>

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <CfxState.h>

void SHMR_EarlySelect()
{
	auto state = CfxState::Get();
	const auto realCli = (state->initCommandLine[0]) ? state->initCommandLine : GetCommandLineW();

	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	auto shMode = GetPrivateProfileInt(L"Game", L"ShMode", 0, fpath.c_str());

	if (shMode && !wcsstr(realCli, L"sh"))
	{
		wcscat(state->initCommandLine, L"sh");
	}

}


#endif
