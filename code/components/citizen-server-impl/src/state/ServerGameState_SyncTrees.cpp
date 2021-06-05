#include "StdInc.h"
#include <state/SyncTrees.h>

namespace fx
{
std::shared_ptr<sync::SyncTreeBase> MakeSyncTree(sync::NetObjEntityType objectType)
{
	switch (objectType)
	{
#ifdef STATE_FIVE
	case sync::NetObjEntityType::Automobile:
		return std::make_shared<sync::CAutomobileSyncTree>();
	case sync::NetObjEntityType::Bike:
		return std::make_shared<sync::CBikeSyncTree>();
	case sync::NetObjEntityType::Boat:
		return std::make_shared<sync::CBoatSyncTree>();
	case sync::NetObjEntityType::Door:
		return std::make_shared<sync::CDoorSyncTree>();
	case sync::NetObjEntityType::Heli:
		return std::make_shared<sync::CHeliSyncTree>();
	case sync::NetObjEntityType::Object:
		return std::make_shared<sync::CObjectSyncTree>();
	case sync::NetObjEntityType::Ped:
		return std::make_shared<sync::CPedSyncTree>();
	case sync::NetObjEntityType::Pickup:
		return std::make_shared<sync::CPickupSyncTree>();
	case sync::NetObjEntityType::PickupPlacement:
		return std::make_shared<sync::CPickupPlacementSyncTree>();
	case sync::NetObjEntityType::Plane:
		return std::make_shared<sync::CPlaneSyncTree>();
	case sync::NetObjEntityType::Submarine:
		return std::make_shared<sync::CSubmarineSyncTree>();
	case sync::NetObjEntityType::Player:
		return std::make_shared<sync::CPlayerSyncTree>();
	case sync::NetObjEntityType::Trailer:
		return std::make_shared<sync::CAutomobileSyncTree>();
	case sync::NetObjEntityType::Train:
		return std::make_shared<sync::CTrainSyncTree>();
#elif defined(STATE_RDR3)
	case sync::NetObjEntityType::Animal:
		return std::make_shared<sync::CAnimalSyncTree>();
	case sync::NetObjEntityType::Automobile:
		return std::make_shared<sync::CAutomobileSyncTree>();
	case sync::NetObjEntityType::Bike:
		return std::make_shared<sync::CBikeSyncTree>();
	case sync::NetObjEntityType::Boat:
		return std::make_shared<sync::CBoatSyncTree>();
	case sync::NetObjEntityType::Door:
		return std::make_shared<sync::CDoorSyncTree>();
	case sync::NetObjEntityType::Heli:
		return std::make_shared<sync::CHeliSyncTree>();
	case sync::NetObjEntityType::Object:
		return std::make_shared<sync::CObjectSyncTree>();
	case sync::NetObjEntityType::Ped:
		return std::make_shared<sync::CPedSyncTree>();
	case sync::NetObjEntityType::Pickup:
		return std::make_shared<sync::CPickupSyncTree>();
	case sync::NetObjEntityType::PickupPlacement:
		return std::make_shared<sync::CPickupPlacementSyncTree>();
	case sync::NetObjEntityType::Plane:
		return std::make_shared<sync::CPlaneSyncTree>();
	case sync::NetObjEntityType::Submarine:
		return std::make_shared<sync::CSubmarineSyncTree>();
	case sync::NetObjEntityType::Player:
		return std::make_shared<sync::CPlayerSyncTree>();
	case sync::NetObjEntityType::Trailer:
		return std::make_shared<sync::CAutomobileSyncTree>();
	case sync::NetObjEntityType::Train:
		return std::make_shared<sync::CTrainSyncTree>();
	case sync::NetObjEntityType::DraftVeh:
		return std::make_shared<sync::CDraftVehSyncTree>();
	case sync::NetObjEntityType::StatsTracker:
		return std::make_shared<sync::CStatsTrackerSyncTree>();
	case sync::NetObjEntityType::PropSet:
		return std::make_shared<sync::CPropSetSyncTree>();
	case sync::NetObjEntityType::AnimScene:
		return std::make_shared<sync::CAnimSceneSyncTree>();
	case sync::NetObjEntityType::GroupScenario:
		return std::make_shared<sync::CGroupScenarioSyncTree>();
	case sync::NetObjEntityType::Herd:
		return std::make_shared<sync::CHerdSyncTree>();
	case sync::NetObjEntityType::Horse:
		return std::make_shared<sync::CHorseSyncTree>();
	case sync::NetObjEntityType::WorldState:
		return std::make_shared<sync::CWorldStateSyncTree>();
	case sync::NetObjEntityType::WorldProjectile:
		return std::make_shared<sync::CWorldProjectileSyncTree>();
	case sync::NetObjEntityType::Incident:
		return std::make_shared<sync::CIncidentSyncTree>();
	case sync::NetObjEntityType::Guardzone:
		return std::make_shared<sync::CGuardzoneSyncTree>();
	case sync::NetObjEntityType::PedGroup:
		return std::make_shared<sync::CPedGroupSyncTree>();
	case sync::NetObjEntityType::CombatDirector:
		return std::make_shared<sync::CCombatDirectorSyncTree>();
	case sync::NetObjEntityType::PedSharedTargeting:
		return std::make_shared<sync::CPedSharedTargetingSyncTree>();
	case sync::NetObjEntityType::Persistent:
		return std::make_shared<sync::CPersistentSyncTree>();
#endif
	}

	return {};
}
}
