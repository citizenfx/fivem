#include "StdInc.h"

#if defined(GTA_FIVE)
#include <EntitySystem.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceCallbackComponent.h>

static InitFunction initFunction([]
{
	OnTriggerGameEvent.Connect([](const GameEventMetaData& data)
	{
		auto resman = Instance<fx::ResourceManager>::Get();

		auto rec = resman->GetComponent<fx::ResourceEventManagerComponent>();

		std::vector<int32_t> argTable(data.numArguments);

		for (size_t arg = 0; arg < data.numArguments; arg++)
		{
			argTable[arg] = int32_t(data.arguments[arg]);
		}

		/*NETEV gameEventTriggered CLIENT
		/#*
		 * An event that is triggered when the game triggers an internal network event.
		 *
		 * @param name - The name of the triggered event.
		 * @param data - The type-specific event data.
		 #/
		declare function gameEventTriggered(name: string, data: number[]): void;
		*/
		rec->QueueEvent2("gameEventTriggered", {}, std::string(data.name), argTable);
	});
});
#endif
