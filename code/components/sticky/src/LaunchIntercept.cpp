#include <StdInc.h>

#include <CfxState.h>
#include <CfxSubProcess.h>

#include <HostSharedData.h>

#include <jitasm.h>

#include <CL2LaunchMode.h>
#include <CrossBuildRuntime.h>

void Component_RunPreInit()
{
	static HostSharedData<CfxState> hostData("CfxInitState");

	bool debugMode = false;

#ifdef GTA_FIVE
#ifdef _DEBUG
	debugMode = true;
#endif

	if (wcsstr(GetCommandLineW(), L"+run_lc"))
	{
		debugMode = false;
	}

	if (hostData->IsMasterProcess() && !debugMode)
	{
		auto processName = MakeCfxSubProcess(L"GameProcess.exe", Is372() ? L"game_372" : (!Is2060() ? L"game" : L"game_2060"));

		STARTUPINFOW si = { 0 };
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi;

		BOOL result = CreateProcessW(processName, GetCommandLineW(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi);

		if (result)
		{
			// set the PID and create the game thread
			hostData->gamePid = pi.dwProcessId;
			ResumeThread(pi.hThread);

			// wait for the game process to exit
			WaitForSingleObject(pi.hProcess, INFINITE);

			TerminateProcess(GetCurrentProcess(), 0);
		}
	}
	else if (hostData->IsMasterProcess() && debugMode)
	{
		hostData->gamePid = GetCurrentProcessId();
	}
#else
	if (hostData->IsMasterProcess())
	{
		hostData->gamePid = GetCurrentProcessId();
	}
#endif
}
