#include <StdInc.h>

#include <Pool.h>
#include <PoolSizesState.h>
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
	std::unordered_map<uint32_t, std::string_view> m_lookupList;
};

static std::unordered_map<uint32_t, atPoolBase*> g_pools;
static std::unordered_map<atPoolBase*, uint32_t> g_inversePools;
static std::unordered_map<std::string, atPoolBase*> g_namedPools;

static const char* poolEntriesTable[] = {
	"ActionTable_Branches",
	"ActionTable_Damages",
	"ActionTable_Definitions",
	"ActionTable_FacialAnimSets",
	"ActionTable_Homings",
	"ActionTable_Impulses",
	"ActionTable_Interrelations",
	"ActionTable_Results",
	"ActionTable_Rumbles",
	"ActionTable_StealthKills",
	"ActionTable_StrikeBones",
	"ActionTable_Vfx",
	"ActiveLoadedInfo",
	"ActivePersistentLoadedInfo",
	"AircraftFlames",
	"AnimatedBuilding",
	"AnimScenes",
	"AnimSceneStore",
	"AnimStore",
	"atDGameServerTransactionNode",
	"atDNetEventNode",
	"atDScriptObjectNode",
	"atDNetObjectNode",
	"AttachmentExtension",
	"audDynMixPatch",
	"audDynMixPatchSettings",
	"AudioHeap",
	"audScene",
	"AvoidanceVolumes",
	"BehaviorDataDictStore",
	"BlendshapeStore",
	"BodyDataDictStore",
	"Building",
	"CActionCache",
	"CActionConfigInfo",
	"CAIHandlingInfo",
	"CAimHelper",
	"CAimSolver::InternalState",
	"CAimSolver",
	"CAmbientFlockSpawnContainer",
	"CAmbientLookAt",
	"CAmbientMaskVolume",
	"CAmbientMaskVolumeDoor",
	"CAmbientMaskVolumeEntity",
	"CAmmoItem",
	"camStickyAimHelper",
	"CAMVolumeExtensionComponent",
	"CAnimalAttackGroup",
	"CAnimalAttractor",
	"CAnimalDispatch",
	"CAnimalFlock",
	"CAnimalGroup",
	"CAnimalGroupMember",
	"CAnimalTargeting",
	"CAnimalTuning",
	"CAnimalUnalertedGroup",
	"CAnimatedBuildingAnimationComponent",
	"CAnimationComponent",
	"CAnimDirectorComponentIk",
	"CAnimSceneHelper",
	"CArmIkSolver",
	"CArmPostureSolver",
	"CArmSolver",
	"carrec",
	"CarriableExtension",
	"CarryActionFinderFSM",
	"CAudioCollisionExtensionComponent",
	"CAudioEffectExtensionComponent",
	"CAudioEmitter",
	"CAvoidanceComponent",
	"CBalanceSolver",
	"CBarBrawler",
	"CBirdCurveContainer",
	"CBoatChaseDirector",
	"CBodyAimSolver::InternalState",
	"CBodyAimSolver",
	"CBodyDampingSolver",
	"CBodyLookIkSolver::InternalState",
	"CBodyLookIkSolver",
	"CBodyLookIkSolverProxy",
	"CBodyReachSolver",
	"CBodyRecoilIkSolver",
	"CBuoyancyExtensionComponent",
	"CCampItem",
	"CCargen",
	"CCargenForScenarios",
	"CCarryConfigTargetClipSetRequester",
	"CCharacterItem",
	"CChatHelper",
	"CClimbHandHoldDetected",
	"CClimbSolver",
	"CClothingItem",
	"CCoachInventoryItem",
	"CCombatDirector",
	"CCombatInfo",
	"CCombatMeleeGroup",
	"CCombatSituation",
	"CContainedObjectId",
	"CContainedObjectIdsInfo",
	"CContourSolver",
	"CContourSolverProxy",
	"CCoverFinder",
	"CCrimeObserver",
	"CCustomModelBoundsMappings::CMapping",
	"CDecalAttr",
	"CDecalExtensionComponent",
	"CDefaultCrimeInfo",
	"CDogItem",
	"CDoorExtension",
	"CDynamicEntityAnimationComponent",
	"CEmotionalLocoHelper",
	"CEntityGameInfoComponent",
	"CEvent",
	"CEventNetwork",
	"CEventUi",
	"CExplosionAttr",
	"CExplosionExtensionComponent",
	"CExpressionExtensionComponent",
	"CFakeDoorExtension::FakeDoorInfo",
	"CFakeDoorExtension",
	"CFakeDoorGroupExtensionComponent",
	"CFakeDoorInfo",
	"CFleeDecision",
	"CFlockTuning",
	"CFogVolumeExtensionComponent",
	"CFragObjectAnimExtensionComponent",
	"CFullBodySolver",
	"CGameOwnership",
	"CGameScriptHandler",
	"CGameScriptHandlerNetwork",
	"CGameScriptResource",
	"CGpsNumNodesStored",
	"CGrabHelper",
	"CGrappleClipRequestHelper",
	"CGroupScenario",
	"CGuidComponent",
	"CHandlingObject",
	"CHealthComponent",
	"CHighHeelSolver",
	"CHorseEquipmentInventoryItem",
	"CHorseInventoryItem",
	"CImpulseReactionSolver",
	"CInventoryItem",
	"CKinematicComponent",
	"CLadderInfo",
	"CLadderInfoExtensionComponent",
	"CLegIkSolver",
	"CLegIkSolverProxy",
	"CLegIkSolverState",
	"CLegPostureSolver",
	"CLightComponent",
	"CLightEntity",
	"CLightGroupExtensionComponent",
	"CLightShaftComponent",
	"CLightShaftExtensionComponent",
	"ClipStore",
	"clothManagerHeapSize",
	"ClothStore",
	"CMapDataReference",
	"CMapEntityRequest",
	"CMeleeClipRequestHelper",
	"CModelSetSpawner",
	"CMountedLegSolver",
	"CMountedLegSolverProxy",
	"CMoveAnimatedBuilding",
	"CMoveObject",
	"CNavObstructionPath",
	"CNetObjAnimScene",
	"CNetObjCombatDirector",
	"CNetObjDoor",
	"CNetObjDraftVehicle",
	"CNetObjGroupScenario",
	"CNetObjGuardZone",
	"CNetObjHorse"
	"CNetObjHerd",
	"CNetObjIncident",
	"CNetObjObject",
	"CNetObjPedBase",
	"CNetObjPedGroup",
	"CNetObjPedSharedTargeting",
	"CNetObjPersistent",
	"CNetObjPickupPlacement",
	"CNetObjPlayer",
	"CNetObjProjectile",
	"CNetObjPropSet",
	"CNetObjStatsTracker",
	"CNetObjVehicle",
	"CVehicleSyncData",
	"CNetObjWorldState",
	"CNetBlenderPed",
	"CNetBlenderPhysical",
	"CPedSyncData",
	"CNetworkTrainTrackJunctionSwitchWorldStateData",
	"CObjectAnimationComponent",
	"CObjectAutoStartAnimComponent",
	"CObjectAutoStartAnimExtensionComponent",
	"CObjectBreakableGlassComponent",
	"CObjectBuoyancyModeComponent",
	"CObjectCollisionDetectedComponent",
	"CObjectCollisionEffectsComponent",
	"CObjectDoorComponent",
	"CObjectDraftVehicleWheelComponent",
	"CObjectIntelligenceComponent",
	"CObjectLinksExtensionComponent",
	"CObjectNetworkComponent",
	"CObjectPhysicsComponent",
	"CObjectRiverProbeSubmissionComponent",
	"CObjectVehicleParentDeletedComponent",
	"CObjectWeaponsComponent",
	"CObjectSyncData",
	"reassignObjectInfo",
	"CombatMeleeManager_Groups",
	"CombatMountedManager_Attacks",
	"CompEntity",
	"CompositeLootableEntityDefInst",
	"CPairedAnimationReservationComponent",
	"CParticleAttr",
	"CParticleExtensionComponent",
	"CPathNodeRouteSearchHelper",
	"CPedAnimalAudioComponent",
	"CPedAnimalComponent",
	"CPedAnimalEarsComponent",
	"CPedAnimalTailComponent",
	"CPedAnimationComponent",
	"CPedAttributeComponent",
	"CPedAudioComponent",
	"CPedAvoidanceComponent",
	"CPedBreatheComponent",
	"CPedClothComponent",
	"CPedCoreComponent",
	"CPedCreatureComponent",
	"CPedDamageModifierComponent",
	"CPedDistractionComponent",
	"CPedDrivingComponent",
	"CPedDummyComponent",
	"CPedEquippedWeapon",
	"CPedEventComponent",
	"CPedFacialComponent",
	"CPedFootstepComponent",
	"CPedGameplayComponent",
	"CPedGraphicsComponent",
	"CPedHealthComponent",
	"CPedHorseComponent",
	"CPedHumanAudioComponent",
	"CPedIntelligenceComponent",
	"CPedInventory",
	"CPedInventoryComponent",
	"CPedLookAtComponent",
	"CPedMeleeModifierComponent",
	"CPedMotionComponent",
	"CPedMotivationComponent",
	"CPedPhysicsComponent",
	"CPedPlayerComponent",
	"CPedProjDecalComponent",
	"CPedRagdollComponent",
	"CPedScriptDataComponent",
	"CPedScriptedTaskRecordData",
	"CPedSharedTargeting",
	"CPedStaminaComponent",
	"CPedTargetingComponent",
	"CPedThreatResponseComponent",
	"CPedTransportComponent",
	"CPedTransportUserComponent",
	"CPedVfxComponent",
	"CPedVisibilityComponent",
	"CPedWeaponComponent",
	"CPedWeaponManagerComponent",
	"CPersCharGroup",
	"CPersCharHorse",
	"CPersCharVehicle",
	"CPersistentCharacter",
	"CPersistentCharacterGroupInfo",
	"CPersistentCharacterInfo",
	"CPhysicalPhysicsComponent",
	"CPhysicsComponent",
	"CPickup",
	"CPickupData",
	"CPickupPlacement",
	"CPickupPlacementCustomScriptData",
	"CPinMapDataExtension",
	"CPointGunHelper",
	"CPopZoneSpawner",
	"CPortableComponent",
	"CPoseFixupSolver",
	"CPrioritizedClipSetBucket",
	"CPrioritizedClipSetRequest",
	"CPrioritizedDictionaryRequest",
	"CPrioritizedSetRequest",
	"CProcObjAttr",
	"CProcObjectExtensionComponent",
	"CProjDecalComponent",
	"CPropInstanceHelper",
	"CPropManagementHelper",
	"CPropSetObjectExtension",
	"CQuadLegIkSolver",
	"CQuadLegIkSolverProxy",
	"CQuadrupedInclineSolver",
	"CQuadrupedInclineSolverProxy",
	"CQuadrupedReactSolver",
	"CQueriableTaskInfo",
	"crCreatureComponentCloth",
	"crCreatureComponentExternalDofs",
	"crCreatureComponentExtraDofs",
	"crCreatureComponentHistory",
	"crCreatureComponentParticleEffect",
	"crCreatureComponentPhysical",
	"crCreatureComponentShaderVars",
	"crCreatureComponentSkeleton",
	"CRegenerationInfo",
	"CRemoteScriptArgs",
	"CRemoteTaskData",
	"crExpressionPlayer",
	"crFrameAcceleratorEntryHeaders",
	"crFrameAcceleratorHeapSize",
	"crFrameAcceleratorMapSlots",
	"crikHeap",
	"crmtNodeFactoryPool",
	"CRoadBlock",
	"CRootSlopeFixupIkSolver",
	"crRelocatableAsyncCompactSize",
	"crRelocatableHeapSize",
	"crRelocatableMapSlots",
	"crWeightSetAcceleratorEntryHeaders",
	"crWeightSetAcceleratorHeapSize",
	"crWeightSetAcceleratorMapSlots",
	"CSatchelItem",
	"CScenarioClusterSpawnedTrackingData",
	"CScenarioInfo",
	"CScenarioPoint",
	"CScenarioPointChainUseInfo",
	"CScenarioPointExtraData",
	"CScenarioPropManager::ActiveSchedule",
	"CScenarioPropManager::LoadedPropInfo",
	"CScenarioPropManager::PendingPropInfo",
	"CScenarioPropManager::UprootedPropInfo",
	"CScenarioRequest",
	"CScenarioRequestHandler",
	"CScenarioRequestResults",
	"CScriptEntityExtension",
	"CScriptEntityIdExtension",
	"CSimulatedRouteManager::Route",
	"CSPClusterFSMWrapper",
	"CSquad",
	"CStairsExtension",
	"CStairsExtensionComponent",
	"CStatEvent",
	"CStirrupSolver",
	"CStuntHelper",
	"CStuntJump",
	"CSubscriberEntityComponent",
	"CSwayableAttr",
	"CSwayableExtensionComponent",
	"CTacticalAnalysis",
	"CTask",
	"CTaskConversationHelper",
	"CTaskNetworkComponent",
	"CTaskSequenceList",
	"CTaskUseScenarioEntityExtension",
	"CTerrainAdaptationHelper",
	"CThreatenedHelper",
	"CTimedCarryConfigTargetClipSetRequester",
	"CTorsoReactIkSolver",
	"CTorsoVehicleIkSolver",
	"CTorsoVehicleIkSolverProxy",
	"CTransportComponent",
	"CTwoBoneSolver",
	"CUnlock",
	"CUpperBodyBlend::CBodyBlendBoneCache",
	"CUpperBodyBlend",
	"CurveLib::Curve",
	"CustomShaderEffectBatchSlodType",
	"CustomShaderEffectBatchType",
	"CustomShaderEffectCommonType",
	"CustomShaderEffectGrassType",
	"CustomShaderEffectTreeType",
	"CutsceneStore",
	"CutSceneStore",
	"CVehicle",
	"CVehicleAnimationComponent",
	"CVehicleChaseDirector",
	"CVehicleClipRequestHelper",
	"CVehicleCombatAvoidanceArea",
	"CVehicleCoreComponent",
	"CVehicleDrivingComponent",
	"CVehicleIntelligenceComponent",
	"CVehiclePhysicsComponent",
	"CVehicleTurretSolver",
	"CVehicleWeaponsComponent",
	"CVolumeLocationExtension",
	"CVolumeOwnerExtension",
	"CWeapon",
	"CWeaponComponent",
	"CWeaponComponentInfo",
	"CWeaponComponentItem",
	"CWeaponItem",
	"CWildlifeSpawnRequest",
	"CWindDisturbanceExtensionComponent",
	"Decorator",
	"DecoratorExtension",
	"DrawableStore",
	"Dummy Object",
	"DwdStore",
	"Entity Alt request data",
	"EntityBatch",
	"EntityBatchBitset",
	"ExprDictStore",
	"FlocksPerPopulationZone",
	"fragCacheEntriesProps",
	"fragCacheHeadroom",
	"fragInstGta",
	"FragmentStore",
	"FrameFilterStore",
	"fwActiveManagedWaveSlotInterface",
	"fwAnimationComponent",
	"fwAnimDirector",
	"fwAnimDirectorComponentCharacterCreator",
	"fwAnimDirectorComponentCreature",
	"fwAnimDirectorComponentExpressions",
	"fwAnimDirectorComponentExtraOutputs",
	"fwAnimDirectorComponentFacialRig",
	"fwAnimDirectorComponentMotionTree",
	"fwAnimDirectorComponentMove",
	"fwAnimDirectorComponentParent_Parent",
	"fwAnimDirectorComponentParent",
	"fwAnimDirectorComponentPose",
	"fwAnimDirectorComponentRagDoll",
	"fwAnimDirectorComponentReplay",
	"fwArchetypePooledMap",
	"fwClothCollisionsExtension",
	"fwContainerLod",
	"fwCreatureComponent",
	"fwDynamicArchetypeComponent",
	"fwDynamicEntityComponent",
	"fwEntityContainer",
	"fwLodNode",
	"fwMatrixTransform",
	"fwMtUpdateSchedulerOperation",
	"fwQuaternionTransform",
	"fwScriptGuid",
	"fwSimpleTransform",
	"fwuiAnimationOpBase",
	"fwuiAnimationOpInstanceDataBase",
	"fwuiAnimationTargetBase",
	"fwuiAnimationValueBase",
	"fwuiBlip",
	"fwuiIconHandle",
	"fwuiVisualPromptData",
	"GamePlayerBroadcastDataHandler_Remote",
	"GrassBatch",
	"HandlingData",
	"InstanceBuffer",
	"InteriorInst",
	"InteriorProxy",
	"IplStore",
	"ItemSet",
	"ItemSetBuffer",
	"JointLimitDictStore",
	"Known Refs",
	"LadderEntities",
	"Landing gear parts",
	"LastInstMatrices",
	"LayoutNode",
	"LightEntity",
	"LootActionFinderFSM",
	"ManagedLootableEntityData",
	"MapDataLoadedNode",
	"MapDataStore",
	"MapTypesStore",
	"MaxBroadphasePairs",
	"MaxCachedRopeCount",
	"MaxClothCount",
	"MaxFoliageCollisions",
	"MaxLoadedInfo",
	"MaxLoadRequestedInfo",
	"MaxManagedRequests",
	"MaxNonRegionScenarioPointSpatialObjects",
	"MaxPreSimDependency",
	"MaxReleaseRefs",
	"MaxRiverPhysicsInsts",
	"MaxRopeCount",
	"MaxScenarioInteriorNames",
	"MaxScenarioPrompts",
	"MaxSingleThreadedPhysicsCallbacks",
	"MaxTrainScenarioPoints",
	"MaxUnguardedRequests",
	"MaxVisibleClothCount",
	"MetaDataStore",
	"MotionStore",
	"mvPageBufferSize",
	"naFoliageContactEvent",
	"naFoliageEntity",
	"naSpeechInst",
	"NavMeshes",
	"NavMeshRoute",
	"naVocalization",
	"netGameEvent",
	"netScriptSerialisationPlan_ExtraLarge",
	"netScriptSerialisationPlan_Large",
	"netScriptSerialisationPlan_Small",
	"NetworkCrewDataMgr",
	"NetworkDefStore",
	"NetworkEntityAreas",
	"NetworkScriptStatusManager",
	"Object",
	"ObjectDependencies",
	"ObjectIntelligence",
	"OcclusionInteriorInfo",
	"OcclusionPathNode",
	"OcclusionPortalEntity",
	"OcclusionPortalInfo",
	"PedProp render data",
	"PedProp req data",
	"PedRoute",
	"Peds",
	"PersistentLootableData",
	"phInstGta",
	"PhysicsBounds",
	"PMStore",
	"PortalInst",
	"PoseMatcherStore",
	"PtFxAssetStore",
	"PtFxSortedEntity",
	"QuadTreeNodes",
	"ScaleformMgrArray",
	"ScaleformStore",
	"ScenarioCarGensPerRegion",
	"ScenarioPoint",
	"ScenarioPointEntity",
	"ScenarioPointsAndEdgesPerRegion",
	"ScenarioPointWorld",
	"scrGlobals",
	"ScriptBrains",
	"ScriptStore",
	"sRequest",
	"StairsEntities",
	"StaticBounds",
	"StreamPed render data",
	"StreamPed req data",
	"SyncedScenes",
	"TaskSequenceInfo",
	"tcBox",
	"tcVolume",
	"TextStore",
	"TrafficLightInfos",
	"TxdStore",
	"Vehicles",
	"VehicleScenarioAttractors",
	"VehicleStreamRender",
	"VehicleStreamRequest",
	"VehicleStruct",
	"volAggregate",
	"volBox",
	"volCylinder",
	"volNetDataStateAggregate",
	"volNetDataStatePrimitive",
	"volSphere",
	"WorldUpdateEntities",
	"wptrec",
	"Wheels",
	"CMoveVehicle",
	"Vehicle Intelligence",
	"fragInstNMGta",
};

