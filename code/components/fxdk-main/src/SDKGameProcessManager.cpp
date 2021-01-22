#include <StdInc.h>
#include <SDKGameProcessManager.h>

#include <HostSharedData.h>
#include <CfxState.h>
#include <ReverseGameData.h>
#include <CfxSubProcess.h>

static std::mutex gameProcessMutex;

void SDKGameProcessManager::StartGame()
{
	std::unique_lock<std::mutex> gameProcessMutexLock(gameProcessMutex);

	if (GetGameProcessState() != SDKGameProcessManager::GameProcessState::GP_STOPPED)
	{
		return;
	}

	SetGameProcessState(SDKGameProcessManager::GameProcessState::GP_STARTING);

	static HostSharedData<CfxState> hostData("CfxInitState");
	hostData->isReverseGame = true;

	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

	HANDLE nestHandles[] = { rgd->inputMutex, rgd->consumeSema, rgd->produceSema };

	// as we start at loading screen, limit to 60fps by default
	rgd->fpsLimit = 60;

	// prepare initial structures
	STARTUPINFOEX startupInfo = { 0 };
	startupInfo.StartupInfo.cb = sizeof(STARTUPINFOEX);

	SIZE_T size = 0;
	InitializeProcThreadAttributeList(NULL, 1, 0, &size);

	std::vector<uint8_t> attListData(size);
	auto attList = (LPPROC_THREAD_ATTRIBUTE_LIST)attListData.data();

	assert(attList);

	InitializeProcThreadAttributeList(attList, 1, 0, &size);
	UpdateProcThreadAttribute(attList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, &nestHandles, std::size(nestHandles) * sizeof(HANDLE), NULL, NULL);

	startupInfo.lpAttributeList = attList;

	gameProcessInfo = { 0 };

	auto processName = MakeCfxSubProcess(L"GameRuntime.exe", L"game");
	auto processCommand = const_cast<wchar_t*>(va(L"\"%s\" -dkguest -windowed", processName));

	BOOL result = CreateProcessW(processName, processCommand, nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, &startupInfo.StartupInfo, &gameProcessInfo);

	if (result)
	{
		// set the PID and create the game thread
		hostData->gamePid = gameProcessInfo.dwProcessId;
		ResumeThread(gameProcessInfo.hThread);

		SetGameProcessState(SDKGameProcessManager::GameProcessState::GP_RUNNING);

		std::thread([this]()
		{
			WaitForSingleObject(gameProcessInfo.hProcess, INFINITE);

			std::unique_lock<std::mutex> gameProcessMutexLock(gameProcessMutex);

			CloseHandle(gameProcessInfo.hProcess);

			rgd->inited = false;

			SetGameProcessState(SDKGameProcessManager::GameProcessState::GP_STOPPED);
		}).detach();
	}
	else
	{
		SetGameProcessState(SDKGameProcessManager::GameProcessState::GP_STOPPED);
	}
}

void SDKGameProcessManager::StopGame()
{
	if (GetGameProcessState() != SDKGameProcessManager::GameProcessState::GP_RUNNING)
	{
		return;
	}

	SetGameProcessState(SDKGameProcessManager::GameProcessState::GP_STOPPING);

	TerminateProcess(gameProcessInfo.hProcess, 0);
}

void SDKGameProcessManager::RestartGame()
{
	StopGame();
	StartGame();
}
