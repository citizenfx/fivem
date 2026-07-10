#include <StdInc.h>

#include <Hooking.h>
#include <netObject.h>
#include <netSyncTree.h>

#include <atPool.h>
#include <Pool.h>

using TCreateCloneObjFn = rage::netObject*(*)(uint16_t objectId, uint8_t, int, int);
using TPoolPtr = atPoolBase**;

static TCreateCloneObjFn createCloneFuncs[(int)NetObjEntityType::Max];
static TPoolPtr validatePools[(int)NetObjEntityType::Max];

struct CloneBlockReferences
{
	TPoolPtr pool;
	TCreateCloneObjFn createClone;
};

static bool MatchBytes(const uint8_t* address, const uint8_t* pattern, const char* mask)
{
	for (size_t i = 0; mask[i]; ++i)
	{
		if (mask[i] == 'x' && address[i] != pattern[i])
		{
			return false;
		}
	}

	return true;
}

static bool DiscoverCloneBlockReferences(char* location, CloneBlockReferences* references, size_t referenceCount)
{
	static constexpr uint8_t poolPattern[] = {
		0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x48, 0x20, 0x8B, 0x40, 0x10,
		0xC1, 0xE1, 0x02, 0xC1, 0xF9, 0x02,
		0x2B, 0xC1, 0x85, 0xC0
	};
	static constexpr char poolMask[] = "xxx????xxxxxxxxxxxxxxxx";
	static constexpr uint8_t callMarker[] = { 0x41, 0x0F, 0xB7, 0xCC, 0xE8 };
	static constexpr char callMask[] = "xxxxx";

	constexpr size_t searchSize = 0x1000;
	constexpr size_t maximumBlockSize = 0x100;
	size_t found = 0;

	for (size_t offset = 0; offset + sizeof(poolPattern) <= searchSize && found < referenceCount; ++offset)
	{
		auto block = reinterpret_cast<uint8_t*>(location + offset);

		if (!MatchBytes(block, poolPattern, poolMask))
		{
			continue;
		}

		uint8_t* call = nullptr;
		for (size_t blockOffset = sizeof(poolPattern);
			blockOffset + sizeof(callMarker) <= maximumBlockSize;
			++blockOffset)
		{
			if (MatchBytes(block + blockOffset, callMarker, callMask))
			{
				call = block + blockOffset + 4;
				break;
			}
		}

		if (!call)
		{
			return false;
		}

		references[found++] = {
			reinterpret_cast<TPoolPtr>(hook::get_address<void*>(block + 3)),
			reinterpret_cast<TCreateCloneObjFn>(hook::get_call(call))
		};

		offset = static_cast<size_t>((call + 5) - reinterpret_cast<uint8_t*>(location)) - 1;
	}

	return found == referenceCount;
}

namespace rage
{
	netObject* CreateCloneObject(NetObjEntityType type, uint16_t objectId, uint8_t a2, int a3, int a4)
	{
		auto pool = *validatePools[(int)type];

		if (pool->GetCountDirect() >= pool->GetSize())
		{
			return nullptr;
		}

		if (type == NetObjEntityType::Ped || type == NetObjEntityType::Player)
		{
			auto entityPool = rage::GetPoolBase("Peds");

			if (entityPool->GetCountDirect() >= (entityPool->GetSize() - 4))
			{
				return nullptr;
			}
		}

		return createCloneFuncs[(int)type](objectId, a2, a3, a4);
	}
}

