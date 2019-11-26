#include "StdInc.h"
#include "Hooking.h"

#include <ETWProviders/etwprof.h>

#include <optick.h>

#include <gameSkeleton.h>
#include <Error.h>

#include <ICoreGameInit.h>

static std::unordered_map<uint32_t, std::string> g_initFunctionNames;

// rage::strStreamingEngine::ms_bIsPerformingAsyncInit
static bool* bIsPerformingAsyncInit;

// #TODORDR: NEEDS CODE SHARING WITH FIVE

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

					*bIsPerformingAsyncInit = true;

					for (bool async : { true, false })
					{
						for (int index : entry->functions)
						{
							auto func = m_initFunctions[index];

							bool isAsync = (func.asyncInitMask & type) != 0;

							if (async != isAsync)
							{
								continue;
							}

							if (OnInitFunctionInvoking(type, i, func))
							{
								trace(__FUNCTION__ ": Invoking %s %s%s init (%i out of %i)\n", func.GetName(), InitFunctionTypeToString(type), async ? " (async)" : "", i + 1, entry->functions.GetCount());

								CETWScope scope(va("%s %s", func.GetName(), InitFunctionTypeToString(type)));

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
					}

					*bIsPerformingAsyncInit = false;

					OnInitFunctionEndOrder(type, entry->order);
				}
			}
		}

		OnInitFunctionEnd(type);

		trace(__FUNCTION__ ": Done running %s init functions!\n", InitFunctionTypeToString(type));
	}

	static void RunEntries(gameSkeleton_updateBase* update)
	{
		for (auto entry = update; entry; entry = entry->m_nextPtr)
		{
#if USE_OPTICK
			static std::unordered_map<uint32_t, Optick::EventDescription*> events;

			auto it = events.find(entry->m_hash);

			if (it == events.end())
			{
				auto entryIt = g_initFunctionNames.find(entry->m_hash);
				std::string name;

				if (entryIt != g_initFunctionNames.end())
				{
					name = entryIt->second;
				}
				else
				{
					name = fmt::sprintf("0x%08x", entry->m_hash);
				}

				it = events.emplace(entry->m_hash, Optick::EventDescription::Create(strdup(va("%s update", name)), __FILE__, __LINE__, Optick::Color::Beige)).first;
			}

			Optick::Event event(*it->second);
#endif

			// skip a potential crashing subsystem
			if (entry->m_hash == 0x73AA6F9E)
			{
				return;
			}

			entry->Run();
		}
	}

	void gameSkeleton::RunUpdate(int type)
	{
#if USE_OPTICK
		static Optick::EventDescription* events[3];
		if (events[type] == nullptr)
		{
			events[type] = Optick::EventDescription::Create(strdup(va("gameSkeleton update %d", type)), __FILE__, __LINE__, Optick::Color::Gold);
		}

		Optick::Event outerEvent(*events[type]);
#endif

		for (auto list = m_updateFunctionList; list; list = list->next)
		{
			if (list->type == type)
			{
				RunEntries(list->entry);
			}
		}
	}

	void gameSkeleton_updateBase::RunGroup()
	{
		RunEntries(this->m_childPtr);
	}
}

static HookFunction hookFunction([] ()
{
	{
		void* loc = hook::pattern("BA 04 00 00 00 48 8D 0D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8").count(1).get(0).get<void>(12);
		//void* loc = hook::pattern("BA 04 00 00 00 48 8D 0D ? ? ? ? E8 ? ? ? ? C6 05").count(1).get(0).get<void>(12);

		hook::jump(hook::get_call(loc), hook::get_member(&rage::gameSkeleton::RunInitFunctions));
	}

	{
		hook::jump(hook::get_call(hook::get_pattern("E8 ? ? ? ? BA 01 00 00 00 48 8D 0D ? ? ? ? E8 ? ? ? ? E8", 12)), hook::get_member(&rage::gameSkeleton::RunUpdate));
	}

	hook::jump(hook::get_pattern("40 53 48 83 EC 20 48 8B 59 20 EB 0D 48 8B 03 48"), hook::get_member(&rage::gameSkeleton_updateBase::RunGroup));

	bIsPerformingAsyncInit = hook::get_address<bool*>(hook::get_pattern("42 F6 04 13 01 75 10 80 3D", 9)) + 1; // as insn is longer
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
