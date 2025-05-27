#include "StdInc.h"

#if defined(GTA_FIVE)
#include <EntitySystem.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceCallbackComponent.h>

static InitFunction initFunction([]
{
	OnTriggerGameEventExt.Connect([](const GameEventData& data)
	{
		auto resman = Instance<fx::ResourceManager>::Get();
		auto rec = resman->GetComponent<fx::ResourceEventManagerComponent>();

		/*NETEV CEventName CLIENT
		/#*
		 * An event that is triggered when the game triggers an internal network event.
		 * CEventName can be any event name that GTA 5 throws, e.g.: "CEventShockingCarCrash".
		 * see: https://docs.fivem.net/docs/game-references/game-events/ for a list of known events.
		 *
		 * @param entities - All entities receiving/emitting the event, can be empty.
		 * @param eventEntity - Entity the event is related to.
		 * @param data - Extra event data.
		 #/
		declare function CEventName(entities: number[], eventEntity: number, data: any[]): void;
		*/
		
		rec->QueueEvent(std::string(data.name), std::string(data.argsData), "");
	});

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

	OnEntityDamaged.Connect([](const DamageEventMetaData& data)
	{
		auto resman = Instance<fx::ResourceManager>::Get();

		auto rec = resman->GetComponent<fx::ResourceEventManagerComponent>();

		/*NETEV entityDamaged CLIENT
		/#*
		 * An event that is triggered when an entity is *locally* damaged.
		 *
		 * @param victim - The entity which is damaged, or 0 if none.
		 * @param culprit - The damaging entity, or 0 if none.
		 * @param weapon - The hash of the weapon inflicting damage.
		 * @param baseDamage - The base amount of damage inflicted, discounting any modifiers. (This is hard coded as zero)
		 * @param isMelee - Weather the damage is inflicted via melee.
		 #/
		declare function entityDamaged(victim: number, culprit: number, weapon: number, baseDamage: number, isMelee: boolean): void;
		*/
		rec->QueueEvent2(
			"entityDamaged",
			{},
			data.victim ? rage::fwScriptGuid::GetGuidFromBase(data.victim) : 0,
			data.culprit ? rage::fwScriptGuid::GetGuidFromBase(data.culprit) : 0,
			int64_t(int(data.weapon)),
			data.baseDamage, data.isMelee);
	});
});
#endif
