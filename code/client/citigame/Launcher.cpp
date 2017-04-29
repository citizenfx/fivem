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

#include <ComponentLoader.h>

IGameSpecToHooks* g_hooksDLL;

__declspec(dllexport) void SetHooksDll(IGameSpecToHooks* dll);

bool LauncherInterface::PreLoadGame(void* cefSandbox)
{
	bool continueRunning = true;

	// HooksDLL only exists for GTA_NY
#ifdef GTA_NY
	HooksDLLInterface::PreGameLoad(&continueRunning, &g_hooksDLL);
#endif

	// initialize component instances
    if (!getenv("CitizenFX_ToolMode"))
    {
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

	ComponentLoader::GetInstance()->DoGameLoad(hModule);

	// HooksDLL only exists for GTA_NY
#ifdef GTA_NY
	HooksDLLInterface::PostGameLoad(hModule, &continueRunning);
#endif

	InitFunctionBase::RunAll();

#if defined(GTA_NY)
	*entryPoint = (void(*)())0xD0D011;
#elif defined(PAYNE)
	// don't modify the entry point
	//*entryPoint = (void(*)())0;
#elif defined(GTA_FIVE)
#else
#error TODO: define entry point for this title
#endif

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