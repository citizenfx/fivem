#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.FlexStruct.h>

#include <netSyncTree.h>
#include <CrossBuildRuntime.h>
#include <netObject.h>
#include <MinHook.h>

#include <EntitySystem.h>

#include <CoreConsole.h>

enum eVehicleType : uint32_t
{
	VEHICLE_TYPE_CAR = 0,
	VEHICLE_TYPE_PLANE,
	VEHICLE_TYPE_TRAILER,
	VEHICLE_TYPE_QUADBIKE,
	VEHICLE_TYPE_DRAFT,
	VEHICLE_TYPE_SUBMARINECAR,
	VEHICLE_TYPE_AMPHIBIOUS_AUTOMOBILE,
	VEHICLE_TYPE_AMPHIBIOUS_QUADBIKE,
	VEHICLE_TYPE_HELI,
	VEHICLE_TYPE_BLIMP,
	VEHICLE_TYPE_AUTOGYRO,
	VEHICLE_TYPE_BIKE,
	VEHICLE_TYPE_BICYCLE,
	VEHICLE_TYPE_BOAT,
	VEHICLE_TYPE_TRAIN,
	VEHICLE_TYPE_SUBMARINE,
};

struct bitField
{
	uint8_t m_type : 5;
	uint8_t m_bHasPreRenderEffects : 1;
	uint8_t m_bHasDrawableProxyForWaterReflections : 1;
	uint8_t m_tbdGeneralFlag : 1;
};

static int32_t g_vehicleDataNodeModelHashOffset = 0;
static int32_t g_playerDataNodeModelHashOffset = 0;
static int32_t g_pedDataNodeModelHashOffset = 0;
static int32_t g_modelInfoModelTypeOffset = 0;
static int32_t g_netObjVehicleEntityOffset = 0;
constexpr uint8_t kModelTypeVehicle = 5;
constexpr uint8_t kModelTypePed = 6;
constexpr uint32_t kModelHeliFallback = 0x2C75F0DD; // Buzzard2
constexpr uint32_t kModelBoatFallback = 0x107F392C; // Dinghy2
constexpr uint32_t kModelTrainFallback = 0x3D6AAA9B; // Freight
constexpr uint32_t kModelBikeFallback = 0x2EF89E46; // Sanchez
constexpr uint32_t kModelSubmarineFallback = 0x2DFF622F; // Submersible
constexpr uint32_t kModelTrailerFallback = 0xD46F4737; // Tanker
constexpr uint32_t kModelPlaneFallback = 0x15F27762; // Cargoplane
constexpr uint32_t kModelPedFallback = 0x705E61F2; // MP_M_Freemode_01