static RageHashList poolEntries(poolEntriesTable);

static std::unordered_map<uint32_t, std::string_view> pedPoolEntries{
	{ HashString("Peds"), "Peds" },
	{ HashString("CPedInventory"), "CPedInventory" },
	{ HashString("CPedAnimationComponent"), "CPedAnimationComponent" },
	{ HashString("CPedCreatureComponent"), "CPedCreatureComponent" },
	{ HashString("CPedAnimalComponent"), "CPedAnimalComponent" },
	{ HashString("CPedAnimalAudioComponent"), "CPedAnimalAudioComponent" },
	{ HashString("CPedCoreComponent"), "CPedCoreComponent" },
	{ HashString("CPedFacialComponent"), "CPedFacialComponent" },
	{ HashString("CPedGameplayComponent"), "CPedGameplayComponent" },
	{ HashString("CPedAttributeComponent"), "CPedAttributeComponent" },
	{ HashString("CPedDummyComponent"), "CPedDummyComponent" },
	{ HashString("CPedGraphicsComponent"), "CPedGraphicsComponent" },
	{ HashString("CPedVfxComponent"), "CPedVfxComponent" },
	{ HashString("CPedHealthComponent"), "CPedHealthComponent" },
	{ HashString("CPedHorseComponent"), "CPedHorseComponent" },
	{ HashString("CPedHumanAudioComponent"), "CPedHumanAudioComponent" },
	{ HashString("CPedEventComponent"), "CPedEventComponent" },
	{ HashString("CPedIntelligenceComponent"), "CPedIntelligenceComponent" },
	{ HashString("CPedInventoryComponent"), "CPedInventoryComponent" },
	{ HashString("CPedLookAtComponent"), "CPedLookAtComponent" },
	{ HashString("CPedMotionComponent"), "CPedMotionComponent" },
	{ HashString("CPedMotivationComponent"), "CPedMotivationComponent" },
	{ HashString("CPedPhysicsComponent"), "CPedPhysicsComponent" },
	{ HashString("CPedProjDecalComponent"), "CPedProjDecalComponent" },
	{ HashString("CPedRagdollComponent"), "CPedRagdollComponent" },
	{ HashString("CPedScriptDataComponent"), "CPedScriptDataComponent" },
	{ HashString("CPedStaminaComponent"), "CPedStaminaComponent" },
	{ HashString("CPedThreatResponseComponent"), "CPedThreatResponseComponent" },
	{ HashString("CPedTransportUserComponent"), "CPedTransportUserComponent" },
	{ HashString("CPedWeaponComponent"), "CPedWeaponComponent" },
	{ HashString("CPedWeaponManagerComponent"), "CPedWeaponManagerComponent" },
	{ HashString("CPedVisibilityComponent"), "CPedVisibilityComponent" },
	{ HashString("CNetBlenderPed"), "CNetBlenderPed" },
	{ HashString("CPedSyncData"), "CPedSyncData" },

	{ HashString("CPedFootstepComponent"), "CPedSyncData" },
	{ HashString("CCharacterItem"), "CCharacterItem" },
	{ HashString("CPedBreatheComponent"), "CPedBreatheComponent" }
};

