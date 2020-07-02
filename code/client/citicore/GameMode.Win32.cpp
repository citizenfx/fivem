#include "StdInc.h"

#ifndef IS_FXSERVER
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

class SDKMain
{
public:
	virtual void Run(fwRefContainer<Component> component) = 0;
};

DECLARE_INSTANCE_TYPE(SDKMain);

extern "C" DLL_EXPORT void GameMode_RunSDK()
{
	ComponentLoader* loader = ComponentLoader::GetInstance();
	loader->Initialize();

	// get the right component
	auto compName = "fxdk:main";

	fwRefContainer<ComponentData> mainComponent = loader->LoadComponent(compName);

    RunLifeCycleCallback([](LifeCycleComponent* component)
	{
		component->PreInitGame();
	});

	if (!mainComponent.GetRef())
	{
		FatalError("Could not obtain fxdk:main component, which is required for the SDK to start.\n");
		return;
	}

	std::set<std::string> depsLoaded;

	std::function<void(fwRefContainer<ComponentData>)> loadDeps = [&depsLoaded, &loadDeps](fwRefContainer<ComponentData> compRef)
	{
		if (depsLoaded.find(compRef->GetName()) != depsLoaded.end())
		{
			return;
		}

		for (auto& dep : compRef->GetDependencyDataList())
		{
			loadDeps(dep);
		}

		depsLoaded.insert(compRef->GetName());

		auto insts = compRef->GetInstances();

		if (insts.size() > 0 && insts[0].GetRef())
		{
			insts[0]->Initialize();
		}
	};

	loadDeps(mainComponent);

	// create a component instance
	fwRefContainer<Component> componentInstance = mainComponent->CreateInstance("");

	// check if the server initialized properly
	if (componentInstance.GetRef() == nullptr)
	{
		return;
	}

	Instance<SDKMain>::Get()->Run(componentInstance);
}
#endif
