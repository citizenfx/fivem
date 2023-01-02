#include <StdInc.h>
#include <Pool.h>

#include <jitasm.h>
#include <Hooking.h>

#include <MinHook.h>

#include <Error.h>

class RageHashList
{
public:
	template<int Size>
	RageHashList(const char*(&list)[Size])
	{
		for (int i = 0; i < Size; i++)
		{
			m_lookupList.insert({ HashString(list[i]), list[i] });
		}
	}

	inline std::string LookupHash(uint32_t hash)
	{
		auto it = m_lookupList.find(hash);

		if (it != m_lookupList.end())
		{
			return std::string(it->second);
		}

		return fmt::sprintf("0x%08x", hash);
	}

private:
	std::map<uint32_t, std::string_view> m_lookupList;
};

static std::map<uint32_t, atPoolBase*> g_pools;
static std::map<atPoolBase*, uint32_t> g_inversePools;

static const char* poolEntriesTable[] = {
	"AnimatedBuilding",
	"AttachmentExtension",
	"AudioHeap",
	"BlendshapeStore",
	"Building",
	"carrec",
	"CBoatChaseDirector",
	"CVehicleCombatAvoidanceArea",
	"CCargen",
	"CCargenForScenarios",
	"CCombatDirector",
	"CCombatInfo",
	"CCombatSituation",
	"CCoverFinder",
	"CDefaultCrimeInfo",
	"CTacticalAnalysis",
	"CTaskUseScenarioEntityExtension",
	"AnimStore",
	"CGameScriptResource",
	"ClothStore",
	"CombatMeleeManager_Groups",
	"CombatMountedManager_Attacks",
	"CompEntity",
	"CPrioritizedClipSetBucket",
	"CPrioritizedClipSetRequest",
	"CRoadBlock",
	"CStuntJump",
	"CScenarioInfo",
	"CScenarioPointExtraData",
	"CutsceneStore",
	"CScriptEntityExtension",
	"CVehicleChaseDirector",
	"CVehicleClipRequestHelper",
	"CPathNodeRouteSearchHelper",
	"CGrabHelper",
	"CGpsNumNodesStored",
	"CClimbHandHoldDetected",
	"CAmbientLookAt",
	"DecoratorExtension",
	"DrawableStore",
	"Dummy Object",
	"DwdStore",
	"EntityBatch",
	"GrassBatch",
	"ExprDictStore",
	"FrameFilterStore",
	"FragmentStore",
	"GamePlayerBroadcastDataHandler_Remote",
	"InstanceBuffer",
	"InteriorInst",
	"InteriorProxy",
	"IplStore",
	"MaxLoadedInfo",
	"MaxLoadRequestedInfo",
	"ActiveLoadedInfo",
	"ActivePersistentLoadedInfo",
	"Known Refs",
	"LightEntity",
	"MapDataLoadedNode",
	"MapDataStore",
	"MapTypesStore",
	"MetaDataStore",
	"NavMeshes",
	"NetworkDefStore",
	"NetworkCrewDataMgr",
	"Object",
	"OcclusionInteriorInfo",
	"OcclusionPathNode",
	"OcclusionPortalEntity",
	"OcclusionPortalInfo",
	"Peds",
	"CWeapon",
	"phInstGta",
	"PhysicsBounds",
	"CPickup",
	"CPickupPlacement",
	"CPickupPlacementCustomScriptData",
	"CRegenerationInfo",
	"PortalInst",
	"PoseMatcherStore",
	"PMStore",
	"PtFxSortedEntity",
	"PtFxAssetStore",
	"QuadTreeNodes",
	"ScaleformStore",
	"ScaleformMgrArray",
	"ScriptStore",
	"StaticBounds",
	"tcBox",
	"TrafficLightInfos",
	"TxdStore",
	"Vehicles",
	"VehicleStreamRequest",
	"VehicleStreamRender",
	"VehicleStruct",
	"HandlingData",
	"wptrec",
	"fwLodNode",
	"CTask",
	"CEvent",
	"CMoveObject",
	"CMoveAnimatedBuilding",
	"atDScriptObjectNode",
	"fwDynamicArchetypeComponent",
	"fwDynamicEntityComponent",
	"fwEntityContainer",
	"fwMatrixTransform",
	"fwQuaternionTransform",
	"fwSimpleTransform",
	"ScenarioCarGensPerRegion",
	"ScenarioPointsAndEdgesPerRegion",
	"ScenarioPoint",
	"ScenarioPointEntity",
	"ScenarioPointWorld",
	"MaxNonRegionScenarioPointSpatialObjects",
	"ObjectIntelligence",
	"VehicleScenarioAttractors",
	"AircraftFlames",
	"CScenarioPointChainUseInfo",
	"CScenarioClusterSpawnedTrackingData",
	"CSPClusterFSMWrapper",
	"fwArchetypePooledMap",
	"CTaskConversationHelper",
	"SyncedScenes",
	"AnimScenes",
	"CPropManagementHelper",
	"ActionTable_Definitions",
	"ActionTable_Results",
	"ActionTable_Impulses",
	"ActionTable_Interrelations",
	"ActionTable_Homings",
	"ActionTable_Damages",
	"ActionTable_StrikeBones",
	"ActionTable_Rumbles",
	"ActionTable_Branches",
	"ActionTable_StealthKills",
	"ActionTable_Vfx",
	"ActionTable_FacialAnimSets",
	"NetworkEntityAreas",
	"NavMeshRoute",
	"CScriptEntityExtension",
	"AnimStore",
	"CutSceneStore",
	"OcclusionPathNode",
	"OcclusionPortalInfo",
	"CTask",
	"OcclusionPathNode",
	"OcclusionPortalInfo",
#include "gta_vtables.h"
	"Decorator",
};