static std::unordered_map<uint32_t, std::string_view> objectPoolEntries{
	{ HashString("CObjectSyncData"), "CObjectSyncData" },
	{ HashString("CNetBlenderPhysical"), "CNetBlenderPhysical" },
	{ HashString("atDNetObjectNode"), "atDNetObjectNode" },
	{ HashString("CObjectCollisionDetectedComponent"), "CObjectCollisionDetectedComponent" },
	{ HashString("reassignObjectInfo"), "reassignObjectInfo" }
};

static std::unordered_map<uint32_t, std::string_view> vehiclePoolEntries {
	{ HashString("Vehicles"), "Vehicles" }, // This already apply default size for CVehicle & 0x4CF5F35A(audVehicleAudioEntity)
	{ HashString("CVehicleSyncData"), "CVehicleSyncData" },
	{ HashString("CVehicleAnimationComponent"), "CVehicleAnimationComponent" },
	{ HashString("CVehicleDrivingComponent"), "CVehicleDrivingComponent" },
	{ HashString("CVehicleIntelligenceComponent"), "CVehicleIntelligenceComponent" },
	{ HashString("CVehiclePhysicsComponent"), "CVehiclePhysicsComponent" },
	{ HashString("CVehicleWeaponsComponent"), "CVehicleWeaponsComponent" },
	{ HashString("CVehicleCoreComponent"), "CVehicleCoreComponent" },
	
	{ HashString("atDNetObjectNode"), "atDNetObjectNode" },
	{ HashString("reassignObjectInfo"), "reassignObjectInfo" },
	{ HashString("CNetBlenderPhysical"), "CNetBlenderPhysical" },
};

