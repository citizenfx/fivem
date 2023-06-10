#include <StdInc.h>

#include <Hooking.h>
#include <netObject.h>
#include <netSyncTree.h>
#include <GameInit.h>
#include <CrossBuildRuntime.h>

#include <atPool.h>
#include <Pool.h>

using TCreateCloneObjFn = rage::netObject* (*)(uint16_t objectId, uint8_t, int, int);
using TPoolPtr = atPoolBase*;

static TCreateCloneObjFn createCloneFuncs[(int)NetObjEntityType::Max];
static TPoolPtr validatePools[(int)NetObjEntityType::Max];
static uint32_t validatePoolHashes[(int)NetObjEntityType::Max];

static TPoolPtr GetValidatePool(int type)
{
	if (!validatePools[type])
	{
		auto poolHash = validatePoolHashes[type];
		auto poolPtr = rage::GetPoolBase(poolHash);
		validatePools[type] = poolPtr;
	}

	return validatePools[type];
}

namespace rage
{
netObject* CreateCloneObject(NetObjEntityType type, uint16_t objectId, uint8_t a2, int a3, int a4)
{
	auto pool = GetValidatePool((int)type);

	if (pool->GetCountDirect() >= pool->GetSize())
	{
#ifdef _DEBUG
		trace("CreateCloneObject - pool for %d (%d) is overloaded (%d of %d)\n", (int)type, objectId, pool->GetCountDirect(), pool->GetSize());
#endif

		return nullptr;
	}

	return createCloneFuncs[(int)type](objectId, a2, a3, a4);
}
}

