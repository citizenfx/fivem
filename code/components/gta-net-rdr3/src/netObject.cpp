#include <StdInc.h>

#include <Hooking.h>
#include <netObject.h>
#include <netSyncTree.h>

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

	validatePoolHashes[(int)NetObjEntityType::Animal] = 0xB040F3A2;
	validatePoolHashes[(int)NetObjEntityType::Automobile] = 0x365AB39F;
	validatePoolHashes[(int)NetObjEntityType::Bike] = 0x365AB39F;
	validatePoolHashes[(int)NetObjEntityType::Boat] = 0x365AB39F;
	validatePoolHashes[(int)NetObjEntityType::Door] = 0x3C787BB2;
	validatePoolHashes[(int)NetObjEntityType::Heli] = 0x365AB39F;
	validatePoolHashes[(int)NetObjEntityType::Object] = 0xB81C53E2;
	validatePoolHashes[(int)NetObjEntityType::Ped] = 0xB040F3A2;
	validatePoolHashes[(int)NetObjEntityType::Pickup] = 0x1CBCE31C;
	validatePoolHashes[(int)NetObjEntityType::PickupPlacement] = 0xF120209C;
	validatePoolHashes[(int)NetObjEntityType::Plane] = 0x365AB39F;
	validatePoolHashes[(int)NetObjEntityType::Submarine] = 0x365AB39F;
	validatePoolHashes[(int)NetObjEntityType::Player] = 0x886B3327;
	validatePoolHashes[(int)NetObjEntityType::Trailer] = 0x365AB39F;
	validatePoolHashes[(int)NetObjEntityType::Train] = 0x365AB39F;
	validatePoolHashes[(int)NetObjEntityType::DraftVeh] = 0xF05E4D7D;
	validatePoolHashes[(int)NetObjEntityType::StatsTracker] = 0x1EA58E5F;
	validatePoolHashes[(int)NetObjEntityType::PropSet] = 0xB80C0BF9;
	validatePoolHashes[(int)NetObjEntityType::AnimScene] = 0xD900F3E9;
	validatePoolHashes[(int)NetObjEntityType::GroupScenario] = 0x5EA00FF2;
	validatePoolHashes[(int)NetObjEntityType::Herd] = 0x63600CEB;
	validatePoolHashes[(int)NetObjEntityType::Horse] = 0xB040F3A2;
	validatePoolHashes[(int)NetObjEntityType::WorldState] = 0x781C9E02;
	validatePoolHashes[(int)NetObjEntityType::WorldProjectile] = 0x2F013899;
	validatePoolHashes[(int)NetObjEntityType::Incident] = 0x8069B054;
	validatePoolHashes[(int)NetObjEntityType::Guardzone] = 0x5CFB677C;
	validatePoolHashes[(int)NetObjEntityType::PedGroup] = 0x5CFB677C;
	validatePoolHashes[(int)NetObjEntityType::CombatDirector] = 0x212D594D;
	validatePoolHashes[(int)NetObjEntityType::PedSharedTargeting] = 0xD0228BA4;
	validatePoolHashes[(int)NetObjEntityType::Persistent] = 0xE9B12BBE;
});