static RageHashList poolEntries(poolEntriesTable);

GTA_CORE_EXPORT atPoolBase* rage::GetPoolBase(uint32_t hash)
{
	auto it = g_pools.find(hash);

	if (it == g_pools.end())
	{
		return nullptr;
	}

	return it->second;
}

static atPoolBase* SetPoolFn(atPoolBase* pool, uint32_t hash)
{
	g_pools[hash] = pool;
	g_inversePools.insert({ pool, hash });

	return pool;
}

static void(*g_origPoolDtor)(atPoolBase*);

static void PoolDtorWrap(atPoolBase* pool)
{
	auto hashIt = g_inversePools.find(pool);

	if (hashIt != g_inversePools.end())
	{
		auto hash = hashIt->second;

		g_pools.erase(hash);
		g_inversePools.erase(pool);
	}

	return g_origPoolDtor(pool);
}

static void*(*g_origPoolAllocate)(atPoolBase*);

static void* PoolAllocateWrap(atPoolBase* pool)
{
	void* value = g_origPoolAllocate(pool);

	if (!value)
	{
		auto it = g_inversePools.find(pool);
		std::string poolName = "<<unknown pool>>";

		if (it != g_inversePools.end())
		{
			uint32_t poolHash = it->second;
			
			poolName = poolEntries.LookupHash(poolHash);
		}

		// non-fatal pools
		if (poolName == "AircraftFlames")
		{
			return nullptr;
		}

		AddCrashometry("pool_error", "%s (%d)", poolName, pool->GetSize());

		FatalErrorNoExcept("%s Pool Full, Size == %d", poolName, pool->GetSize());
	}

	return value;
}

static hook::cdecl_stub<void(atPoolBase*, void*)> poolRelease([]()
{
	return hook::get_call(hook::get_pattern("48 8B D3 E8 ? ? ? ? 0F 28 45 E0 66 0F 7F", 3));
});

namespace rage
{
	GTA_CORE_EXPORT void* PoolAllocate(atPoolBase* pool)
	{
		return PoolAllocateWrap(pool);
	}

