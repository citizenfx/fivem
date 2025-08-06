#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <netSyncTree.h>
#include <netObject.h>

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

enum eVehicleType : uint32_t
{
	VEHICLE_TYPE_NONE = -1,
	VEHICLE_TYPE_CAR = 0,
	VEHICLE_TYPE_PLANE = 1,
	VEHICLE_TYPE_TRAILER = 2,
	VEHICLE_TYPE_ARTILLERYGUN = 3,
	VEHICLE_TYPE_QUADBIKE = 4,
	VEHICLE_TYPE_DRAFT = 5,
	VEHICLE_TYPE_SUBMARINECAR = 6,
	VEHICLE_TYPE_MOUNTEDWEAPON = 7,
	VEHICLE_TYPE_HELI = 8,
	VEHICLE_TYPE_BLIMP = 9,
	VEHICLE_TYPE_BALLOON = 10,
	VEHICLE_TYPE_AUTOGYRO = 11,
	VEHICLE_TYPE_BIKE = 12,
	VEHICLE_TYPE_BICYCLE = 13,
	VEHICLE_TYPE_BOAT = 14,
	VEHICLE_TYPE_CANOE = 15,
	VEHICLE_TYPE_ROWINGBOAT = 16,
	VEHICLE_TYPE_TRAIN = 17,
	VEHICLE_TYPE_TRAIN_ENGINE = 18,
	VEHICLE_TYPE_TRAINCART = 19,
	VEHICLE_TYPE_SUBMARINE = 20,
};

// RDR3 now uses a function to fetch node data for individual creationDataNode types.
typedef hook::FlexStruct* (*GetSyncNodeDataFn)(void*);
static GetSyncNodeDataFn g_getPlayerSyncNodeData;
static GetSyncNodeDataFn g_getVehicleSyncNodeData;
static GetSyncNodeDataFn g_getPedSyncNodeData;
static GetSyncNodeDataFn g_getAnimalSyncNodeData;

static uint8_t kModelTypePed = 6;
static uint8_t kModelTypeVehicle = 5;

static uint32_t kModelPedFallback = HashString("PLAYER_ZERO");
static uint32_t kModelBoatFallback = HashString("CANOE");
static uint32_t kModelTrainFallback = HashString("WINTERSTEAMER");
static uint32_t kModelAutomobileFallback = HashString("HOTAIRBALLOON01");
static uint32_t kModelDraftFallback = HashString("BUGGY01");
static uint32_t kModelTrailerFallback = HashString("BREACH_CANNON");
static uint32_t kModelAnimalFallback = HashString("A_C_BEAVER_01");
static uint32_t kModelHorseFallback = HashString("A_C_DONKEY_01");

static uint32_t g_vehicleDataNodeModelHashOffset = 0;
static uint32_t g_playerDataNodeModelHashOffset = 0;
static uint32_t g_pedDataNodeModelHashOffset = 0;
static uint32_t g_modelInfoVehicleTypeOffset = 0;
static uint32_t g_modelInfoModelTypeOffset = 0;

typedef hook::FlexStruct* (*GetArchetypeForHashFn)(uint32_t, int*);
static GetArchetypeForHashFn g_getArcheTypeForHash;

// basically a reconstruction of IS_THIS_MODEL_A_XXXX
static int32_t GetVehicleModelType(uint32_t hash)
{
	int flags = 0xFFFF;
	hook::FlexStruct* archetype = g_getArcheTypeForHash(hash, &flags);
	if (archetype)
	{
		uint32_t type = archetype->Get<uint32_t>(g_modelInfoModelTypeOffset);

		if ((type & 0x1f) == kModelTypeVehicle)
		{
			int32_t vehicleType = archetype->Get<int32_t>(g_modelInfoVehicleTypeOffset);
			return vehicleType;
		}

		return -2;
	}
	return -1;
}

static bool IsModelAPed(uint32_t hash)
{
	int flags = 0xFFFF;
	hook::FlexStruct* archetype = g_getArcheTypeForHash(hash, &flags);
	if (archetype)
	{
		uint32_t type = archetype->Get<uint32_t>(g_modelInfoModelTypeOffset);
		if ((type & 0x1f) == kModelTypePed)
		{
			return true;
		}
	}
	return false;
}

static bool IsModelAHorse(uint32_t hash)
{
	int flags = 0xFFFF;
	hook::FlexStruct* archetype = g_getArcheTypeForHash(hash, &flags);
	if (archetype)
	{
		uint32_t type = archetype->Get<uint32_t>(g_modelInfoModelTypeOffset);
		if ((type & 0x1f) == kModelTypePed)
		{
			return archetype->Get<uint32_t>(0x300) == 0xB905D604;
		}
	}
	return false;
}