static HookFunction hookFunction([]()
{
	if (xbr::IsGameBuildOrGreater<xbr::Build::Summer_2025>())
	{
		auto location = hook::get_pattern<char>("45 84 ED 0F 84 ? ? ? ? 83 FE ? 0F 8F");
		static constexpr NetObjEntityType blockOrder[] = {
			NetObjEntityType::Ped,
			NetObjEntityType::Object,
			NetObjEntityType::Heli,
			NetObjEntityType::Door,
			NetObjEntityType::Boat,
			NetObjEntityType::Bike,
			NetObjEntityType::Automobile,
			NetObjEntityType::Pickup,
			NetObjEntityType::Train,
			NetObjEntityType::Trailer,
			NetObjEntityType::Player,
			NetObjEntityType::Submarine,
			NetObjEntityType::Plane,
			NetObjEntityType::PickupPlacement
		};
		CloneBlockReferences references[std::size(blockOrder)]{};

		if (!DiscoverCloneBlockReferences(location, references, std::size(references)))
		{
			throw std::runtime_error("Failed to discover net object clone pool references");
		}

		for (size_t i = 0; i < std::size(blockOrder); ++i)
		{
			auto type = static_cast<int>(blockOrder[i]);
			validatePools[type] = references[i].pool;
			createCloneFuncs[type] = references[i].createClone;
		}
	}
	else
	{
		auto location = hook::get_pattern<char>("0F 8E 12 03 00 00 41 8A", 22);

		createCloneFuncs[(int)NetObjEntityType::Ped] = (TCreateCloneObjFn)hook::get_call(location);
		createCloneFuncs[(int)NetObjEntityType::Object] = (TCreateCloneObjFn)hook::get_call(location + 0x39);
		createCloneFuncs[(int)NetObjEntityType::Heli] = (TCreateCloneObjFn)hook::get_call(location + 0x72);
		createCloneFuncs[(int)NetObjEntityType::Door] = (TCreateCloneObjFn)hook::get_call(location + 0xAB);
		createCloneFuncs[(int)NetObjEntityType::Boat] = (TCreateCloneObjFn)hook::get_call(location + 0xE4);
		createCloneFuncs[(int)NetObjEntityType::Bike] = (TCreateCloneObjFn)hook::get_call(location + 0x11D);
		createCloneFuncs[(int)NetObjEntityType::Automobile] = (TCreateCloneObjFn)hook::get_call(location + 0x156);
		createCloneFuncs[(int)NetObjEntityType::Pickup] = (TCreateCloneObjFn)hook::get_call(location + 0x18F);
		createCloneFuncs[(int)NetObjEntityType::Train] = (TCreateCloneObjFn)hook::get_call(location + 0x1EF);
		createCloneFuncs[(int)NetObjEntityType::Trailer] = (TCreateCloneObjFn)hook::get_call(location + 0x228);
		createCloneFuncs[(int)NetObjEntityType::Player] = (TCreateCloneObjFn)hook::get_call(location + 0x261);
		createCloneFuncs[(int)NetObjEntityType::Submarine] = (TCreateCloneObjFn)hook::get_call(location + 0x296);
		createCloneFuncs[(int)NetObjEntityType::Plane] = (TCreateCloneObjFn)hook::get_call(location + 0x2C8);
		createCloneFuncs[(int)NetObjEntityType::PickupPlacement] = (TCreateCloneObjFn)hook::get_call(location + 0x2FA);

		validatePools[(int)NetObjEntityType::Ped] = (TPoolPtr)hook::get_address<void*>(location - 42);
		validatePools[(int)NetObjEntityType::Object] = (TPoolPtr)hook::get_address<void*>(location + 13);
		validatePools[(int)NetObjEntityType::Heli] = (TPoolPtr)hook::get_address<void*>(location + 70);
		validatePools[(int)NetObjEntityType::Door] = (TPoolPtr)hook::get_address<void*>(location + 127);
		validatePools[(int)NetObjEntityType::Boat] = (TPoolPtr)hook::get_address<void*>(location + 184);
		validatePools[(int)NetObjEntityType::Bike] = (TPoolPtr)hook::get_address<void*>(location + 241);
		validatePools[(int)NetObjEntityType::Automobile] = (TPoolPtr)hook::get_address<void*>(location + 298);
		validatePools[(int)NetObjEntityType::Pickup] = (TPoolPtr)hook::get_address<void*>(location + 355);
		validatePools[(int)NetObjEntityType::Train] = (TPoolPtr)hook::get_address<void*>(location + 453);
		validatePools[(int)NetObjEntityType::Trailer] = (TPoolPtr)hook::get_address<void*>(location + 508);
		validatePools[(int)NetObjEntityType::Player] = (TPoolPtr)hook::get_address<void*>(location + 565);
		validatePools[(int)NetObjEntityType::Submarine] = (TPoolPtr)hook::get_address<void*>(location + 622);
		validatePools[(int)NetObjEntityType::Plane] = (TPoolPtr)hook::get_address<void*>(location + 672);
		validatePools[(int)NetObjEntityType::PickupPlacement] = (TPoolPtr)hook::get_address<void*>(location + 722);
	}

	static_assert(offsetof(rage::CNetworkSyncDataULBase, isRemote) == 59, "offset 75");
	static_assert(offsetof(rage::CNetworkSyncDataULBase, creationAckedPlayers) == 96, "offset 112");
	//static_assert(offsetof(CNetGamePlayer, physicalPlayerIndex) == 45 + 8, "offset 45"); // #TODO2060: NOT ANYMORE! HAHA!
	//static_assert(offsetof(CNetGamePlayer, playerInfo) == 168 + 8, "offset 168");
});
