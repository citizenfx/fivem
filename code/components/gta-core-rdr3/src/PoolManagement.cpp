#include <StdInc.h>

#include <Pool.h>
#include <Hooking.h>
#include <MinHook.h>
#include <Error.h>

class RageHashList
{
public:
	template<int Size>
	RageHashList(const char* (&list)[Size])
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

static void* (*g_origPoolAllocate)(atPoolBase*, uint64_t);

static void* PoolAllocateWrap(atPoolBase* pool, uint64_t unk)
{
	void* value = g_origPoolAllocate(pool, unk);

	if (!value)
	{
		auto it = g_inversePools.find(pool);
		std::string poolName = "<<unknown pool>>";

		if (it != g_inversePools.end())
		{
			uint32_t poolHash = it->second;

			poolName = poolEntries.LookupHash(poolHash);
		}

		AddCrashometry("pool_error", "%s (%d)", poolName, pool->GetSize());

		std::string extraWarning = (poolName.find("0x") == std::string::npos)
			? fmt::sprintf(" (you need to raise %s PoolSize in common/data/gameconfig.xml)", poolName)
			: "";

		FatalErrorNoExcept("%s Pool Full, Size == %d%s", poolName, pool->GetSize(), extraWarning);
	}

	return value;
}

static hook::cdecl_stub<void(atPoolBase*)> poolRelease([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? BA 88 01 00 00 48 8B CB E8", 13));
});

namespace rage
{
	GTA_CORE_EXPORT void* PoolAllocate(atPoolBase* pool)
	{
		return PoolAllocateWrap(pool, 0);
	}

	GTA_CORE_EXPORT void PoolRelease(atPoolBase* pool)
	{
		return poolRelease(pool);
	}
}

static hook::cdecl_stub<void()> _loadStreamingFiles([]()
{
	return hook::get_pattern("C7 85 78 02 00 00 61 00 00 00 41 BE", -0x28);
});

void (*g_origLevelLoad)(const char* r);

void WrapLevelLoad(const char* r)
{
	_loadStreamingFiles();

	g_origLevelLoad(r);
}

static HookFunction hookFunction([]()
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

	auto registerUnhashedPools = [&](hook::pattern& patternMatch, int callOffset, int nameOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			char* name = hook::get_address<char*>(match.get<void*>(nameOffset));
			generateAndCallStub(match, callOffset, HashString(name), false);
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

	// find initial pools
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 8B D8 E8"), 51, 1);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 89 44 24 28 4C 8D 05 ? ? ? ? 44 8B CD"), 41, 1);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 89 44 24 28 4C 8D 05 ? ? ? ? 44 8B CE"), 45, 1);

	registerUnhashedPools(hook::pattern("48 8D 15 ? ? ? ? 33 C9 E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B8"), 0x45, 0x3);
	registerAssetPools(hook::pattern("48 8D 15 ? ? ? ? C6 40 E8 ? 41 B9 ? ? ? ? C6"), 0x23, 0x3);

	// no-op assertation to ensure our pool crash reporting is used instead
	hook::nop(hook::get_pattern("83 C9 FF BA EF 4F 91 02 E8", 8), 5);

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("4C 63 41 1C 4C 8B D1 49 3B D0 76", -4), PoolAllocateWrap, (void**)&g_origPoolAllocate);
	MH_CreateHook(hook::get_pattern("8B 41 28 A9 00 00 00 C0 74", -15), PoolDtorWrap, (void**)&g_origPoolDtor);
	MH_EnableHook(MH_ALL_HOOKS);

	// raw sfe reg from non-startup
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 8B D8 48 85 C0 75 26 8D 50 5C", -0x38), WrapLevelLoad, (void**)&g_origLevelLoad);
	MH_EnableHook(MH_ALL_HOOKS);
});
