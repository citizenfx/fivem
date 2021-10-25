#include <StdInc.h>
#include <Hooking.h>

#include <netSyncTree.h>

const char* GetNetObjEntityName(uint16_t type)
{
	switch ((NetObjEntityType)type)
	{
	case NetObjEntityType::Animal: return "CNetObjAnimal";
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
	}

	return "Unknown";
}

static hook::cdecl_stub<rage::netSyncTree* (void*, int)> getSyncTreeForType([]()
{
	return hook::get_pattern("0F B7 CA 83 F9 0F 0F 87");
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, int flags, int flags2, rage::datBitBuffer* buffer, void* netLogStub)> netSyncTree_ReadFromBuffer([]()
{
	return hook::get_pattern("48 83 EC 40 48 83 B9 ? ? ? ? 00 49 8B F1", -13);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_CanApplyToObject([]()
{
	return hook::get_pattern("48 8B CE BB 01 00 00 00 E8 ? ? ? ? 49 8B 07", -0x2E);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_PrepareObject([]()
{
	return hook::get_pattern("48 8B F1 48 8B FA 48 8B 0D ? ? ? ? 48 8B 01 FF", -0xF);
});

namespace rage
{
bool netSyncTree::CanApplyToObject(netObject* object)
{
	return netSyncTree_CanApplyToObject(this, object);
}

bool netSyncTree::ReadFromBuffer(int flags, int flags2, rage::datBitBuffer* buffer, void* netLogStub)
{
	return netSyncTree_ReadFromBuffer(this, flags, flags2, buffer, netLogStub);
}

netSyncTree* netSyncTree::GetForType(NetObjEntityType type)
{
	return getSyncTreeForType(nullptr, (int)type);
}
}
