#include <StdInc.h>
#include <ResourceManager.h>
#include <Profiler.h>

#include <DrawCommands.h>

static InitFunction initFunction([]()
{
	OnRequestInternalScreenshot.Connect([](bool* should)
	{
		if (fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>()->IsRecording())
		{
			*should = true;
		}
	});

	OnInternalScreenshot.Connect([](const uint8_t* data, int width, int height)
	{
		if (fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>()->IsRecording())
		{
			fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>()->SubmitScreenshot(data, width, height);
		}
	});
});
