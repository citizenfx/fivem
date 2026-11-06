#include "StdInc.h"

#ifndef IS_FXSERVER
#include "ComponentLoader.h"
#include <ResumeComponent.h>

#include <Hooking.Aux.h>

#include <Error.h>

// Declared in WineCompat.Win32.cpp
void WineCompat_InstallHooks();

extern "C" DLL_EXPORT void EarlyMode_Init()
{
	// Install GetProcAddress hook to hide wine_get_version from legitimacy.dll
	// and adhesive.dll before either DLL is loaded.  Must come first so their
	// DllMain and static initialisers see a clean environment.
	WineCompat_InstallHooks();

	ComponentLoader* loader = ComponentLoader::GetInstance();

	putenv("CitizenFX_ToolMode=1");
	loader->Initialize();
	putenv("CitizenFX_ToolMode=");

	std::vector<fwRefContainer<Component>> components;

	for (auto compName : {
		"legitimacy",
		"adhesive" })
	{
		fwRefContainer<ComponentData> cliComponent = loader->LoadComponent(compName);
		components.push_back(cliComponent->CreateInstance(""));
	}

	DisableToolHelpScope thScope;

	for (auto& comp : components)
	{
		auto lifeCycle = dynamic_component_cast<LifeCycleComponent*>(comp.GetRef());

		if (lifeCycle)
		{
			lifeCycle->PreInitGame();
		}
	}
}
#endif