static bool (*g_origVehicleCreationDataNode__CanApply)(hook::FlexStruct*, rage::netObject*);
static bool CVehicleCreationDataNode__CanApply(hook::FlexStruct* thisptr, rage::netObject* netObj /*Hidden argument*/)
{
	hook::FlexStruct* dataNode = g_getVehicleSyncNodeData(thisptr);
	if (!dataNode)
	{
		return false;
	}

	uint32_t& hash = dataNode->At<uint32_t>(g_vehicleDataNodeModelHashOffset);
	int32_t vehicleModelType = GetVehicleModelType(hash);

	if (vehicleModelType < 0)
	{
		trace("CNetObjVehicle_SetVehicleCreateData: model hash 0x%x is not a vehicle, skipping.\n", hash);
		return false;
	}

	uint16_t objType = netObj->GetObjectType();

	// Boat
	if (objType == (uint16_t)NetObjEntityType::Boat)
	{
		if (vehicleModelType != VEHICLE_TYPE_BOAT && vehicleModelType != VEHICLE_TYPE_CANOE && vehicleModelType != VEHICLE_TYPE_ROWINGBOAT)
		{
			// Force hash to canoe
			hash = kModelBoatFallback;
		}
	}

	if (objType == (uint16_t)NetObjEntityType::DraftVeh)
	{
		if (vehicleModelType != VEHICLE_TYPE_DRAFT)
		{
			// Force hash to buggy01
			hash = kModelDraftFallback;
		}
	}

	if (objType == (uint16_t)NetObjEntityType::Bike)
	{
		if (vehicleModelType != VEHICLE_TYPE_BIKE && vehicleModelType != VEHICLE_TYPE_BICYCLE)
		{
			// There isn't a default bike model to fallback to.
			trace("CVehicleCreationDataNode__CanApply: attempted to spawn bike but model hash 0x%x is not a bike, skipping.\n", hash);
			return false;
		}
	}

	if (objType == (uint16_t)NetObjEntityType::Submarine)
	{
		if (vehicleModelType != VEHICLE_TYPE_SUBMARINE)
		{
			// There isn't a default submarine model to fallback to.
			trace("CVehicleCreationDataNode__CanApply: attempted to spawn submarine but model hash 0x%x is not a submarine, skipping.\n", hash);
			return false;
		}
	}

	if (objType == (uint16_t)NetObjEntityType::Plane)
	{
		if (vehicleModelType != VEHICLE_TYPE_PLANE && vehicleModelType != VEHICLE_TYPE_BLIMP)
		{
			// There isn't a default plane model to fallback to.
			trace("CVehicleCreationDataNode__CanApply: attempted to spawn plane but model hash 0x%x is not a plane, skipping.\n", hash);
			return false;
		}
	}

	if (objType == (uint16_t)NetObjEntityType::Trailer)
	{
		if (vehicleModelType != VEHICLE_TYPE_TRAILER && vehicleModelType != VEHICLE_TYPE_ARTILLERYGUN)
		{
			// Force hash to breach cannon
			hash = kModelTrailerFallback;
		}
	}

	if (objType == (uint16_t)NetObjEntityType::Automobile)
	{
		if (vehicleModelType != VEHICLE_TYPE_BALLOON && vehicleModelType != VEHICLE_TYPE_MOUNTEDWEAPON && vehicleModelType != VEHICLE_TYPE_SUBMARINECAR && vehicleModelType != VEHICLE_TYPE_QUADBIKE && vehicleModelType != VEHICLE_TYPE_CAR)
		{
			// Force hash to hotairballoon01
			hash = kModelAutomobileFallback;
		}
	}

	if (objType == (uint16_t)NetObjEntityType::Train)
	{
		if (vehicleModelType != VEHICLE_TYPE_TRAIN && vehicleModelType != VEHICLE_TYPE_TRAIN_ENGINE && vehicleModelType != VEHICLE_TYPE_TRAINCART)
		{
			// Force hash to wintersteamer
			hash = kModelTrainFallback;
		}
	}

	return g_origVehicleCreationDataNode__CanApply(thisptr, netObj);
}

static bool (*g_origCPlayerCreationDataNode__CanApply)(hook::FlexStruct*, rage::netObject*);
static bool CPlayerCreationDataNode__CanApply(hook::FlexStruct* thisptr, rage::netObject* netObj /*Hidden argument*/)
{
	if (netObj->GetObjectType() != (uint16_t)NetObjEntityType::Player)
	{
		return false;
	}

	hook::FlexStruct* dataNode = g_getPlayerSyncNodeData(thisptr);
	if (!dataNode)
	{
		return false;
	}

	uint32_t& hash = dataNode->At<uint32_t>(g_playerDataNodeModelHashOffset);
	if (!IsModelAPed(hash))
	{
		hash = kModelPedFallback;
	}

	return g_origCPlayerCreationDataNode__CanApply(thisptr, netObj);
}