	GTA_CORE_EXPORT void PoolRelease(atPoolBase* pool, void* entry)
	{
		return poolRelease(pool, entry);
	}
}

static void(*g_origLoadObjectsNow)(void*, bool);

#include <ICoreGameInit.h>

static void* volatile* resourcePlacement;

static void LoadObjectsNowWrap(void* streaming, bool a2)
{
	// await resource placement having initialized first (the game does this very asynchronously, but doesn't ever block on it)
	while (!*resourcePlacement)
	{
		YieldProcessor();
	}

	// usual loop
	uint64_t beginTime = GetTickCount64();

	ICoreGameInit* init = Instance<ICoreGameInit>::Get();

	init->SetData("gta-core-five:loadCaller", fmt::sprintf("%016x", (uintptr_t)_ReturnAddress()));
	init->SetData("gta-core-five:loadTime", fmt::sprintf("%d", beginTime));

	g_origLoadObjectsNow(streaming, a2);

	init->SetData("gta-core-five:loadCaller", "");

	uint64_t elapsedTime = (GetTickCount64() - beginTime);

	if (elapsedTime > 2000)
	{
		trace("Warning: LoadObjectsNow took %d msec (invoked from %016x)!\n", elapsedTime, (uintptr_t)_ReturnAddress());
		trace("---------------- DO FIX THE ABOVE ^\n");

		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			trace("---------------- IF YOU CAN NOT FIX IT AND THIS OCCURS DURING GAMEPLAY\n");
			trace("---------------- PLEASE CONTACT THE FIVEM DEVELOPERS ON https://forum.fivem.net/\n");
			trace("---------------- WITH THIS CITIZENFX.LOG FILE\n");
			trace("---------------- \n");
			trace("---------------- THIS BLOCKING LOAD _WILL_ CAUSE CLIENT GAME CRASHES\n");
		}
	}
}

static struct MhInit
{
	MhInit()
	{
		MH_Initialize();
	}
} mhInit;

bool(*g_origShouldWriteToPlayer)(void* a1, void* a2, int playerIdx, int a4);

static bool ShouldWriteToPlayerWrap(void* a1, void* a2, int playerIdx, int a4)
{
	if (playerIdx == 31)
	{
		//return true;
	}

	return g_origShouldWriteToPlayer(a1, a2, playerIdx, a4);
}

uint32_t(*g_origGetRelevantSectorPosPlayers)(void* a1, void* a2, uint8_t a3);

uint32_t GetRelevantSectorPosPlayersWrap(void* a1, void* a2, uint8_t a3)
{
	auto val = g_origGetRelevantSectorPosPlayers(a1, a2, a3);

	if (Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		if (val & (1 << 31))
		{
			val &= ~(1 << 31);
		}
		else
		{
			//val |= (1 << 31);
		}
	}

	return val;
}

void*(*g_origGetNetObjPosition)(void*, void*);

static void* GetNetObjPositionWrap(void* a1, void* a2)
{
	trace("getting netobj %llx position\n", (uintptr_t)a1);

	return g_origGetNetObjPosition(a1, a2);
}

