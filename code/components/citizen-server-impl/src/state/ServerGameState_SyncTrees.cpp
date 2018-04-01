#include "StdInc.h"
#include <state/SyncTrees_Five.h>

namespace fx
{
std::unique_ptr<sync::SyncTreeBase> MakeSyncTree(sync::NetObjEntityType objectType)
{
	switch (objectType)
	{
	case sync::NetObjEntityType::Automobile:
		return std::make_unique<sync::CAutomobileSyncTree>();
	case sync::NetObjEntityType::Bike:
		return std::make_unique<sync::CBikeSyncTree>();
	case sync::NetObjEntityType::Boat:
		return std::make_unique<sync::CBoatSyncTree>();
	case sync::NetObjEntityType::Door:
		return std::make_unique<sync::CDoorSyncTree>();
	case sync::NetObjEntityType::Heli:
		return std::make_unique<sync::CHeliSyncTree>();
	case sync::NetObjEntityType::Object:
		return std::make_unique<sync::CObjectSyncTree>();
	case sync::NetObjEntityType::Ped:
		return std::make_unique<sync::CPedSyncTree>();
	case sync::NetObjEntityType::Pickup:
		return std::make_unique<sync::CPickupSyncTree>();
	case sync::NetObjEntityType::PickupPlacement:
		return std::make_unique<sync::CPickupPlacementSyncTree>();
	case sync::NetObjEntityType::Plane:
		return std::make_unique<sync::CPlaneSyncTree>();
	case sync::NetObjEntityType::Submarine:
		return std::make_unique<sync::CSubmarineSyncTree>();
	case sync::NetObjEntityType::Player:
		return std::make_unique<sync::CPlayerSyncTree>();
	case sync::NetObjEntityType::Trailer:
		return std::make_unique<sync::CAutomobileSyncTree>();
	case sync::NetObjEntityType::Train:
		//return std::make_unique<sync::CTrainSyncTree>();
		break;
	}

	return {};
}
}
