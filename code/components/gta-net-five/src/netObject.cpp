#include <StdInc.h>

#include <Hooking.h>
#include <netObject.h>
#include <netSyncTree.h>

using TCreateCloneObjFn = rage::netObject*(*)(uint16_t objectId, uint8_t, int, int);

static TCreateCloneObjFn createCloneFuncs[(int)NetObjEntityType::Max];

namespace rage
{
	netObject* CreateCloneObject(NetObjEntityType type, uint16_t objectId, uint8_t a2, int a3, int a4)
	{
		return createCloneFuncs[(int)type](objectId, 31, 0, 32);
	}
}

static HookFunction hookFunction([]()
{
	auto location = hook::get_pattern<char>("0F 8E 12 03 00 00 41 8A 57 2D", 22);

	createCloneFuncs[(int)NetObjEntityType::Ped]				= (TCreateCloneObjFn)hook::get_call(location);
	createCloneFuncs[(int)NetObjEntityType::Object]				= (TCreateCloneObjFn)hook::get_call(location + 0x39);
	createCloneFuncs[(int)NetObjEntityType::Heli]				= (TCreateCloneObjFn)hook::get_call(location + 0x72);
	createCloneFuncs[(int)NetObjEntityType::Door]				= (TCreateCloneObjFn)hook::get_call(location + 0xAB);
	createCloneFuncs[(int)NetObjEntityType::Boat]				= (TCreateCloneObjFn)hook::get_call(location + 0xE4);
	createCloneFuncs[(int)NetObjEntityType::Bike]				= (TCreateCloneObjFn)hook::get_call(location + 0x11D);
	createCloneFuncs[(int)NetObjEntityType::Automobile]			= (TCreateCloneObjFn)hook::get_call(location + 0x156);
	createCloneFuncs[(int)NetObjEntityType::Pickup]				= (TCreateCloneObjFn)hook::get_call(location + 0x18F);
	createCloneFuncs[(int)NetObjEntityType::Train]				= (TCreateCloneObjFn)hook::get_call(location + 0x1EF);
	createCloneFuncs[(int)NetObjEntityType::Trailer]			= (TCreateCloneObjFn)hook::get_call(location + 0x228);
	createCloneFuncs[(int)NetObjEntityType::Player]				= (TCreateCloneObjFn)hook::get_call(location + 0x261);
	createCloneFuncs[(int)NetObjEntityType::Submarine]			= (TCreateCloneObjFn)hook::get_call(location + 0x296);
	createCloneFuncs[(int)NetObjEntityType::Plane]				= (TCreateCloneObjFn)hook::get_call(location + 0x2C8);
	createCloneFuncs[(int)NetObjEntityType::PickupPlacement]	= (TCreateCloneObjFn)hook::get_call(location + 0x2FA);

	static_assert(offsetof(rage::CNetworkSyncDataULBase, isRemote) == 59, "offset 75");
	static_assert(offsetof(rage::CNetworkSyncDataULBase, creationAckedPlayers) == 96, "offset 112");
	static_assert(offsetof(CNetGamePlayer, physicalPlayerIndex) == 45, "offset 45");
	static_assert(offsetof(CNetGamePlayer, playerInfo) == 168, "offset 168");
});
