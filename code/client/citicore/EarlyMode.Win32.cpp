#include "StdInc.h"

#ifndef IS_FXSERVER
#include "ComponentLoader.h"
#include <ResumeComponent.h>

#include <Hooking.Aux.h>

#include <Error.h>

extern "C" DLL_EXPORT void EarlyMode_Init()
{
	ComponentLoader* loader = ComponentLoader::GetInstance();

	putenv("CitizenFX_ToolMode=1");
	loader->Initialize();
	putenv("CitizenFX_ToolMode=");

	std::vector<fwRefContainer<Component>> components;

	for (auto compName : {
#ifdef IS_RDR3
		"ros-patches:rdr3",
#else
		"ros-patches:five",
#endif
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
