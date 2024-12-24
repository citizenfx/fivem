#include <StdInc.h>

#include <atArray.h>
#include "CrossBuildRuntime.h"
#include <Hooking.h>
#include <net/NetObjEntityType.h>
#include "RageHashList.h"

#include <string>
#include <unordered_map>

#include <CustomRtti.h>


namespace {

static std::unordered_map<void*, std::string> g_vtableToTypeNameMapping;
static uint8_t g_syncTreeDataNodesOffset;

static hook::cdecl_stub<void*(void*, uint16_t)> getSyncTreeForType([]()
{
	return hook::get_pattern("0F B7 CA 83 F9 ? 7F");
});


uint32_t ReadHashUnsafe(void* vtableEntryPtr)
{
	void* functionPtr = *(void**)vtableEntryPtr;
	uint8_t* funcByteCode = (uint8_t*)functionPtr;

	// Corresponds to "mov eax, cs:dword_.* ; retn" assembly.
	if (funcByteCode[0] == 0x8B && funcByteCode[1] == 0x05 && funcByteCode[6] == 0xC3)
	{
		uint32_t* hashAddress = hook::get_address<uint32_t*>(functionPtr, 2, 6);
		return *hashAddress;
	}
	return 0;
}

uint32_t TryReadHash(void* vtableEntryPtr)
{
	// We are trying to read arbitrary memory address. So access violation may occur.
	__try
	{
		return ReadHashUnsafe(vtableEntryPtr);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return 0;
}

std::string GetNameFromHash(void* ptr, bool debugFormat)
{
	static const char* classNamesTable[] = {
#include "gta_vtables.h"
	};

	static RageHashList rttiHashList(classNamesTable);

	void* vtablePtr = *(void**)ptr;

	std::vector<int> potentialRttiFunctionOffsets{0x10, 0x18};
	for (auto offset: potentialRttiFunctionOffsets)
	{
		uint32_t hash = TryReadHash((void*)((uint64_t)vtablePtr + offset));
		if (hash != 0 && rttiHashList.ContainsHash(hash))
		{
			std::string typeName = rttiHashList.LookupHash(hash);
			if (debugFormat)
			{
				typeName = "class " + typeName;
			}
			return typeName;
		}
	}

	return "";
}

bool AddSyncTreeNodeInfo(std::unordered_map<void*, std::string>& mapping)
{
	if (getSyncTreeForType(nullptr, 0) == nullptr)
	{
		// SyncTrees are not initialized yet.
		return false;
	}

	for (int i = 0; i < (int)fx::sync::NetObjEntityType::Max; i++)
	{
		uintptr_t synTree = (uintptr_t)getSyncTreeForType(nullptr, (uint16_t)i);
		uintptr_t dataNodes = synTree + g_syncTreeDataNodesOffset;

		fx::sync::NetObjEntityType objType = (fx::sync::NetObjEntityType)i;
		// See result of dumpSyncTree for earlier game builds or SyncTrees_Five.h file to confirm this mapping.
		// It relies on the fact that nodes in m_DataNodes array are always allocated in the same order and don't change from version to version.
		switch ((fx::sync::NetObjEntityType)i)
		{
			case fx::sync::NetObjEntityType::Automobile:
			{
				// CAutomobileSyncTree and CTrailerSyncTree  are technically different SyncTrees, but they share the same vtable.
				// Use class name that emphasizes this.
				mapping[*(void**)synTree] = "CAutomobileTrailerSyncTree";
				mapping[**(void***)(dataNodes + 0 * 8)] = "CVehicleCreationDataNode";
				mapping[**(void***)(dataNodes + 1 * 8)] = "CAutomobileCreationDataNode";
				mapping[**(void***)(dataNodes + 2 * 8)] = "CGlobalFlagsDataNode";
				mapping[**(void***)(dataNodes + 3 * 8)] = "CDynamicEntityGameStateDataNode";
				mapping[**(void***)(dataNodes + 4 * 8)] = "CPhysicalGameStateDataNode";
				mapping[**(void***)(dataNodes + 5 * 8)] = "CVehicleGameStateDataNode";
				mapping[**(void***)(dataNodes + 6 * 8)] = "CEntityScriptGameStateDataNode";
				mapping[**(void***)(dataNodes + 7 * 8)] = "CPhysicalScriptGameStateDataNode";
				mapping[**(void***)(dataNodes + 8 * 8)] = "CVehicleScriptGameStateDataNode";
				mapping[**(void***)(dataNodes + 9 * 8)] = "CEntityScriptInfoDataNode";
				mapping[**(void***)(dataNodes + 10 * 8)] = "CPhysicalAttachDataNode";
				mapping[**(void***)(dataNodes + 11 * 8)] = "CVehicleAppearanceDataNode";
				mapping[**(void***)(dataNodes + 12 * 8)] = "CVehicleDamageStatusDataNode";
				mapping[**(void***)(dataNodes + 13 * 8)] = "CVehicleComponentReservationDataNode";
				mapping[**(void***)(dataNodes + 14 * 8)] = "CVehicleHealthDataNode";
				mapping[**(void***)(dataNodes + 15 * 8)] = "CVehicleTaskDataNode";
				mapping[**(void***)(dataNodes + 16 * 8)] = "CSectorDataNode";
				mapping[**(void***)(dataNodes + 17 * 8)] = "CSectorPositionDataNode";
				mapping[**(void***)(dataNodes + 18 * 8)] = "CEntityOrientationDataNode";
				mapping[**(void***)(dataNodes + 19 * 8)] = "CPhysicalVelocityDataNode";
				mapping[**(void***)(dataNodes + 20 * 8)] = "CVehicleAngVelocityDataNode";
				mapping[**(void***)(dataNodes + 21 * 8)] = "CVehicleSteeringDataNode";
				mapping[**(void***)(dataNodes + 22 * 8)] = "CVehicleControlDataNode";
				mapping[**(void***)(dataNodes + 23 * 8)] = "CVehicleGadgetDataNode";
				mapping[**(void***)(dataNodes + 24 * 8)] = "CMigrationDataNode";
				mapping[**(void***)(dataNodes + 25 * 8)] = "CPhysicalMigrationDataNode";
				mapping[**(void***)(dataNodes + 26 * 8)] = "CPhysicalScriptMigrationDataNode";
				mapping[**(void***)(dataNodes + 27 * 8)] = "CVehicleProximityMigrationDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Bike:
			{
				// Bike, Boat and Train are technically different SyncTrees, but they share the same vtable.
				// Use class name that emphasizes this.
				mapping[*(void**)synTree] = "CBikeBoatTrainSyncTree";
				mapping[**(void***)(dataNodes + 5 * 8)] = "CBikeGameStateDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Boat:
			{
				mapping[**(void***)(dataNodes + 5 * 8)] = "CBoatGameStateDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Door:
			{
				mapping[*(void**)synTree] = "CDoorSyncTree";
				mapping[**(void***)(dataNodes + 0 * 8)] = "CDoorCreationDataNode";
				mapping[**(void***)(dataNodes + 2 * 8)] = "CDoorScriptInfoDataNode";
				mapping[**(void***)(dataNodes + 3 * 8)] = "CDoorScriptGameStateDataNode";
				mapping[**(void***)(dataNodes + 4 * 8)] = "CDoorMovementDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Heli:
			{
				mapping[*(void**)synTree] = "CHeliSyncTree";
				mapping[**(void***)(dataNodes + 16 * 8)] = "CHeliHealthDataNode";
				mapping[**(void***)(dataNodes + 25 * 8)] = "CHeliControlDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Object:
			{
				mapping[*(void**)synTree] = "CObjectSyncTree";
				mapping[**(void***)(dataNodes + 0 * 8)] = "CObjectCreationDataNode";
				mapping[**(void***)(dataNodes + 4 * 8)] = "CObjectGameStateDataNode";
				mapping[**(void***)(dataNodes + 7 * 8)] = "CObjectScriptGameStateDataNode";
				mapping[**(void***)(dataNodes + 10 * 8)] = "CPhysicalHealthDataNode";
				// Same vtable as CObjectOrientationNode.
				mapping[**(void***)(dataNodes + 12 * 8)] = "CObjectSectorPosNode";
				mapping[**(void***)(dataNodes + 13 * 8)] = "CObjectOrientationDataNode";
				mapping[**(void***)(dataNodes + 15 * 8)] = "CPhysicalAngVelocityDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Ped:
			{
				mapping[*(void**)synTree] = "CPedSyncTree";
				mapping[**(void***)(dataNodes + 0 * 8)] = "CPedCreationDataNode";
				mapping[**(void***)(dataNodes + 1 * 8)] = "CPedScriptCreationDataNode";
				mapping[**(void***)(dataNodes + 5 * 8)] = "CPedGameStateDataNode";
				mapping[**(void***)(dataNodes + 6 * 8)] = "CPedComponentReservationDataNode";
				mapping[**(void***)(dataNodes + 9 * 8)] = "CPedScriptGameStateDataNode";
				mapping[**(void***)(dataNodes + 11 * 8)] = "CPedAttachDataNode";
				mapping[**(void***)(dataNodes + 12 * 8)] = "CPedHealthDataNode";
				mapping[**(void***)(dataNodes + 13 * 8)] = "CPedMovementGroupDataNode";
				mapping[**(void***)(dataNodes + 14 * 8)] = "CPedAIDataNode";
				mapping[**(void***)(dataNodes + 15 * 8)] = "CPedAppearanceDataNode";
				mapping[**(void***)(dataNodes + 16 * 8)] = "CPedOrientationDataNode";
				mapping[**(void***)(dataNodes + 17 * 8)] = "CPedMovementDataNode";
				mapping[**(void***)(dataNodes + 18 * 8)] = "CPedTaskTreeDataNode";
				mapping[**(void***)(dataNodes + 19 * 8)] = "CPedTaskSpecificDataNode";
				mapping[**(void***)(dataNodes + 28 * 8)] = "CPedSectorPosMapNode";
				mapping[**(void***)(dataNodes + 29 * 8)] = "CPedSectorPosNavMeshNode";
				mapping[**(void***)(dataNodes + 33 * 8)] = "CPedInventoryDataNode";
				mapping[**(void***)(dataNodes + 34 * 8)] = "CPedTaskSequenceDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Pickup:
			{
				mapping[*(void**)synTree] = "CPickupSyncTree";
				mapping[**(void***)(dataNodes + 0 * 8)] = "CPickupCreationDataNode";
				mapping[**(void***)(dataNodes + 3 * 8)] = "CPickupScriptGameStateNode";
				mapping[**(void***)(dataNodes + 11 * 8)] = "CPickupSectorPosNode";
				break;
			}
			case fx::sync::NetObjEntityType::PickupPlacement:
			{
				mapping[*(void**)synTree] = "CPickupPlacementSyncTree";
				mapping[**(void***)(dataNodes + 0 * 8)] = "CPickupPlacementCreationDataNode";
				mapping[**(void***)(dataNodes + 3 * 8)] = "CPickupPlacementStateDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Plane:
			{
				mapping[*(void**)synTree] = "CPlaneSyncTree";
				mapping[**(void***)(dataNodes + 15 * 8)] = "CPlaneGameStateDataNode";
				mapping[**(void***)(dataNodes + 24 * 8)] = "CPlaneControlDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Submarine:
			{
				mapping[*(void**)synTree] = "CSubmarineSyncTree";
				mapping[**(void***)(dataNodes + 5 * 8)] = "CSubmarineGameStateDataNode";
				mapping[**(void***)(dataNodes + 24 * 8)] = "CSubmarineControlDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Player:
			{
				mapping[*(void**)synTree] = "CPlayerSyncTree";
				mapping[**(void***)(dataNodes + 0 * 8)] = "CPlayerCreationDataNode";
				mapping[**(void***)(dataNodes + 7 * 8)] = "CPlayerGameStateDataNode";
				mapping[**(void***)(dataNodes + 12 * 8)] = "CPlayerAppearanceDataNode";
				mapping[**(void***)(dataNodes + 13 * 8)] = "CPlayerPedGroupDataNode";
				mapping[**(void***)(dataNodes + 14 * 8)] = "CPlayerAmbientModelStreamingNode";
				mapping[**(void***)(dataNodes + 15 * 8)] = "CPlayerGamerDataNode";
				mapping[**(void***)(dataNodes + 16 * 8)] = "CPlayerExtendedGameStateNode";
				mapping[**(void***)(dataNodes + 29 * 8)] = "CPlayerSectorPosNode";
				mapping[**(void***)(dataNodes + 30 * 8)] = "CPlayerCameraDataNode";
				mapping[**(void***)(dataNodes + 31 * 8)] = "CPlayerWantedAndLOSDataNode";
				break;
			}
			case fx::sync::NetObjEntityType::Trailer:
			{
				break;
			}
			case fx::sync::NetObjEntityType::Train:
			{
				mapping[**(void***)(dataNodes + 5 * 8)] = "CTrainGameStateDataNode";
				break;
			}
		}
	}

	return true;
}

std::string GetNameFromVtableMapping(void* ptr, bool debugFormat)
{
	static bool syncTreeNodeInfoInitialized = false;

	if (!syncTreeNodeInfoInitialized)
	{
		syncTreeNodeInfoInitialized = AddSyncTreeNodeInfo(g_vtableToTypeNameMapping);
	}
	
	void* vtablePtr = *(void**)ptr;
	if (g_vtableToTypeNameMapping.find(vtablePtr) != g_vtableToTypeNameMapping.end())
	{
		std::string typeName = g_vtableToTypeNameMapping[vtablePtr];
		if (debugFormat)
		{
			typeName = "class " + typeName;
		}
		return typeName;
	}

	return "";
}

std::string GetVtableAddress(void* ptr, bool debugFormat)
{
	if (debugFormat)
	{
		return fmt::sprintf("unknown (vtable %p)", (void*)hook::get_unadjusted(*(uint64_t**)ptr));
	}
	return fmt::sprintf("%016llx", hook::get_unadjusted(*(uint64_t*)ptr));
}

// Tricking typeid(..) to use RTTI. Used for GTA builds below 2802.
struct VirtualBase
{
	virtual ~VirtualBase() = 0;
};

} // namespace

std::string SearchTypeName(void* ptr, bool debugFormat)
{
#ifdef GTA_FIVE
	if (!xbr::IsGameBuildOrGreater<2802>())
	{
		try
		{
			std::string name = typeid(*(VirtualBase*)ptr).name();
			if (!debugFormat && name.find("class ", 0) == 0)
			{
				name = name.substr(6);
			}
			return name;
		}
		catch (std::__non_rtti_object&)
		{
			return GetVtableAddress(ptr, debugFormat);
		}
	}

	std::string typeName = GetNameFromVtableMapping(ptr, debugFormat);
	if (!typeName.empty())
	{
		return typeName;
	}

	typeName = GetNameFromHash(ptr, debugFormat);
	if (!typeName.empty())
	{
		return typeName;
	}

	// Failed to find type information. Fallback to returning vtable address.
	return GetVtableAddress(ptr, debugFormat);
#else
	// Custom RTTI search is only supported for GTA5. Return vtable address instead.
	return GetVtableAddress(ptr, debugFormat);
#endif
}

static HookFunction hookFunction([] ()
{
	g_syncTreeDataNodesOffset = *(uint8_t*)hook::get_pattern("48 8D 4B ? 33 D2 41 B8 ? ? ? ? E8 ? ? ? ? 48 8D 8B ? ? ? ? 33 D2 41 B8 ? ? ? ? E8 ? ? ? ? 48 8B C3", 3);

	void* CDynamicEntitySyncTreeBase = nullptr;
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		auto ctor = hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 8D BB ? ? ? ? 48 89 03 48 8B CF E8 ? ? ? ? 48 8D 05"));
		CDynamicEntitySyncTreeBase = hook::get_address<void*>(ctor, 14, 7);
	}
	else
	{
		CDynamicEntitySyncTreeBase = hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 8D 9F ? ? ? ? 48 89 07 48 8B CB E8 ? ? ? ? 33 C9"), 3, 7);
	}

	g_vtableToTypeNameMapping = {
		{
			hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 8B CF 48 89 07 8B 43"), 3, 7),
			"CGameScriptId"
		},
		{
			hook::get_address<void*>((intptr_t)hook::get_call(xbr::IsGameBuildOrGreater<3407>() ? hook::get_pattern("48 89 33 E8 ? ? ? ? 48 8D 8F ? ? ? ? E8 ? ? ? ? 48 8B 5C 24", 3) : hook::get_pattern("E8 ? ? ? ? 48 8D 9F ? ? ? ? 48 8B CB E8 ? ? ? ? 48 8B 74 2")) + 0x8B, 3, 7),
			"CGameScriptObjInfo"
		},
		{
			hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 51 ? 48 89 51 ? 48 89 01 48 89 51 ? 48 89 51"), 3, 7),
			"netSyncParentNode"
		},
		{
			hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 8D 4B ? 44 8D 42"), 3, 7),
			"netSyncDataNode"
		},
		{
			hook::get_address<void*>(xbr::IsGameBuildOrGreater<3407>() ? hook::get_pattern("48 8D 05 ? ? ? ? 48 8D BD") : hook::get_pattern("48 8D 05 ? ? ? ? 48 89 03 48 8D 9F ? ? ? ? 48 8B CB E8 ? ? ? ? 33 ED"), 3, 7),
			"CProjectBaseSyncParentNode"
		},
		{
			hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 8B D9 48 89 01 4C 89 59"), 3, 7),
			"netSyncTree"
		},
		{
			hook::get_address<void*>(xbr::IsGameBuildOrGreater<3407>() ? hook::get_pattern("48 8D 05 ? ? ? ? 89 9F ? ? ? ? 48 89 07 C6 87") : hook::get_pattern("48 8D 05 ? ? ? ? 89 9F ? ? ? ? 48 89 07 48 89 AF"), 3, 7),
			"CProjectSyncTree"
		},
		{
			hook::get_address<void*>(xbr::IsGameBuildOrGreater<3407>() ? hook::get_pattern("48 8D 05 ? ? ? ? 48 8D 9F ? ? ? ? 48 89 07 48 8B CB E8 ? ? ? ? 33 C9") :  hook::get_pattern("48 8D 05 ? ? ? ? 48 8D 9F ? ? ? ? 48 89 07 48 8B CB E8 ? ? ? ? 33 ED"), 3, 7),
			"CPhysicalSyncTreeBase"
		},
		{
			CDynamicEntitySyncTreeBase,
			"CDynamicEntitySyncTreeBase"
		},
		{
			hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 8D 9F ? ? ? ? 48 89 07 48 8B CB E8 ? ? ? ? 48 8D 35"), 3, 7),
			"CEntitySyncTreeBase"
		},
		{
			hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 8D 9F ? ? ? ? 48 8B CB 48 89 07"), 3, 7),
			"CProximityMigrateableSyncTreeBase"
		},
	};
});