static HookFunction hookFunction([] ()
{
	auto generateAndCallStub = [](hook::pattern_match match, int callOffset, uint32_t hash, bool isAssetStore)
	{
		struct : jitasm::Frontend
		{
			uint32_t hash;
			uint64_t origFn;
			bool isAssetStore;

			void InternalMain() override
			{
				sub(rsp, 0x38);

				mov(rax, qword_ptr[rsp + 0x38 + 0x28]);
				mov(qword_ptr[rsp + 0x20], rax);

				mov(rax, qword_ptr[rsp + 0x38 + 0x30]);
				mov(qword_ptr[rsp + 0x28], rax);

				mov(rax, origFn);
				call(rax);

				mov(rcx, rax);

				if(isAssetStore == true)
				 add(rcx, 0x38);

				mov(edx, hash);

				mov(rax, (uint64_t)&SetPoolFn);
				call(rax);

				add(rsp, 0x38);

				ret();
			}
		} *stub = new std::remove_pointer_t<decltype(stub)>();

		stub->hash = hash;
		stub->isAssetStore = isAssetStore;

		auto call = match.get<void>(callOffset);
		hook::set_call(&stub->origFn, call);
		hook::call(call, stub->GetCode());
	};
	
	auto registerPools = [&](hook::pattern& patternMatch, int callOffset, int hashOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			generateAndCallStub(match, callOffset, *match.get<uint32_t>(hashOffset), false);
		}
	};

	auto registerAssetPools = [&](hook::pattern& patternMatch, int callOffset, int nameOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			char* name = hook::get_address<char*>(match.get<void*>(nameOffset));
			generateAndCallStub(match, callOffset, HashString(name), true);
		}
	};

	// Find initial pools
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? ? ? 00 E8 ? ? ? ? 4C 8D 05"), 0x2C, 1);
	registerPools(hook::pattern("C6 BA ? ? ? ? E8 ? ? ? ? 4C 8D 05"), 0x27, 2);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? C6 ? ? ? 01 4C"), 0x2F, 1);
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? ? 00 00 E8 ? ? ? ? C6"), 0x35, 1);
	registerPools(hook::pattern("44 8B C0 BA ? ? ? ? E8 ? ? ? ? 4C 8D 05"), 0x25, 4);
	
	// fwAssetStores
	registerAssetPools(hook::pattern("48 8D 15 ? ? ? ? 45 8D 41 ? 48 8B ? C7"), 0x15, 3); 

	// min hook
	MH_CreateHook(hook::get_pattern("18 83 F9 FF 75 03 33 C0 C3 41", -6), PoolAllocateWrap, (void**)&g_origPoolAllocate);

	// pool dtor wrap
	MH_CreateHook(hook::get_pattern("7E 38 F7 41 20 00 00 00 C0 74 1B", -0xD), PoolDtorWrap, (void**)&g_origPoolDtor);

	// in a bit of a wrong place, but OK
	MH_CreateHook(hook::get_call(hook::get_pattern("0D ? ? ? ? B2 01 E8 ? ? ? ? B0 01 48", 7)), LoadObjectsNowWrap, (void**)&g_origLoadObjectsNow);

	// cloning stuff
	MH_CreateHook(hook::get_pattern("41 8B D9 41 8A E8 4C 8B F2 48 8B F9", -0x19), ShouldWriteToPlayerWrap, (void**)&g_origShouldWriteToPlayer);

	MH_CreateHook(hook::get_pattern("41 8A E8 48 8B DA 48 85 D2 0F", -0x1E), GetRelevantSectorPosPlayersWrap, (void**)&g_origGetRelevantSectorPosPlayers);

	//MH_CreateHook((void*)0x14159A8F0, AssignObjectIdWrap, (void**)&g_origAssignObjectId);

	//MH_CreateHook((void*)0x141068F3C, GetNetObjPositionWrap, (void**)&g_origGetNetObjPosition);
	
	// player -> ped
	//hook::jump(0x14106B3D0, 0x14106B3B0); // st
	//hook::jump(0x1410A1CDC, 0x1410A1CC0); // m108, st dependency
	//hook::nop(0x141056E6F, 5);

	MH_EnableHook(MH_ALL_HOOKS);

	// resource placement internal ptr
	{
		char* basePtr = hook::get_address<char*>(hook::get_pattern("48 89 85 30 02 00 00 48 8D 45 30 48 8D 0D", 14));
		resourcePlacement = (decltype(resourcePlacement))(basePtr + *hook::get_pattern<uint32_t>("7E 22 48 8B 8B ? ? ? ? 8B D7 48 69", 5));
	}
});
