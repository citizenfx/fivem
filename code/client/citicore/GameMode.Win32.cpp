#include "StdInc.h"

#include "ComponentLoader.h"
#include "ToolComponent.h"

#include <ResumeComponent.h>

#include <Error.h>

extern "C" DLL_EXPORT void GameMode_Init()
{
	ComponentLoader* loader = ComponentLoader::GetInstance();
	loader->Initialize();

	// TODO: init dep tree
	fwRefContainer<ComponentData> cliComponent = loader->LoadComponent("http-client");
	cliComponent->GetInstances()[0]->Initialize();

	fwRefContainer<ComponentData> gameComponent = loader->LoadComponent("citizen:game:main");
	fwRefContainer<ComponentData> nuiComponent = loader->LoadComponent("nui:core");
	nuiComponent->GetInstances()[0]->Initialize();
	(dynamic_cast<LifeCycleComponent*>(nuiComponent->GetInstances()[0].GetRef()))->PreInitGame();

	fwRefContainer<ComponentData> conComponent = loader->LoadComponent("conhost:v2");
	conComponent->GetInstances()[0]->Initialize();

	/*ComponentLoader::GetInstance()->ForAllComponents([&](fwRefContainer<ComponentData> componentData)
	{
		for (auto& instance : componentData->GetInstances())
		{
			instance->Initialize();
		}
	});*/

	if (!gameComponent.GetRef())
	{
		FatalError("Could not obtain citizen:game:main component, which is required for the game to start.\n");
		return;
	}

	gameComponent->GetInstances()[0]->Initialize();

	fwRefContainer<RunnableComponent> runnableGame = dynamic_component_cast<RunnableComponent*>(gameComponent->GetInstances()[0].GetRef());

	if (runnableGame.GetRef() != nullptr)
	{
		runnableGame->Run();
	}
	else
	{
		FatalError("citizen:game:main component does not implement RunnableComponent. Exiting.\n");
	}
}