static bool (*g_origCPedCreationDataNode__CanApply)(hook::FlexStruct*, rage::netObject*);
static bool CPedCreationDataNode__CanApply(hook::FlexStruct* thisptr, rage::netObject* netObj /*Hidden argument*/)
{
	hook::FlexStruct* dataNode = g_getPedSyncNodeData(thisptr);
	if (!dataNode)
	{
		return false;
	}

	uint32_t& hash = dataNode->At<uint32_t>(g_pedDataNodeModelHashOffset);
	uint16_t objType = netObj->GetObjectType();

	if (objType != (uint16_t)NetObjEntityType::Ped)
	{
		return false;
	}

	if (!IsModelAPed(hash))
	{
		hash = kModelPedFallback;
	}

	return g_origCPedCreationDataNode__CanApply(thisptr, netObj);
}

static bool (*g_origCAnimalCreationDataNode__CanApply)(hook::FlexStruct*, rage::netObject*);
static bool CAnimalCreationDataNode__CanApply(hook::FlexStruct* thisptr, rage::netObject* netObj /*Hidden argument*/)
{
	hook::FlexStruct* dataNode = g_getPedSyncNodeData(thisptr);
	if (!dataNode)
	{
		return false;
	}

	uint32_t& hash = dataNode->At<uint32_t>(g_pedDataNodeModelHashOffset);
	uint16_t objType = netObj->GetObjectType();

	if (objType != (uint16_t)NetObjEntityType::Horse && objType != (uint16_t)NetObjEntityType::Animal)
	{
		return false;
	}

	// Horses
	if (objType == (uint16_t)NetObjEntityType::Horse && !IsModelAHorse(hash))
	{
		// Force hash to a_c_donkey_01
		hash = kModelHorseFallback;
	}

	// All other animals
	if (objType == (uint16_t)NetObjEntityType::Animal && !IsModelAPed(hash))
	{
		// Force hash to a_c_beaver_01
		hash = kModelAnimalFallback;
	}

	return g_origCAnimalCreationDataNode__CanApply(thisptr, netObj);
}

static HookFunction hookinit([]()
{
	g_getArcheTypeForHash = hook::get_address<GetArchetypeForHashFn>(hook::get_pattern("85 C9 75 ? 33 C0 EB ? 48 8D 54 24", 21), 1, 5);
	// CPlayerCreationDataNode
	{
		auto location = hook::get_pattern<char>("48 89 5C 24 ? 55 48 8B EC 48 83 EC ? E8 ? ? ? ? C7 45 ? ? ? ? ? 48 8D 55 ? C7 45 ? ? ? ? ? 8B 88");
		hook::set_call(&g_getPlayerSyncNodeData, location + 13);
		g_origCPlayerCreationDataNode__CanApply = hook::trampoline(location, CPlayerCreationDataNode__CanApply);
	}

	// CVehicleCreationDataNode
	{
		auto location = hook::get_pattern<char>("8B 48 ? E8 ? ? ? ? 0F B7 45 ? BB", -36);
		hook::set_call(&g_getVehicleSyncNodeData, location + 13);
		g_origVehicleCreationDataNode__CanApply = hook::trampoline(location, CVehicleCreationDataNode__CanApply);
	}

	// CPedCreationDataNode
	{
		auto location = hook::get_pattern<char>("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8D 54 24");
		hook::set_call(&g_getPedSyncNodeData, location + 0x12);
		g_origCPedCreationDataNode__CanApply = hook::trampoline(location, CPedCreationDataNode__CanApply);
	}

	// CAnimalCreationDataNode
	{
		auto location = hook::get_pattern<char>("48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8D 54 24 ? C7 44 24 ? ? ? ? ? C7 44 24 ? ? ? ? ? 8B 48");
		hook::set_call(&g_getAnimalSyncNodeData, location + 0xC);
		g_origCAnimalCreationDataNode__CanApply = hook::trampoline(location, CAnimalCreationDataNode__CanApply);
	}

	{
		g_modelInfoModelTypeOffset = *hook::get_pattern<uint32_t>("8A 80 ? ? ? ? 24 ? 3C ? 75 ? 83 B9", 2);
		g_modelInfoVehicleTypeOffset = *hook::get_pattern<uint32_t>("8B 81 ? ? ? ? BA ? ? ? ? 83 E8 ? 0F B6 DB 3B C2", 2);
		g_playerDataNodeModelHashOffset = *hook::get_pattern<uint32_t>("8B 88 ? ? ? ? E8 ? ? ? ? 0F B7 45 ? BB", 2);
		g_vehicleDataNodeModelHashOffset = *hook::get_pattern<uint8_t>("8B 48 ? E8 ? ? ? ? 0F B7 45 ? BB", 2);
		g_pedDataNodeModelHashOffset = *hook::get_pattern<uint8_t>("8B 48 ? E8 ? ? ? ? 0F B7 54 24 ? BB", 2);
	}
});