static hook::cdecl_stub<rage::netSyncTree*(void*, int)> getSyncTreeForType([]()
{
	return hook::get_pattern("0F B7 CA 83 F9 07 7F 5E");
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, int flags, int flags2, rage::datBitBuffer* buffer, void* netLogStub)> netSyncTree_ReadFromBuffer([]()
{
	return xbr::IsGameBuildOrGreater<2802>() ? hook::get_pattern("44 89 40 18 57 48 83 EC 30 44 8B 05 ? ? ? ? 65", -15) : hook::get_pattern("45 89 43 18 57 48 83 EC 30 48 83 79 10 00 49", -15);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_CanApplyToObject([]()
{
	return xbr::IsGameBuildOrGreater<2802>() ? hook::get_pattern("49 8B 06 49 8B CE FF 90 A0 00 00 00 84 C0", -0x29) : hook::get_pattern("49 8B CE FF 50 70 84 C0 74 31 33 FF", -0x2C);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_PrepareObject([]()
{
	return xbr::IsGameBuildOrGreater<2545>() ? hook::get_pattern("48 85 D2 74 45 48 8B 02 48 8B CA FF 50", -0x21) : hook::get_pattern("48 85 D2 74 4E 48 8B 02 48 8B CA", -0x21);
});

namespace rage
{
	bool netSyncTree::CanApplyToObject(netObject * object)
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

// CVehicleCreationDataNode::CanApply()
// Basically the above rage::netSyncTree::CanApplyToObject()
// will call each of its this->SyncTrees and call this virtual function that we are hooking
typedef hook::FlexStruct* (*GetArchetypeForHashFn)(uint32_t, int*);
static GetArchetypeForHashFn g_getArcheTypeForHash;

// basically a reconstruction of IS_THIS_MODEL_A_XXXX
static int32_t GetModelType(uint32_t hash)
{
	int flags = 0xFFFF;
	hook::FlexStruct* archetype = g_getArcheTypeForHash(hash, &flags);
	if (archetype)
	{
		bitField pType = archetype->Get<bitField>(g_modelInfoModelTypeOffset);
		if (pType.m_type == kModelTypeVehicle)
		{
			int32_t vehicleType = *(int32_t*)(((uint8_t*)archetype) + 0x340);
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
		bitField pType = archetype->Get<bitField>(g_modelInfoModelTypeOffset);
		if (pType.m_type == kModelTypePed)
		{
			return true;
		}
	}
	return false;
}

static bool (*g_origVehicleCreationDataNode__CanApply)(void*, rage::netObject*);

static bool CVehicleCreationDataNode__CanApply(hook::FlexStruct* thisptr, rage::netObject* netObj /*Hidden argument*/)
{
	uint32_t& hash = thisptr->At<uint32_t>(g_vehicleDataNodeModelHashOffset);

	int32_t vehicleModelType = GetModelType(hash);
	if (vehicleModelType < 0)
	{
		trace("CNetObjVehicle_SetVehicleCreateData: model hash %x is not a vehicle, skipping.\n", hash);
		return false;
	}

	// Various Crashfixes for de-sync between netobj type and model hash
	if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Heli)
	{
		if (vehicleModelType != VEHICLE_TYPE_HELI && vehicleModelType != VEHICLE_TYPE_BLIMP)
		{
			// Force hash to a Buzzard2, or it will cause crashes down the line(ropemanager, damagestatus, etc.)
			// better than returning false (makes them invisible)
			hash = kModelHeliFallback;
		}
	}
	if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Boat)
	{
		if (vehicleModelType != VEHICLE_TYPE_BOAT)
		{
			// force to dinghy2
			hash = kModelBoatFallback;
		}
	}
	// There is some usage of the older CREATE_AUTOMOBILE native which triggers this
	// if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Automobile)
	//{
	//	if (vehicleModelType != VEHICLE_TYPE_CAR
	//		&& vehicleModelType != VEHICLE_TYPE_SUBMARINECAR
	//		&& vehicleModelType != VEHICLE_TYPE_QUADBIKE
	//		&& vehicleModelType != VEHICLE_TYPE_AMPHIBIOUS_AUTOMOBILE
	//		&& vehicleModelType != VEHICLE_TYPE_AMPHIBIOUS_QUADBIKE)
	//	{
	//		// force to stromberg
	//		hash = 0x34DBA661;
	//	}
	//}
	if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Train)
	{
		if (vehicleModelType != VEHICLE_TYPE_TRAIN)
		{
			// freight
			hash = kModelTrainFallback;
		}
	}
	if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Bike)
	{
		if (vehicleModelType != VEHICLE_TYPE_BICYCLE && vehicleModelType != VEHICLE_TYPE_BIKE)
		{
			// sanchez
			hash = kModelBikeFallback;
		}
	}
	if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Submarine)
	{
		if (vehicleModelType != VEHICLE_TYPE_SUBMARINE)
		{
			// submersible
			hash = kModelSubmarineFallback;
		}
	}
	if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Trailer)
	{
		if (vehicleModelType != VEHICLE_TYPE_TRAILER)
		{
			// tanker
			hash = kModelTrailerFallback;
		}
	}
	if (netObj->GetObjectType() == (uint16_t)NetObjEntityType::Plane)
	{
		if (vehicleModelType != VEHICLE_TYPE_PLANE)
		{
			// cargoplane
			hash = kModelPlaneFallback;
		}
	}
	return g_origVehicleCreationDataNode__CanApply(thisptr, netObj);
}

static bool (*g_origPlayerCreationDataNode__CanApply)(void*, rage::netObject*);

static bool CPlayerCreationDataNode__CanApply(hook::FlexStruct* thisptr, rage::netObject* netObj /*Hidden argument*/)
{
	if (netObj->GetObjectType() != (uint16_t)NetObjEntityType::Player)
	{
		return false;
	}

	uint32_t& hash = thisptr->At<uint32_t>(g_playerDataNodeModelHashOffset);

	if (!IsModelAPed(hash))
	{
		hash = kModelPedFallback;
	}

	return g_origPlayerCreationDataNode__CanApply(thisptr, netObj);
}

static bool (*g_origPedCreationDataNode__CanApply)(void*, rage::netObject*);

static bool CPedCreationDataNode__CanApply(hook::FlexStruct* thisptr, rage::netObject* netObj /*Hidden argument*/)
{
	if (netObj->GetObjectType() != (uint16_t)NetObjEntityType::Ped)
	{
		return false;
	}

	uint32_t& hash = thisptr->At<uint32_t>(g_pedDataNodeModelHashOffset);

	if (!IsModelAPed(hash))
	{
		hash = kModelPedFallback;
	}

	return g_origPedCreationDataNode__CanApply(thisptr, netObj);
}

class CNetObjVehicle : public rage::netObject
{
};

class CVehicleControlDataNode
{
public:
	inline static ptrdiff_t kIsSubmarineOffset;

	inline bool IsSubmarine()
	{
		auto subLoc = reinterpret_cast<char*>(this) + kIsSubmarineOffset;
		return *reinterpret_cast<bool*>(subLoc);
	}

	inline void SetSubmarine(bool isSubmarine)
	{
		auto subLoc = reinterpret_cast<char*>(this) + kIsSubmarineOffset;
		*reinterpret_cast<bool*>(subLoc) = isSubmarine;
	}
};

class IVehicleNodeDataAccessor
{
public:
	inline static ptrdiff_t kBaseOffset;
	inline static ptrdiff_t kVehicleTypeOffset;

	inline CNetObjVehicle* GetBaseObject()
	{
		auto baseLoc = reinterpret_cast<char*>(this) - kBaseOffset;
		return reinterpret_cast<CNetObjVehicle*>(baseLoc);
	}

	inline eVehicleType GetVehicleType()
	{
		if (auto netObj = GetBaseObject())
		{
			if (auto vehicle = static_cast<char*>(netObj->GetGameObject()))
			{
				return *reinterpret_cast<eVehicleType*>(vehicle + kVehicleTypeOffset);
			}
		}

		return VEHICLE_TYPE_CAR; // Unexpected path.
	}
};

static void (*g_CNetObjVehicle_SetVehicleControlData)(IVehicleNodeDataAccessor*, CVehicleControlDataNode*);
static void CNetObjVehicle_SetVehicleControlData(IVehicleNodeDataAccessor* netObj, CVehicleControlDataNode* dataNode)
{
	if (dataNode->IsSubmarine())
	{
		if (netObj->GetVehicleType() != VEHICLE_TYPE_SUBMARINE)
		{
			dataNode->SetSubmarine(false);
		}
	}

	g_CNetObjVehicle_SetVehicleControlData(netObj, dataNode);
}

static HookFunction hookinit([]()
{
	MH_Initialize();

	// sig intended to break if offsets change (haven't so far)
	char* location = hook::get_pattern<char>("83 B9 40 03 00 00 08 0F B6");
	location -= 25;
	g_getArcheTypeForHash = hook::get_address<GetArchetypeForHashFn>(location, 1, 5);
	MH_CreateHook(hook::get_pattern("48 89 5C 24 10 55 48 8B EC 48 83 EC 20 8B 45 10 8B 89 C8 00 00 00"), CVehicleCreationDataNode__CanApply, (void**)&g_origVehicleCreationDataNode__CanApply);
	MH_CreateHook(hook::get_pattern("48 89 5C 24 10 55 48 8B EC 48 83 EC 20 8B 45 10 8B 89 C0 00 00 00"), CPlayerCreationDataNode__CanApply, (void**)&g_origPlayerCreationDataNode__CanApply);
	MH_CreateHook(hook::get_pattern("48 89 5C 24 10 55 48 8B EC 48 83 EC 20 8B 45 10 8B 89 C4 00 00 00"), CPedCreationDataNode__CanApply, (void**)&g_origPedCreationDataNode__CanApply);

	{
		// Offset from CNetObjVehicle to IVehicleNodeDataAccessor
		IVehicleNodeDataAccessor::kBaseOffset = *hook::get_pattern<int32_t>("48 89 83 ? ? ? ? 48 89 B3 ? ? ? ? 80 A3", 3);

		// Stolen from "VehicleExtraNatives.cpp" and "PatchVehicleHoodCamera.cpp"
		IVehicleNodeDataAccessor::kVehicleTypeOffset = *hook::get_pattern<int32_t>("41 83 BF ? ? ? ? 0B 74", 3);

		// CNetObjVehicle::SetVehicleControlData has changed quite a bit across builds.
		auto pattern = hook::pattern("80 ? ? ? ? ? ? 74 26 ? 8B 06 ? 8B CE").count(1).get(0);
		ptrdiff_t offset = xbr::IsGameBuildOrGreater<3095>() ? -0xD1 :
						   xbr::IsGameBuildOrGreater<2372>() ? -0xC5 :
						   xbr::IsGameBuildOrGreater<2060>() ? -0xCE : -0x73;

		CVehicleControlDataNode::kIsSubmarineOffset = *pattern.get<int32_t>(2);
		MH_CreateHook(pattern.get<void>(offset), CNetObjVehicle_SetVehicleControlData, (void**)&g_CNetObjVehicle_SetVehicleControlData);

#ifdef _DEBUG
		assert(IVehicleNodeDataAccessor::kBaseOffset == 512);
		assert(CVehicleControlDataNode::kIsSubmarineOffset == 281);
#endif
	}

	MH_EnableHook(MH_ALL_HOOKS);

	g_modelInfoModelTypeOffset = *hook::get_pattern<int32_t>("8A 86 ? ? ? ? 24 ? 3C ? 75 ? 48 8D 4D", 2);
	g_vehicleDataNodeModelHashOffset = *hook::get_pattern<int32_t>("8B 8E ? ? ? ? 0B C7", 2);
	g_playerDataNodeModelHashOffset = *hook::get_pattern<int32_t>("41 8B 8D ? ? ? ? BE", 3);
	g_pedDataNodeModelHashOffset = *hook::get_pattern<int32_t>("8B 8F ? ? ? ? 41 BC ? ? ? ? BB", 2);
});