atPoolBase* rage::GetPoolBase(uint32_t hash)
{
	auto it = g_pools.find(hash);

	if (it == g_pools.end())
	{
		return nullptr;
	}

	return it->second;
}

const std::unordered_map<std::string, atPoolBase*>& rage::GetPools()
{
	return g_namedPools;
}

static atPoolBase* SetPoolFn(atPoolBase* pool, uint32_t hash)
{
	g_pools[hash] = pool;
	g_inversePools.insert({ pool, hash });
	g_namedPools[poolEntries.LookupHash(hash)] = pool;

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
		g_namedPools.erase(poolEntries.LookupHash(hash));
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

		FatalErrorNoExcept("%s Pool Full, Size == %d", poolName, pool->GetSize());
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

int64_t(*g_origGetSizeOfPool)(void*, uint32_t, int);

static int64_t GetSizeOfPool(void* configManager, uint32_t poolHash, int defaultSize)
{
	int64_t size = g_origGetSizeOfPool(configManager, poolHash, defaultSize);

	// There are several pools that are tied to Peds/CNetObjPedBase. We want to ensure that the increased value is applied to all of these components.
	if (auto it = pedPoolEntries.find(poolHash); it != pedPoolEntries.end())
	{
		auto sizeIncreaseEntry = fx::PoolSizeManager::GetIncreaseRequest().find("CNetObjPedBase");
		if (sizeIncreaseEntry != fx::PoolSizeManager::GetIncreaseRequest().end())
		{
			size += sizeIncreaseEntry->second;
		}
		return size;
	}

	// There are several pools that are tied to Objects/CNetObjObject. We want to ensure that the increased value is applied to all of these components.
	if (auto it = objectPoolEntries.find(poolHash); it != objectPoolEntries.end())
	{
		auto sizeIncreaseEntry = fx::PoolSizeManager::GetIncreaseRequest().find("CNetObjObject");
		if (sizeIncreaseEntry != fx::PoolSizeManager::GetIncreaseRequest().end())
		{
			size += sizeIncreaseEntry->second;
		}
		return size;
	}

	if (auto it = vehiclePoolEntries.find(poolHash); it != vehiclePoolEntries.end())
	{
		auto sizeIncreaseEntry = fx::PoolSizeManager::GetIncreaseRequest().find("CNetObjVehicle");
		if (sizeIncreaseEntry != fx::PoolSizeManager::GetIncreaseRequest().end())
		{
			size += sizeIncreaseEntry->second;
		}
		return size;
	}

	auto sizeIncreaseEntry = fx::PoolSizeManager::GetIncreaseRequest().find(poolEntries.LookupHash(poolHash));
	if (sizeIncreaseEntry != fx::PoolSizeManager::GetIncreaseRequest().end())
	{
		size += sizeIncreaseEntry->second;
	}

	return size;
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
	MH_CreateHook(hook::get_pattern("83 79 ? ? 44 8B D2 74 ? 33 D2 41 8B C2 F7 71 ? 48 8B 41 ? 48 8B 0C D0 EB ? 44 3B 11 74 ? 48 8B 49 ? 48 85 C9 75 ? 48 85 C9 74 ? 8B 01"), GetSizeOfPool, (void**)&g_origGetSizeOfPool);
	MH_EnableHook(MH_ALL_HOOKS);

	// raw sfe reg from non-startup
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 8B D8 48 85 C0 75 26 8D 50 5C", -0x38), WrapLevelLoad, (void**)&g_origLevelLoad);
	MH_EnableHook(MH_ALL_HOOKS);
});
