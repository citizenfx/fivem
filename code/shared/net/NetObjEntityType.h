#pragma once

namespace fx::sync
{
	enum class NetObjEntityType
	{
#if defined(STATE_FIVE) || defined(GTA_FIVE)
		Automobile = 0,
		Bike = 1,
		Boat = 2,
		Door = 3,
		Heli = 4,
		Object = 5,
		Ped = 6,
		Pickup = 7,
		PickupPlacement = 8,
		Plane = 9,
		Submarine = 10,
		Player = 11,
		Trailer = 12,
		Train = 13,
		Max = 14,
 #elif defined(STATE_RDR3) || defined(IS_RDR3)
		Animal = 0,
		Automobile = 1,
		Bike = 2,
		Boat = 3,
		Door = 4,
		Heli = 5,
		Object = 6,
		Ped = 7,
		Pickup = 8,
		PickupPlacement = 9,
		Plane = 10,
		Submarine = 11,
		Player = 12,
		Trailer = 13,
		Train = 14,
		DraftVeh = 15,
		StatsTracker = 16,
		PropSet = 17,
		AnimScene = 18,
		GroupScenario = 19,
		Herd = 20,
		Horse = 21,
		WorldState = 22,
		WorldProjectile = 23,
		Incident = 24,
		Guardzone = 25,
		PedGroup = 26,
		CombatDirector = 27,
		PedSharedTargeting = 28,
		Persistent = 29,
		Max = 30,
#else
		Max = 0,
#endif
	};

	static const char* GetNetObjEntityName(uint16_t type)
	{
		switch ((NetObjEntityType)type)
		{
#if defined(STATE_FIVE) || defined(GTA_FIVE) || defined(STATE_RDR3) || defined(IS_RDR3)
			case NetObjEntityType::Automobile: return "CNetObjAutomobile";
			case NetObjEntityType::Bike: return "CNetObjBike";
			case NetObjEntityType::Boat: return "CNetObjBoat";
			case NetObjEntityType::Door: return "CNetObjDoor";
			case NetObjEntityType::Heli: return "CNetObjHeli";
			case NetObjEntityType::Object: return "CNetObjObject";
			case NetObjEntityType::Ped: return "CNetObjPed";
			case NetObjEntityType::Pickup: return "CNetObjPickup";
			case NetObjEntityType::PickupPlacement: return "CNetObjPickupPlacement";
			case NetObjEntityType::Plane: return "CNetObjPlane";
			case NetObjEntityType::Submarine: return "CNetObjSubmarine";
			case NetObjEntityType::Player: return "CNetObjPlayer";
			case NetObjEntityType::Trailer: return "CNetObjTrailer";
			case NetObjEntityType::Train: return "CNetObjTrain";
#endif
#if defined(STATE_RDR3) || defined(IS_RDR3)
			case NetObjEntityType::Animal: return "CNetObjAnimal";
			case NetObjEntityType::DraftVeh: return "CNetObjDraftVeh";
			case NetObjEntityType::StatsTracker: return "CNetObjStatsTracker";
			case NetObjEntityType::PropSet: return "CNetObjPropSet";
			case NetObjEntityType::AnimScene: return "CNetObjAnimScene";
			case NetObjEntityType::GroupScenario: return "CNetObjGroupScenario";
			case NetObjEntityType::Herd: return "CNetObjHerd";
			case NetObjEntityType::Horse: return "CNetObjHorse";
			case NetObjEntityType::WorldState: return "CNetObjWorldState";
			case NetObjEntityType::WorldProjectile: return "CNetObjWorldProjectile";
			case NetObjEntityType::Incident: return "CNetObjIncident";
			case NetObjEntityType::Guardzone: return "CNetObjGuardzone";
			case NetObjEntityType::PedGroup: return "CNetObjPedGroup";
			case NetObjEntityType::CombatDirector: return "CNetObjCombatDirector";
			case NetObjEntityType::PedSharedTargeting: return "CNetObjPedSharedTargeting";
			case NetObjEntityType::Persistent: return "CNetObjPersistent";
#endif
		}

		return "Unknown";
	}
} // namespace fx::sync
