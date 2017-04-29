#include "StdInc.h"
#include "Hooking.h"

#include <gameSkeleton.h>
#include <Error.h>

static std::unordered_map<uint32_t, std::string> g_initFunctionNames;

namespace rage
{
	const char* InitFunctionTypeToString(InitFunctionType type)
	{
		switch (type)
		{
			case INIT_CORE:
				return "INIT_CORE";

			case INIT_BEFORE_MAP_LOADED:
				return "INIT_BEFORE_MAP_LOADED";

			case INIT_AFTER_MAP_LOADED:
				return "INIT_AFTER_MAP_LOADED";

			case INIT_SESSION:
				return "INIT_SESSION";
		}

		return "INIT_UNKNOWN";
	}

	const char* InitFunctionData::GetName() const
	{
		auto& it = g_initFunctionNames.find(funcHash);

		if (it != g_initFunctionNames.end())
		{
			return it->second.c_str();
		}

		return va("0x%08x", funcHash);
	}

	static int SehRoutine(InitFunctionData* func, InitFunctionType type, PEXCEPTION_POINTERS exception)
	{
		if (exception->ExceptionRecord->ExceptionCode & 0x80000000)
		{
			FatalErrorNoExcept("An exception occurred (%08x at %p) during execution of the %s function for %s. The game will be terminated.",
				exception->ExceptionRecord->ExceptionCode, exception->ExceptionRecord->ExceptionAddress,
				InitFunctionTypeToString(type), func->GetName());
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	bool InitFunctionData::TryInvoke(InitFunctionType type)
	{
#ifndef _DEBUG
		__try
		{
#endif
			initFunction(type);

			return true;
#ifndef _DEBUG
		}
		__except (SehRoutine(this, type, GetExceptionInformation()))
		{
			return false;
		}
#endif
	}

	fwEvent<InitFunctionType> OnInitFunctionStart;
	fwEvent<InitFunctionType, int, int> OnInitFunctionStartOrder;
	fwEvent<InitFunctionType, int, InitFunctionData&> OnInitFunctionInvoking;
	fwEvent<InitFunctionType, const InitFunctionData&> OnInitFunctionInvoked;
	fwEvent<InitFunctionType, int> OnInitFunctionEndOrder;
	fwEvent<InitFunctionType> OnInitFunctionEnd;

	void gameSkeleton::RunInitFunctions(InitFunctionType type)
	{
		trace(__FUNCTION__ ": Running %s init functions\n", InitFunctionTypeToString(type));

		OnInitFunctionStart(type);

		for (auto list = m_initFunctionList; list; list = list->next)
		{
			if (list->type == type)
			{
				for (auto entry = list->entries; entry; entry = entry->next)
				{
					OnInitFunctionStartOrder(type, entry->order, entry->functions.GetCount());
					trace(__FUNCTION__ ": Running functions of order %i (%i total)\n", entry->order, entry->functions.GetCount());

					int i = 0;

					for (int index : entry->functions)
					{
						auto func = m_initFunctions[index];

						if (OnInitFunctionInvoking(type, i, func))
						{
							trace(__FUNCTION__ ": Invoking %s %s init (%i out of %i)\n", func.GetName(), InitFunctionTypeToString(type), i + 1, entry->functions.GetCount());

							assert(func.TryInvoke(type));
						}
						else
						{
							trace(__FUNCTION__ ": %s %s init canceled by event\n", func.GetName(), InitFunctionTypeToString(type));
						}

						// TODO: recalibrate RAGE timer

						OnInitFunctionInvoked(type, func);

						++i;
					}

					OnInitFunctionEndOrder(type, entry->order);
				}
			}
		}

		OnInitFunctionEnd(type);

		trace(__FUNCTION__ ": Done running %s init functions!\n", InitFunctionTypeToString(type));
	}
}

static HookFunction hookFunction([] ()
{
	void* loc = hook::pattern("BA 04 00 00 00 E8 ? ? ? ? E8 ? ? ? ? E8").count(1).get(0).get<void>(5);

	hook::jump(hook::get_call(loc), hook::get_member(&rage::gameSkeleton::RunInitFunctions));
});

static const char* const g_initFunctionKnown[] = {
	"AmbientLights",
	"AnimBlackboard",
	"Audio",
	"BackgroundScripts",
	"CActionManager",
	"CAgitatedManager",
	"CAmbientAnimationManager",
	"CAmbientAudioManager",
	"CAmbientModelSetManager",
	"CAnimBlackboard",
	"CAppDataMgr",
	"CAssistedMovementRouteStore",
	"CBoatChaseDirector",
	"CBuses",
	"CBusySpinner",
	"CCheat",
	"CCheckCRCs",
	"CClipDictionaryStoreInterface",
	"CClock",
	"CCombatDirector",
	"CCombatInfoMgr",
	"CCompEntity",
	"CConditionalAnimManager",
	"CContentExport",
	"CContentSearch",
	"CControl",
	"CControlMgr",
	"CControllerLabelMgr",
	"CCover",
	"CCoverFinder",
	"CCredits",
	"CCrimeInformationManager",
	"CCullZones",
	"CDLCScript",
	"CDecoratorInterface",
	"CDispatchData",
	"CEventDataManager",
	"CExpensiveProcessDistributer",
	"CExplosionManager",
	"CExtraContent",
	"CExtraContentWrapper",
	"CExtraContentWrapper::Shutdown",
	"CExtraContentWrapper::ShutdownStart",
	"CExtraMetadataMgr",
	"CExtraMetadataMgr::ClassInit",
	"CExtraMetadataMgr::ClassShutdown",
	"CExtraMetadataMgr::ShutdownDLCMetaFiles",
	"CFlyingVehicleAvoidanceManager",
	"CFocusEntityMgr",
	"CFrontendStatsMgr",
	"CGameLogic",
	"CGameSituation",
	"CGameStreamMgr",
	"CGameWorld",
	"CGameWorldHeightMap",
	"CGameWorldWaterHeight",
	"CGarages",
	"CGenericGameStorage",
	"CGestureManager",
	"CGps",
	"CGtaAnimManager",
	"CHandlingDataMgr",
	"CInstanceListAssetLoader::Init",
	"CInstanceListAssetLoader::Shutdown",
	"CIplCullBox",
	"CJunctions",
	"CLODLightManager",
	"CLODLights",
	"CLadderMetadataManager",
	"CLoadingScreens",
	"CMapAreas",
	"CMapZoneManager",
	"CMessages",
	"CMiniMap",
	"CModelInfo",
	"CModelInfo::Init",
	"CMovieMeshManager",
	"CMultiplayerGamerTagHud",
	"CNetRespawnMgr",
	"CNetwork",
	"CNetworkTelemetry",
	"CNewHud",
	"CObjectPopulationNY",
	"COcclusion",
	"CParaboloidShadow",
	"CPathFind",
	"CPathServer::InitBeforeMapLoaded",
	"CPathServer::InitSession",
	"CPathServer::ShutdownSession",
	"CPathZoneManager",
	"CPatrolRoutes",
	"CPauseMenu",
	"CPed",
	"CPedAILodManager",
	"CPedGeometryAnalyser",
	"CPedModelInfo",
	"CPedPopulation",
	"CPedPopulation::ResetPerFrameScriptedMu",
	"CPedPopulation::ResetPerFrameScriptedMultipiers",
	"CPedPropsMgr",
	"CPedVariationPack",
	"CPedVariationStream",
	"CPerformance",
	"CPhoneMgr",
	"CPhotoManager",
	"CPhysics",
	"CPickupDataManager",
	"CPickupManager",
	"CPlantMgr",
	"CPlayStats",
	"CPlayerSwitch",
	"CPopCycle",
	"CPopZones",
	"CPopulationStreaming",
	"CPopulationStreamingWrapper",
	"CPortal",
	"CPortalTracker",
	"CPostScan",
	"CPrecincts",
	"CPrioritizedClipSetRequestManager",
	"CPrioritizedClipSetStreamer",
	"CProcObjectMan",
	"CProceduralInfo",
	"CProfileSettings",
	"CRandomEventManager",
	"CRecentlyPilotedAircraft",
	"CRenderPhaseCascadeShadowsInterface",
	"CRenderTargetMgr",
	"CRenderThreadInterface",
	"CRenderer",
	"CReportMenu",
	"CRestart",
	"CRiots",
	"CRoadBlock",
	"CScaleformMgr",
	"CScenarioActionManager",
	"CScenarioManager",
	"CScenarioManager::ResetExclusiveScenari",
	"CScenarioManager::ResetExclusiveScenarioGroup",
	"CScenarioPointManager",
	"CScenarioPointManagerInitSession",
	"CScene",
	"CSceneStreamerMgr::PreScanUpdate",
	"CScriptAreas",
	"CScriptCars",
	"CScriptDebug",
	"CScriptEntities",
	"CScriptHud",
	"CScriptPedAIBlips",
	"CScriptPeds",
	"CScriptedGunTaskMetadataMgr",
	"CShaderHairSort",
	"CShaderLib",
	"CSituationalClipSetStreamer",
	"CSky",
	"CSlownessZonesManager",
	"CSprite2d",
	"CStaticBoundsStore",
	"CStatsMgr",
	"CStreaming",
	"CStreamingRequestList",
	"CStuntJumpManager",
	"CTVPlaylistManager",
	"CTacticalAnalysis",
	"CTask",
	"CTaskClassInfoManager",
	"CTaskRecover",
	"CTexLod",
	"CText",
	"CThePopMultiplierAreas",
	"CTheScripts",
	"CTimeCycle",
	"CTrafficLights",
	"CTrain",
	"CTuningManager",
	"CUserDisplay",
	"CVehicleAILodManager",
	"CVehicleChaseDirector",
	"CVehicleCombatAvoidanceArea",
	"CVehicleDeformation",
	"CVehicleMetadataMgr",
	"CVehicleModelInfo",
	"CVehiclePopulation",
	"CVehiclePopulation::ResetPerFrameScript",
	"CVehiclePopulation::ResetPerFrameScriptedMultipiers",
	"CVehicleRecordingMgr",
	"CVehicleVariationInstance",
	"CVisualEffects",
	"CWarpManager",
	"CWaypointRecording",
	"CWeaponManager",
	"CWitnessInformationManager",
	"CWorldPoints",
	"CZonedAssetManager",
	"Common",
	"CreateFinalScreenRenderPhaseList",
	"Credits",
	"CutSceneManager",
	"CutSceneManagerWrapper",
	"FacialClipSetGroupManager",
	"FireManager",
	"FirstPersonProp",
	"FirstPersonPropCam",
	"Game",
	"GenericGameStoragePhotoGallery",
	"INSTANCESTORE",
	"ImposedTxdCleanup",
	"InitSystem",
	"Kick",
	"LightEntityMgr",
	"Lights",
	"MeshBlendManager",
	"Misc",
	"NewHud",
	"Occlusion",
	"PauseMenu",
	"Ped",
	"PedHeadShotManager",
	"PedModelInfo",
	"PedPopulation",
	"PlantsMgr::UpdateBegin",
	"PlantsMgr::UpdateEnd",
	"Population",
	"PostFX",
	"PostFx",
	"Pre-vis",
	"Prioritized",
	"Proc",
	"ProcessAfterCameraUpdate",
	"ProcessAfterMovement",
	"ProcessPedsEarlyAfterCameraUpdate",
	"Render",
	"ResetSceneLights",
	"Run",
	"Script",
	"ScriptHud",
	"ShaderLib::Update",
	"Situational",
	"SocialClubMenu",
	"Streaming",
	"UI3DDrawManager",
	"UIWorldIconManager",
	"Update",
	"VehPopulation",
	"VideoPlayback",
	"VideoPlaybackThumbnailManager",
	"VideoPlaybackThumbnails",
	"Viewport",
	"ViewportSystemInit",
	"ViewportSystemInitLevel",
	"ViewportSystemShutdown",
	"ViewportSystemShutdownLevel",
	"Visibility",
	"Visual",
	"WarningScreen",
	"Water",
	"WaterHeightSim",
	"World",
	"audNorthAudioEngine",
	"audNorthAudioEngineDLC",
	"cStoreScreenMgr",
	"camManager",
	"decorators",
	"fwAnimDirector",
	"fwClipSetManager",
	"fwClothStore",
	"fwDrawawableStoreWrapper",
	"fwDwdStore",
	"fwDwdStoreWrapper",
	"fwExpressionSetManager",
	"fwFacialClipSetGroupManager",
	"fwFragmentStoreWrapper",
	"fwMapTypesStore",
	"fwMetaDataStore",
	"fwTimer",
	"fwTxdStore",
	"perfClearingHouse",
	"strStreamingEngine::SubmitDeferredAsyncPlacementRequests",
};

static InitFunction initFunction([] ()
{
	for (auto str : g_initFunctionKnown)
	{
		g_initFunctionNames.insert({ HashString(str), str });
		g_initFunctionNames.insert({ HashRageString(str), str });
	}
});