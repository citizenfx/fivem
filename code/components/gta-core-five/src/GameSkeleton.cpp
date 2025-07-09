#include "StdInc.h"
#include "Hooking.h"

#include <optick.h>

#include <gameSkeleton.h>
#include <Error.h>

#include <ICoreGameInit.h>
#include <CrossBuildRuntime.h>

#include <string_view>
#include <cstdio>
#include <algorithm>

extern void ValidateHeaps();

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
		const auto it = g_initFunctionNames.find(funcHash);

		if (it != g_initFunctionNames.end()) [[likely]]
		{
			return it->second.c_str();
		}

		static thread_local std::unordered_map<uint32_t, std::string> hashCache;
		hashCache.reserve(32); 
		
		auto cacheIt = hashCache.find(funcHash);
		if (cacheIt != hashCache.end()) [[likely]]
		{
			return cacheIt->second.c_str();
		}
		
		auto [insertIt, inserted] = hashCache.emplace(funcHash, "");
		insertIt->second.resize(10);
		std::snprintf(insertIt->second.data(), 11, "0x%08x", funcHash);
		
		return insertIt->second.c_str();
	}

	static int SehRoutine(InitFunctionData* func, InitFunctionType type, PEXCEPTION_POINTERS exception)
	{
		const auto* exceptionRecord = exception->ExceptionRecord;
		
		if (exceptionRecord->ExceptionCode & 0x80000000) [[unlikely]]
		{
			const char* funcName = func->GetName();
			const char* typeString = InitFunctionTypeToString(type);
			
			AddCrashometry("init_function", "%s:%s", typeString, funcName);

			if (IsErrorException(exception)) [[unlikely]]
			{
				return EXCEPTION_CONTINUE_SEARCH;
			}

			static const uintptr_t minAddress = hook::get_adjusted(0x140000000);
			static const uintptr_t maxAddress = hook::get_adjusted(0x146000000);
			const uintptr_t exceptionAddress = (uintptr_t)exceptionRecord->ExceptionAddress;
			
			if (exceptionAddress >= minAddress && exceptionAddress < maxAddress) [[likely]]
			{
				FatalErrorNoExcept("An exception occurred (%08x at %p) during execution of the %s function for %s. The game will be terminated.",
					exceptionRecord->ExceptionCode, (void*)hook::get_unadjusted(exceptionAddress),
					typeString, funcName);
			}
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	bool InitFunctionData::TryInvoke(InitFunctionType type)
	{
#ifndef _DEBUG
		__try
		{
#endif
			__builtin_prefetch(reinterpret_cast<const void*>(initFunction), 0, 3);
			
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
		ValidateHeaps();

		const char* typeString = InitFunctionTypeToString(type);
		trace(__FUNCTION__ ": Running %s init functions\n", typeString);

		OnInitFunctionStart(type);

		static auto rageTimer = hook::get_address<void(*)()>(hook::get_pattern("48 83 EC 28 E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9", 4));

		for (auto list = m_initFunctionList; list; list = list->next)
		{
			if (list->type == type) [[likely]]
			{
				for (auto entry = list->entries; entry; entry = entry->next)
				{
					const int functionCount = entry->functions.GetCount();
					OnInitFunctionStartOrder(type, entry->order, functionCount);
					trace(__FUNCTION__ ": Running functions of order %i (%i total)\n", entry->order, functionCount);

					int i = 0;

					for (const int index : entry->functions)
					{
						ValidateHeaps();

						const auto& func = m_initFunctions[index];

						if (OnInitFunctionInvoking(type, i, func)) [[likely]]
						{
							trace(__FUNCTION__ ": Invoking %s %s init (%i out of %i)\n", func.GetName(), typeString, i + 1, functionCount);

							assert(func.TryInvoke(type));
						}
						else
						{
							trace(__FUNCTION__ ": %s %s init canceled by event\n", func.GetName(), typeString);
						}

						if (rageTimer) [[likely]] {
							rageTimer();
						}

						OnInitFunctionInvoked(type, func);

						++i;

						ValidateHeaps();
					}

					OnInitFunctionEndOrder(type, entry->order);
				}
			}
		}

		OnInitFunctionEnd(type);

		ValidateHeaps();

		trace(__FUNCTION__ ": Done running %s init functions!\n", typeString);
	}

	static void RunEntries(gameSkeleton_updateBase* update)
	{
		static const uint32_t rageSecEngineHash = HashString("rageSecEngine");
		
		for (auto entry = update; entry; entry = entry->m_nextPtr)
		{
#if USE_OPTICK
			thread_local std::unordered_map<uint32_t, Optick::EventDescription*> events;
			events.reserve(64); 

			auto it = events.find(entry->m_hash);

			if (it == events.end()) [[unlikely]]
			{
				std::string_view name;
				static thread_local char hashBuffer[16];
				
				auto entryIt = g_initFunctionNames.find(entry->m_hash);
				if (entryIt != g_initFunctionNames.end()) [[likely]]
				{
					name = entryIt->second;
				}
				else
				{
					std::snprintf(hashBuffer, sizeof(hashBuffer), "0x%08x", entry->m_hash);
					name = hashBuffer;
				}

				static thread_local std::string eventName;
				eventName.clear();
				eventName.reserve(name.size() + 8); 
				eventName.append(name);
				eventName.append(" update");
				
				it = events.emplace(entry->m_hash, Optick::EventDescription::Create(eventName.c_str(), __FILE__, __LINE__, Optick::Color::Beige)).first;
			}

			Optick::Event event(*it->second);
#endif

			if (entry->m_hash == rageSecEngineHash) [[unlikely]]
			{
				return;
			}

			entry->Run();
		}
	}

	void gameSkeleton::RunUpdate(int type)
	{
#if USE_OPTICK
		static constexpr int MAX_UPDATE_TYPES = 8;
		static Optick::EventDescription* events[MAX_UPDATE_TYPES] = {nullptr};
		
		if (type >= 0 && type < MAX_UPDATE_TYPES) [[likely]]
		{
			if (events[type] == nullptr) [[unlikely]]
			{
				static thread_local char eventNameBuffer[32];
				std::snprintf(eventNameBuffer, sizeof(eventNameBuffer), "gameSkeleton update %d", type);
				events[type] = Optick::EventDescription::Create(eventNameBuffer, __FILE__, __LINE__, Optick::Color::Gold);
			}

			Optick::Event outerEvent(*events[type]);
		}
#endif

		for (auto list = m_updateFunctionList; list; list = list->next)
		{
			if (list->type == type) [[likely]]
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
		void* loc = hook::pattern("BA 04 00 00 00 E8 ? ? ? ? E8 ? ? ? ? E8").count(1).get(0).get<void>(5);

		hook::jump(hook::get_call(loc), hook::get_member(&rage::gameSkeleton::RunInitFunctions));
	}

	{
		hook::jump(hook::get_call(hook::get_pattern("48 8D 0D ? ? ? ? BA 02 00 00 00 84 DB 75 05", -17)), hook::get_member(&rage::gameSkeleton::RunUpdate));
	}

	hook::jump(hook::get_pattern("40 53 48 83 EC 20 48 8B 59 20 EB 0D 48 8B 03 48"), hook::get_member(&rage::gameSkeleton_updateBase::RunGroup));
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

	"CAnimSceneManager",
	"CTextInputBox",
	"CMultiplayerChat",
	"CCreditsText",
	"CReplayMgr",
	"CReplayCoordinator",
	"CMousePointer",
	"CVideoEditorUI",
	"CVideoEditorInterface",
	"VideoRecording",
	"WatermarkRenderer",
};

static InitFunction initFunction([] ()
{
	constexpr size_t knownFunctionCount = sizeof(g_initFunctionKnown) / sizeof(g_initFunctionKnown[0]);
	g_initFunctionNames.reserve(knownFunctionCount * 2);
	
	for (const auto& str : g_initFunctionKnown)
	{
		const uint32_t hashStr = HashString(str);
		const uint32_t rageHashStr = HashRageString(str);
		
		auto hint = g_initFunctionNames.emplace(hashStr, str).first;
		g_initFunctionNames.emplace_hint(hint, rageHashStr, str);
	}
});
