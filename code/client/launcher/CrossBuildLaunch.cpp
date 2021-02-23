#include <StdInc.h>

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <MinHook.h>
#include <Hooking.Aux.h>

static decltype(&GetCommandLineW) g_origGetCommandLineWBase;
static decltype(&GetCommandLineW) g_origGetCommandLineW32;
static decltype(&GetCommandLineA) g_origGetCommandLineABase;
static decltype(&GetCommandLineA) g_origGetCommandLineA32;

static uint32_t g_intendedBuild;

template<decltype(&g_origGetCommandLineWBase) TFn>
static LPWSTR GetCommandLineWStub()
{
	static LPWSTR cli = ([]()
	{
		auto lastCli = (*TFn)();

		static WCHAR cli[65536];
		WCHAR buf[128];
		swprintf(buf, L" -b%d", g_intendedBuild);

		wcscpy(cli, lastCli);
		wcscat(cli, buf);

		return cli;
	})();

	return cli;
}

template<decltype(&g_origGetCommandLineABase) TFn>
static LPSTR GetCommandLineAStub()
{
	static LPSTR cli = ([]()
	{
		auto lastCli = (*TFn)();

		static CHAR cli[65536];
		CHAR buf[128];
		sprintf(buf, " -b%d", g_intendedBuild);

		strcpy(cli, lastCli);
		strcat(cli, buf);

		return cli;
	})();

	return cli;
}

void XBR_EarlySelect()
{
	uint32_t defaultBuild =
#ifdef GTA_FIVE
		1604
#elif defined(IS_RDR3)
		1311
#else
		0
#endif
		;

	// we *can't* call xbr:: APIs here since they'll `static`-initialize and break GameCache later
	uint32_t builds[] = { 372, 1604, 2060, 2189, 1311, 1355 };
	uint32_t requestedBuild = defaultBuild;

	auto realCli = GetCommandLineW();

	for (uint32_t build : builds)
	{
		if (wcsstr(realCli, fmt::sprintf(L"b%d", build).c_str()) != nullptr)
		{
			requestedBuild = build;
			break;
		}
	}

	if (requestedBuild == defaultBuild)
	{
		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

		if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			auto retainedBuild = GetPrivateProfileInt(L"Game", L"SavedBuildNumber", defaultBuild, fpath.c_str());

			if (retainedBuild != defaultBuild)
			{
				g_intendedBuild = retainedBuild;

				DisableToolHelpScope scope;

				MH_Initialize();
				MH_CreateHookApi(L"kernelbase.dll", "GetCommandLineW", GetCommandLineWStub<&g_origGetCommandLineWBase>, (void**)&g_origGetCommandLineWBase);
				MH_CreateHookApi(L"kernel32.dll", "GetCommandLineW", GetCommandLineWStub<&g_origGetCommandLineW32>, (void**)&g_origGetCommandLineW32);
				MH_CreateHookApi(L"kernelbase.dll", "GetCommandLineA", GetCommandLineAStub<&g_origGetCommandLineABase>, (void**)&g_origGetCommandLineABase);
				MH_CreateHookApi(L"kernel32.dll", "GetCommandLineA", GetCommandLineAStub<&g_origGetCommandLineA32>, (void**)&g_origGetCommandLineA32);
				MH_EnableHook(MH_ALL_HOOKS);
			}
		}
	}
}
#endif
