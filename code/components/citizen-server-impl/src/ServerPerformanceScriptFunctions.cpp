#include "StdInc.h"

#include <ResourceManager.h>
#include <ScriptEngine.h>
#include <ScriptSerialization.h>
#include <ServerInstanceBaseRef.h>
#include <ServerPerfComponent.h>

namespace
{
struct ServerPerformanceData
{
	double tickTime;
	double tickTimeAverage;
	double tickTimeMax;
	double scriptTime;
	uint64_t frameCount;

	MSGPACK_DEFINE_MAP(tickTime, tickTimeAverage, tickTimeMax, scriptTime, frameCount);
};
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_SERVER_PERFORMANCE_DATA", [](fx::ScriptContext& context)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();
		auto snapshot = instance->GetComponent<fx::ServerPerfComponent>()->GetPerformanceSnapshot();

		context.SetResult(fx::SerializeObject(ServerPerformanceData{
			snapshot.tick.currentMilliseconds,
			snapshot.tick.averageMilliseconds,
			snapshot.tick.maximumMilliseconds,
			snapshot.script.currentMilliseconds,
			snapshot.tick.sampleCount
		}));
	});
});
