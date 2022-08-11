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

namespace sync
{
	template<typename TVisitor>
	bool VisitSyncTree(const SyncEntityPtr& entity, TVisitor&& cb)
	{
		auto st = entity->syncTree.get();

		switch (entity->type)
		{
#ifdef STATE_FIVE
			case sync::NetObjEntityType::Automobile:
				return static_cast<sync::CAutomobileSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Bike:
				return static_cast<sync::CBikeSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Boat:
				return static_cast<sync::CBoatSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Door:
				return static_cast<sync::CDoorSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Heli:
				return static_cast<sync::CHeliSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Object:
				return static_cast<sync::CObjectSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Ped:
				return static_cast<sync::CPedSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Pickup:
				return static_cast<sync::CPickupSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::PickupPlacement:
				return static_cast<sync::CPickupPlacementSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Plane:
				return static_cast<sync::CPlaneSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Submarine:
				return static_cast<sync::CSubmarineSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Player:
				return static_cast<sync::CPlayerSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Trailer:
				return static_cast<sync::CAutomobileSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Train:
				return static_cast<sync::CTrainSyncTree*>(st)->VisitT(cb);
#elif defined(STATE_RDR3)
			case sync::NetObjEntityType::Animal:
				return static_cast<sync::CAnimalSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Automobile:
				return static_cast<sync::CAutomobileSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Bike:
				return static_cast<sync::CBikeSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Boat:
				return static_cast<sync::CBoatSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Door:
				return static_cast<sync::CDoorSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Heli:
				return static_cast<sync::CHeliSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Object:
				return static_cast<sync::CObjectSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Ped:
				return static_cast<sync::CPedSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Pickup:
				return static_cast<sync::CPickupSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::PickupPlacement:
				return static_cast<sync::CPickupPlacementSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Plane:
				return static_cast<sync::CPlaneSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Submarine:
				return static_cast<sync::CSubmarineSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Player:
				return static_cast<sync::CPlayerSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Trailer:
				return static_cast<sync::CAutomobileSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Train:
				return static_cast<sync::CTrainSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::DraftVeh:
				return static_cast<sync::CDraftVehSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::StatsTracker:
				return static_cast<sync::CStatsTrackerSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::PropSet:
				return static_cast<sync::CPropSetSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::AnimScene:
				return static_cast<sync::CAnimSceneSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::GroupScenario:
				return static_cast<sync::CGroupScenarioSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Herd:
				return static_cast<sync::CHerdSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Horse:
				return static_cast<sync::CHorseSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::WorldState:
				return static_cast<sync::CWorldStateSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::WorldProjectile:
				return static_cast<sync::CWorldProjectileSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Incident:
				return static_cast<sync::CIncidentSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Guardzone:
				return static_cast<sync::CGuardzoneSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::PedGroup:
				return static_cast<sync::CPedGroupSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::CombatDirector:
				return static_cast<sync::CCombatDirectorSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::PedSharedTargeting:
				return static_cast<sync::CPedSharedTargetingSyncTree*>(st)->VisitT(cb);
			case sync::NetObjEntityType::Persistent:
				return static_cast<sync::CPersistentSyncTree*>(st)->VisitT(cb);
#endif
		}

		return false;
	}

	void IterateTimestamps(const SyncEntityPtr& entity, fu2::unique_function<void(uint32_t)>&& cb)
	{
		if (!cb)
		{
			return;
		}

		cb(entity->timestamp);

		VisitSyncTree(entity, [&cb](auto& node)
		{
			if constexpr (AffectsBlender<std::decay_t<decltype(node.node)>>())
			{
				cb(node.timestamp);
			}
		});
	}
}
}