static HookFunction hookFunction([]()
{
	OnKillNetworkDone.Connect([=]()
	{
		// clear cached validate pools
		std::fill(std::begin(validatePools), std::end(validatePools), nullptr);
	},
	1000);

	if (xbr::IsGameBuildOrGreater<1491>())
	{
		auto location = hook::get_pattern<char>("7F 2B 41 B9 B8 0A 00 00 C7", 61);
		createCloneFuncs[(int)NetObjEntityType::Object] = (TCreateCloneObjFn)hook::get_call(location);
		createCloneFuncs[(int)NetObjEntityType::Heli] = (TCreateCloneObjFn)hook::get_call(location + 0x98);
		createCloneFuncs[(int)NetObjEntityType::Door] = (TCreateCloneObjFn)hook::get_call(location + 0x135);
		createCloneFuncs[(int)NetObjEntityType::Boat] = (TCreateCloneObjFn)hook::get_call(location + 0x1CD);
		createCloneFuncs[(int)NetObjEntityType::Bike] = (TCreateCloneObjFn)hook::get_call(location + 0x265);
		createCloneFuncs[(int)NetObjEntityType::Automobile] = (TCreateCloneObjFn)hook::get_call(location + 0x2FD);
		createCloneFuncs[(int)NetObjEntityType::Animal] = (TCreateCloneObjFn)hook::get_call(location + 0x395);
		createCloneFuncs[(int)NetObjEntityType::Ped] = (TCreateCloneObjFn)hook::get_call(location + 0x42D);
		createCloneFuncs[(int)NetObjEntityType::Train] = (TCreateCloneObjFn)hook::get_call(location + 0x504);
		createCloneFuncs[(int)NetObjEntityType::Trailer] = (TCreateCloneObjFn)hook::get_call(location + 0x59C);
		createCloneFuncs[(int)NetObjEntityType::Player] = (TCreateCloneObjFn)hook::get_call(location + 0x633);
		createCloneFuncs[(int)NetObjEntityType::Submarine] = (TCreateCloneObjFn)hook::get_call(location + 0x6D0);
		createCloneFuncs[(int)NetObjEntityType::Plane] = (TCreateCloneObjFn)hook::get_call(location + 0x768);
		createCloneFuncs[(int)NetObjEntityType::PickupPlacement] = (TCreateCloneObjFn)hook::get_call(location + 0x800);
		createCloneFuncs[(int)NetObjEntityType::Pickup] = (TCreateCloneObjFn)hook::get_call(location + 0x898);
		createCloneFuncs[(int)NetObjEntityType::DraftVeh] = (TCreateCloneObjFn)hook::get_call(location + 0x930);
		createCloneFuncs[(int)NetObjEntityType::WorldState] = (TCreateCloneObjFn)hook::get_call(location + 0xA1B);
		createCloneFuncs[(int)NetObjEntityType::Horse] = (TCreateCloneObjFn)hook::get_call(location + 0xAB3);
		createCloneFuncs[(int)NetObjEntityType::Herd] = (TCreateCloneObjFn)hook::get_call(location + 0xB50);
		createCloneFuncs[(int)NetObjEntityType::GroupScenario] = (TCreateCloneObjFn)hook::get_call(location + 0xBED);
		createCloneFuncs[(int)NetObjEntityType::AnimScene] = (TCreateCloneObjFn)hook::get_call(location + 0xC8A);
		createCloneFuncs[(int)NetObjEntityType::PropSet] = (TCreateCloneObjFn)hook::get_call(location + 0xD27);
		createCloneFuncs[(int)NetObjEntityType::StatsTracker] = (TCreateCloneObjFn)hook::get_call(location + 0xDC3);
		createCloneFuncs[(int)NetObjEntityType::WorldProjectile] = (TCreateCloneObjFn)hook::get_call(location + 0xE60);
		createCloneFuncs[(int)NetObjEntityType::Persistent] = (TCreateCloneObjFn)hook::get_call(location + 0xEEE);
		createCloneFuncs[(int)NetObjEntityType::PedSharedTargeting] = (TCreateCloneObjFn)hook::get_call(location + 0xF4A);
		createCloneFuncs[(int)NetObjEntityType::CombatDirector] = (TCreateCloneObjFn)hook::get_call(location + 0xFA6);
		createCloneFuncs[(int)NetObjEntityType::PedGroup] = (TCreateCloneObjFn)hook::get_call(location + 0x1043);
		createCloneFuncs[(int)NetObjEntityType::Guardzone] = (TCreateCloneObjFn)hook::get_call(location + 0x10E0);
		createCloneFuncs[(int)NetObjEntityType::Incident] = (TCreateCloneObjFn)hook::get_call(location + 0x1179);
	}
	else if (xbr::IsGameBuildOrGreater<1436>())
	{
		auto location = hook::get_pattern<char>("7F 2B 41 B9 B8 0A 00 00 C7", 61);
		createCloneFuncs[(int)NetObjEntityType::Object] = (TCreateCloneObjFn)hook::get_call(location);
		createCloneFuncs[(int)NetObjEntityType::Heli] = (TCreateCloneObjFn)hook::get_call(location + 0x98);
		createCloneFuncs[(int)NetObjEntityType::Door] = (TCreateCloneObjFn)hook::get_call(location + 0x130);
		createCloneFuncs[(int)NetObjEntityType::Boat] = (TCreateCloneObjFn)hook::get_call(location + 0x1C8);
		createCloneFuncs[(int)NetObjEntityType::Bike] = (TCreateCloneObjFn)hook::get_call(location + 0x260);
		createCloneFuncs[(int)NetObjEntityType::Automobile] = (TCreateCloneObjFn)hook::get_call(location + 0x2F8);
		createCloneFuncs[(int)NetObjEntityType::Animal] = (TCreateCloneObjFn)hook::get_call(location + 0x390);
		createCloneFuncs[(int)NetObjEntityType::Ped] = (TCreateCloneObjFn)hook::get_call(location + 0x428);
		createCloneFuncs[(int)NetObjEntityType::Train] = (TCreateCloneObjFn)hook::get_call(location + 0x4FF);
		createCloneFuncs[(int)NetObjEntityType::Trailer] = (TCreateCloneObjFn)hook::get_call(location + 0x597);
		createCloneFuncs[(int)NetObjEntityType::Player] = (TCreateCloneObjFn)hook::get_call(location + 0x62E);
		createCloneFuncs[(int)NetObjEntityType::Submarine] = (TCreateCloneObjFn)hook::get_call(location + 0x6C6);
		createCloneFuncs[(int)NetObjEntityType::Plane] = (TCreateCloneObjFn)hook::get_call(location + 0x75E);
		createCloneFuncs[(int)NetObjEntityType::PickupPlacement] = (TCreateCloneObjFn)hook::get_call(location + 0x7F6);
		createCloneFuncs[(int)NetObjEntityType::Pickup] = (TCreateCloneObjFn)hook::get_call(location + 0x88E);
		createCloneFuncs[(int)NetObjEntityType::DraftVeh] = (TCreateCloneObjFn)hook::get_call(location + 0x926);
		createCloneFuncs[(int)NetObjEntityType::WorldState] = (TCreateCloneObjFn)hook::get_call(location + 0xA11);
		createCloneFuncs[(int)NetObjEntityType::Horse] = (TCreateCloneObjFn)hook::get_call(location + 0xAA9);
		createCloneFuncs[(int)NetObjEntityType::Herd] = (TCreateCloneObjFn)hook::get_call(location + 0xB46);
		createCloneFuncs[(int)NetObjEntityType::GroupScenario] = (TCreateCloneObjFn)hook::get_call(location + 0xBE3);
		createCloneFuncs[(int)NetObjEntityType::AnimScene] = (TCreateCloneObjFn)hook::get_call(location + 0xC7B);
		createCloneFuncs[(int)NetObjEntityType::PropSet] = (TCreateCloneObjFn)hook::get_call(location + 0xD13);
		createCloneFuncs[(int)NetObjEntityType::StatsTracker] = (TCreateCloneObjFn)hook::get_call(location + 0xDAA);
		createCloneFuncs[(int)NetObjEntityType::WorldProjectile] = (TCreateCloneObjFn)hook::get_call(location + 0xE47);
		createCloneFuncs[(int)NetObjEntityType::Persistent] = (TCreateCloneObjFn)hook::get_call(location + 0xED5);
		createCloneFuncs[(int)NetObjEntityType::PedSharedTargeting] = (TCreateCloneObjFn)hook::get_call(location + 0xF31);
		createCloneFuncs[(int)NetObjEntityType::CombatDirector] = (TCreateCloneObjFn)hook::get_call(location + 0xFCE);
		createCloneFuncs[(int)NetObjEntityType::PedGroup] = (TCreateCloneObjFn)hook::get_call(location + 0x106B);
		createCloneFuncs[(int)NetObjEntityType::Guardzone] = (TCreateCloneObjFn)hook::get_call(location + 0x1108);
		createCloneFuncs[(int)NetObjEntityType::Incident] = (TCreateCloneObjFn)hook::get_call(location + 0x11A1);
	}
	else
	{
		auto location = hook::get_pattern<char>("0F 8E ? 0F 00 00 41 8A 55", 20);
		createCloneFuncs[(int)NetObjEntityType::Object] = (TCreateCloneObjFn)hook::get_call(location);
		createCloneFuncs[(int)NetObjEntityType::Heli] = (TCreateCloneObjFn)hook::get_call(location + 0x6E);
		createCloneFuncs[(int)NetObjEntityType::Door] = (TCreateCloneObjFn)hook::get_call(location + 0xDD);
		createCloneFuncs[(int)NetObjEntityType::Boat] = (TCreateCloneObjFn)hook::get_call(location + 0x14B);
		createCloneFuncs[(int)NetObjEntityType::Bike] = (TCreateCloneObjFn)hook::get_call(location + 0x1B9);
		createCloneFuncs[(int)NetObjEntityType::Automobile] = (TCreateCloneObjFn)hook::get_call(location + 0x227);
		createCloneFuncs[(int)NetObjEntityType::Animal] = (TCreateCloneObjFn)hook::get_call(location + 0x29B);
		createCloneFuncs[(int)NetObjEntityType::Ped] = (TCreateCloneObjFn)hook::get_call(location + 0x30F);
		createCloneFuncs[(int)NetObjEntityType::Train] = (TCreateCloneObjFn)hook::get_call(location + 0x3B2);
		createCloneFuncs[(int)NetObjEntityType::Trailer] = (TCreateCloneObjFn)hook::get_call(location + 0x420);
		createCloneFuncs[(int)NetObjEntityType::Player] = (TCreateCloneObjFn)hook::get_call(location + 0x493);
		createCloneFuncs[(int)NetObjEntityType::Submarine] = (TCreateCloneObjFn)hook::get_call(location + 0x501);
		createCloneFuncs[(int)NetObjEntityType::Plane] = (TCreateCloneObjFn)hook::get_call(location + 0x56F);
		createCloneFuncs[(int)NetObjEntityType::PickupPlacement] = (TCreateCloneObjFn)hook::get_call(location + 0x5DE);
		createCloneFuncs[(int)NetObjEntityType::Pickup] = (TCreateCloneObjFn)hook::get_call(location + 0x64D);
		createCloneFuncs[(int)NetObjEntityType::DraftVeh] = (TCreateCloneObjFn)hook::get_call(location + 0x6BB);
		createCloneFuncs[(int)NetObjEntityType::WorldState] = (TCreateCloneObjFn)hook::get_call(location + 0x76E);
		createCloneFuncs[(int)NetObjEntityType::Horse] = (TCreateCloneObjFn)hook::get_call(location + 0x7E2);
		createCloneFuncs[(int)NetObjEntityType::Herd] = (TCreateCloneObjFn)hook::get_call(location + 0x850);
		createCloneFuncs[(int)NetObjEntityType::GroupScenario] = (TCreateCloneObjFn)hook::get_call(location + 0x8BF);
		createCloneFuncs[(int)NetObjEntityType::AnimScene] = (TCreateCloneObjFn)hook::get_call(location + 0x92D);
		createCloneFuncs[(int)NetObjEntityType::PropSet] = (TCreateCloneObjFn)hook::get_call(location + 0x99C);
		createCloneFuncs[(int)NetObjEntityType::StatsTracker] = (TCreateCloneObjFn)hook::get_call(location + 0xA0A);
		createCloneFuncs[(int)NetObjEntityType::WorldProjectile] = (TCreateCloneObjFn)hook::get_call(location + 0xA79);
		createCloneFuncs[(int)NetObjEntityType::Persistent] = (TCreateCloneObjFn)hook::get_call(location + 0xB1A);
		createCloneFuncs[(int)NetObjEntityType::PedSharedTargeting] = (TCreateCloneObjFn)hook::get_call(location + 0xB8E);
		createCloneFuncs[(int)NetObjEntityType::CombatDirector] = (TCreateCloneObjFn)hook::get_call(location + 0xC02);
		createCloneFuncs[(int)NetObjEntityType::PedGroup] = (TCreateCloneObjFn)hook::get_call(location + 0xC76);
		createCloneFuncs[(int)NetObjEntityType::Guardzone] = (TCreateCloneObjFn)hook::get_call(location + 0xCEA);
		createCloneFuncs[(int)NetObjEntityType::Incident] = (TCreateCloneObjFn)hook::get_call(location + 0xD56);	
	}


	validatePoolHashes[(int)NetObjEntityType::Animal] = HashString("CNetObjPedBase");
	validatePoolHashes[(int)NetObjEntityType::Automobile] = HashString("CNetObjVehicle");
	validatePoolHashes[(int)NetObjEntityType::Bike] = HashString("CNetObjVehicle");
	validatePoolHashes[(int)NetObjEntityType::Boat] = HashString("CNetObjVehicle");
	validatePoolHashes[(int)NetObjEntityType::Door] = HashString("CNetObjDoor");
	validatePoolHashes[(int)NetObjEntityType::Heli] = HashString("CNetObjVehicle");
	validatePoolHashes[(int)NetObjEntityType::Object] = HashString("CNetObjObject");
	validatePoolHashes[(int)NetObjEntityType::Ped] = HashString("CNetObjPedBase");
	validatePoolHashes[(int)NetObjEntityType::Pickup] = HashString("CNetObjPickup");
	validatePoolHashes[(int)NetObjEntityType::PickupPlacement] = HashString("CNetObjPickupPlacement");
	validatePoolHashes[(int)NetObjEntityType::Plane] = HashString("CNetObjVehicle");
	validatePoolHashes[(int)NetObjEntityType::Submarine] = HashString("CNetObjVehicle");
	validatePoolHashes[(int)NetObjEntityType::Player] = HashString("CNetObjPlayer");
	validatePoolHashes[(int)NetObjEntityType::Trailer] = HashString("CNetObjVehicle");
	validatePoolHashes[(int)NetObjEntityType::Train] = HashString("CNetObjVehicle");
	validatePoolHashes[(int)NetObjEntityType::DraftVeh] = HashString("CNetObjDraftVehicle");
	validatePoolHashes[(int)NetObjEntityType::StatsTracker] = HashString("CNetObjStatsTracker");
	validatePoolHashes[(int)NetObjEntityType::PropSet] = HashString("CNetObjPropSet");
	validatePoolHashes[(int)NetObjEntityType::AnimScene] = HashString("CNetObjAnimScene");
	validatePoolHashes[(int)NetObjEntityType::GroupScenario] = HashString("CNetObjGroupScenario");
	validatePoolHashes[(int)NetObjEntityType::Herd] = HashString("CNetObjHerd");
	validatePoolHashes[(int)NetObjEntityType::Horse] = HashString("CNetObjPedBase");
	validatePoolHashes[(int)NetObjEntityType::WorldState] = HashString("CNetObjWorldState");
	validatePoolHashes[(int)NetObjEntityType::WorldProjectile] = HashString("CNetObjProjectile");
	validatePoolHashes[(int)NetObjEntityType::Incident] = HashString("CNetObjIncident");
	validatePoolHashes[(int)NetObjEntityType::Guardzone] = HashString("CNetObjGuardzone");
	validatePoolHashes[(int)NetObjEntityType::PedGroup] = HashString("CNetObjPedGroup");
	validatePoolHashes[(int)NetObjEntityType::CombatDirector] = HashString("CNetObjCombatDirector");
	validatePoolHashes[(int)NetObjEntityType::PedSharedTargeting] = HashString("CNetObjPedSharedTargeting");
	validatePoolHashes[(int)NetObjEntityType::Persistent] = HashString("CNetObjPersistent");
});
