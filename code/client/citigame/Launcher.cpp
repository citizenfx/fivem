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

IGameSpecToHooks* g_hooksDLL;

__declspec(dllexport) void SetHooksDll(IGameSpecToHooks* dll);

bool LauncherInterface::PreLoadGame(void* cefSandbox)
{
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
	bool continueRunning = true;

	{
		DisableToolHelpScope scope;
		ComponentLoader::GetInstance()->DoGameLoad(hModule);
	}

	InitFunctionBase::RunAll();

	return continueRunning;
}

template<typename T>
void RunLifeCycleCallback(const T& cb)
{
	ComponentLoader::GetInstance()->ForAllComponents([&] (fwRefContainer<ComponentData> componentData)
	{
		auto& instances = componentData->GetInstances();

		if (instances.size())
		{
			auto& component = instances[0];

			auto lifeCycle = dynamic_component_cast<LifeCycleComponent*>(component.GetRef());

			if (lifeCycle)
			{
				cb(lifeCycle);
			}
		}
	});
}

bool LauncherInterface::PreResumeGame()
{
	RunLifeCycleCallback([] (LifeCycleComponent* component)
	{
		component->PreResumeGame();
	});

	return true;
}

bool LauncherInterface::PreInitializeGame()
{
	// make the component loader initialize
	ComponentLoader::GetInstance()->Initialize();

	// run callbacks on the component loader
    if (!getenv("CitizenFX_ToolMode"))
    {
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
