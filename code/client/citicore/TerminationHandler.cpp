#include <StdInc.h>

#ifdef _WIN32
#include <ComponentLoader.h>
#include <ResumeComponent.h>

static bool (*g_crashLogHandler)(const char* path);

extern "C" DLL_EXPORT void SetCrashLogHandler(bool (*handler)(const char*))
{
	g_crashLogHandler = handler;
}

extern "C" DLL_EXPORT bool CoreCollectCrashLog(const char* path)
{
	if (g_crashLogHandler)
	{
		return g_crashLogHandler(path);
	}

	return false;
}

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
