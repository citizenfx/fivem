/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "LauncherInterface.h"
#include "Launcher.h"
#include "CrossLibraryInterfaces.h"
#include "ResumeComponent.h"

#include "Hooking.Aux.h"

#include <ComponentLoader.h>
#include <CrossBuildRuntime.h>

#include <HostSharedData.h>
#include <CfxState.h>

#include <UUIState.h>

extern void DisableNvCache();

IGameSpecToHooks* g_hooksDLL;

__declspec(dllexport) void SetHooksDll(IGameSpecToHooks* dll);

bool LauncherInterface::PreLoadGame(void* cefSandbox)
{
	// if we don't have adhesive, set our gamePid
	static HostSharedData<CfxState> initState("CfxInitState");

	if (initState->IsMasterProcess())
	{
		const auto& map = ComponentLoader::GetInstance()->GetKnownComponents();

		if (map.find("adhesive") == map.end())
		{
			initState->gamePid = initState->initialGamePid;
		}
	}
	
	bool continueRunning = true;

	// initialize component instances
    if (!getenv("CitizenFX_ToolMode"))
    {
		DisableToolHelpScope scope;

        ComponentLoader::GetInstance()->ForAllComponents([&] (fwRefContainer<ComponentData> componentData)
        {
            for (auto& instance : componentData->GetInstances())
            {
                instance->Initialize();
            }
        });
    }

	SetHooksDll(g_hooksDLL);

	// and start running the game
	return continueRunning;
}

bool LauncherInterface::PostLoadGame(HMODULE hModule, void(**entryPoint)())
{
	static HostSharedData<UpdaterUIState> uuiState("CfxUUIState");

	uuiState->SetText(1, "Analyzing game data");
	uuiState->SetProgress(100.0);
	uuiState->OpenWhenExpired();

	bool continueRunning = true;

	{
		DisableToolHelpScope scope;
		ComponentLoader::GetInstance()->DoGameLoad(hModule);
	}

	InitFunctionBase::RunAll();

	return continueRunning;
}

bool LauncherInterface::PreResumeGame()
{
	RunLifeCycleCallback([] (LifeCycleComponent* component)
	{
		component->PreResumeGame();
	});

	static HostSharedData<UpdaterUIState> uuiState("CfxUUIState");
	uuiState->Close();

	return true;
}

bool LauncherInterface::PreInitializeGame()
{
	std::thread([]()
	{
		DisableNvCache();
	})
	.detach();

	// make the component loader initialize
	ComponentLoader::GetInstance()->Initialize();

	// run callbacks on the component loader
    if (!getenv("CitizenFX_ToolMode"))
    {
		DisableToolHelpScope thScope;

        RunLifeCycleCallback([] (LifeCycleComponent* component)
        {
            component->PreInitGame();
        });
    }

	return true;
}

static LauncherInterface g_launcherInterface;

extern "C" __declspec(dllexport) ILauncherInterface* GetLauncherInterface()
{
	return &g_launcherInterface;
}

extern "C" __declspec(dllexport) int GetGameVersion()
{
	return xbr::GetGameBuild();
}
