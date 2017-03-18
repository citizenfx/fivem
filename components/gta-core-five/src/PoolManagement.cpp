#include <StdInc.h>
#include <Pool.h>

#include <Hooking.h>

#include <MinHook.h>

#include <Error.h>

class RageHashList
{
public:
	RageHashList(std::initializer_list<std::string> list)
	{
		for (auto& entry : list)
		{
			m_lookupList.insert({ HashString(entry.c_str()), entry });
		}
	}

	inline std::string LookupHash(uint32_t hash)
	{
		auto it = m_lookupList.find(hash);

		if (it != m_lookupList.end())
		{
			return it->second;
		}

		return fmt::sprintf("0x%08x", hash);
	}

private:
	std::map<uint32_t, std::string> m_lookupList;
};

static std::map<uint32_t, atPoolBase*> g_pools;
static std::map<atPoolBase*, uint32_t> g_inversePools;

static RageHashList poolEntries = {
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
};

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
	g_pools.insert({ hash, pool });
	g_inversePools.insert({ pool, hash });

	return pool;
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

		std::string extraWarning = (poolName.find("0x") == std::string::npos)
			? fmt::sprintf(" (you need to raise %s PoolSize in common/data/gameconfig.xml)", poolName)
			: "";

		FatalError("%s Pool Full, Size == %d%s", poolName, pool->GetSize(), extraWarning);
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

static void LoadObjectsNowWrap(void* streaming, bool a2)
{
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

static HookFunction hookFunction([] ()
{
	auto registerPools = [] (hook::pattern& patternMatch, int callOffset, int hashOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			auto hash = *match.get<uint32_t>(hashOffset);

			struct : jitasm::Frontend
			{
				uint32_t hash;
				uint64_t origFn;

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
					mov(edx, hash);

					mov(rax, (uint64_t)&SetPoolFn);
					call(rax);

					add(rsp, 0x38);

					ret();
				}
			}* stub = new std::remove_pointer_t<decltype(stub)>();

			stub->hash = hash;

			auto call = match.get<void>(callOffset);
			hook::set_call(&stub->origFn, call);
			hook::call(call, stub->GetCode());
		}
	};

	// find initial pools
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? ? ? 00 E8 ? ? ? ? 4C 8D 05"), 0x2C, 1);
	registerPools(hook::pattern("C6 BA ? ? ? ? E8 ? ? ? ? 4C 8D 05"), 0x27, 2);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? C6 ? ? ? 01 4C"), 0x2F, 1);
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? 00 00 00 E8 ? ? ? ? C6"), 0x35, 1);

	// min hook
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("18 83 F9 FF 75 03 33 C0 C3 41", -6), PoolAllocateWrap, (void**)&g_origPoolAllocate);

	// in a bit of a wrong place, but OK
	MH_CreateHook(hook::get_call(hook::get_pattern("0D ? ? ? ? B2 01 E8 ? ? ? ? B0 01 48", 7)), LoadObjectsNowWrap, (void**)&g_origLoadObjectsNow);

	MH_EnableHook(MH_ALL_HOOKS);
});