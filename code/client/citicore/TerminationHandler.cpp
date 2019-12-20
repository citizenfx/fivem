#include <StdInc.h>

#ifdef _WIN32
#include <ComponentLoader.h>
#include <ResumeComponent.h>

extern "C" DLL_EXPORT void CoreOnProcessAbnormalTermination(void* reason)
{
	ComponentLoader* loader = ComponentLoader::GetInstance();

	loader->ForAllComponents([&](fwRefContainer<ComponentData> data)
	{
		auto& instances = data->GetInstances();

		if (!instances.empty())
		{
			fwRefContainer<Component> instance = instances[0];
			LifeCycleComponent* lcInstance = dynamic_component_cast<LifeCycleComponent*>(instance.GetRef());

			if (lcInstance != nullptr)
			{
				lcInstance->HandleAbnormalTermination(reason);
			}
		}
	});
}
#endif
