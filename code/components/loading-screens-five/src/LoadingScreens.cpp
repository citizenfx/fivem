#include <StdInc.h>
#include <CefOverlay.h>

#include <ICoreGameInit.h>
#include <CL2LaunchMode.h>

#include <Hooking.h>
#ifdef GTA_FIVE
#include <StatusText.h>
#endif
#include <Resource.h>

#include <gameSkeleton.h>

#include <HostSystem.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>

#include <ScriptEngine.h>
#include <ScriptHandlerMgr.h>

#include <GameInit.h>

#include <CoreConsole.h>

#include <CfxRect.h>
#include <DrawCommands.h>

#include <CrossBuildRuntime.h>

#include <Error.h>

#include <rageVectors.h>

#ifdef IS_RDR3
#include <ConsoleHost.h>
#include <imgui.h>
#endif


static std::shared_ptr<ConVar<bool>> g_loadProfileConvar;
static std::map<uint64_t, std::chrono::milliseconds> g_loadTiming;
static std::chrono::milliseconds g_loadTimingBase;
static std::set<uint64_t> g_visitedTimings;

static bool ShouldSkipLoading();

using fx::Resource;

bool g_doDrawBelowLoadingScreens;
static bool frameOn = false;
static bool primedMapLoad = false;
#ifdef GTA_FIVE
enum NativeIdentifiers : uint64_t
{
	SHUTDOWN_LOADING_SCREEN = 0x078EBE9809CCD637,
	LOAD_ALL_OBJECTS_NOW = 0xBD6E84632DD4CB3F,
	DO_SCREEN_FADE_OUT = 0x891B5B39AC6302AF,
	IS_SCREEN_FADED_OUT = 0xB16FCE9DDC7BA182,
	IS_SCREEN_FADING_OUT = 0x797AC7CB535BA28F,
	DO_SCREEN_FADE_IN = 0xD4E8E24955024033,
	IS_SCREEN_FADING_IN = 0x5C544BC6C57AC575,
	IS_SCREEN_FADED_IN = 0x5A859503B0C08678,
	CREATE_CAM = 0xC3981DCE61D9E13F,
	SET_CAM_COORD = 0x078EBE9809CCD637,
	SET_CAM_ROT = 0x85973643155D0B07,
	SET_CAM_FOV = 0xB13C14F66A00D047,
	RENDER_SCRIPT_CAMS = 0x07E5B515DB0636FC,
	SET_WEATHER_TYPE_PERSIST = 0xED712CA327900C8A,
	NETWORK_OVERRIDE_CLOCK_TIME = 0xE679E3E06E363892,
	SET_ENTITY_COORDS = 0x06843DA7060A026B,
	PLAYER_PED_ID = 0xD80958FC74E988A6,
	FREEZE_ENTITY_POSITION = 0x428CA6DBD1094446,
	SET_FOCUS_AREA = 0xBB7454BAFF08FE25,
	HIDE_HUD_AND_RADAR_THIS_FRAME = 0x719FF505F097FD20,
	DESTROY_CAM = 0x865908C81A2C22E9,
	CLEAR_FOCUS = 0x31B73D1EA9F01DA2,
	CLEAR_WEATHER_TYPE_PERSIST = 0xCCC39339BEF76CF5,

};
#elif IS_RDR3
enum NativeIdentifiers : uint64_t
{
	SHUTDOWN_LOADING_SCREEN = 0xFC179D7E8886DADF,
	DO_SCREEN_FADE_OUT = 0x40C719A5E410B9E4,
	IS_SCREEN_FADED_OUT = 0xF5472C80DF2FF847,
	IS_SCREEN_FADING_OUT = 0x02F39BEFE7B88D00,
	DO_SCREEN_FADE_IN = 0x6A053CF596F67DF7,
	IS_SCREEN_FADING_IN = 0x0CECCC63FFA2EF24,
	IS_SCREEN_FADED_IN = 0x37F9A426FBCF4AF2,
	CREATE_CAM = 0xE72CDBA7F0A02DD6,
	SET_CAM_COORD = 0xF9EE7D419EE49DE6,
	SET_CAM_ROT = 0x63DFA6810AD78719,
	SET_CAM_FOV = 0x27666E5988D9D429,
	RENDER_SCRIPT_CAMS = 0x33281167E4942E4F,
	NETWORK_OVERRIDE_CLOCK_TIME = 0x669E223E64B1903C,
	SET_ENTITY_COORDS = 0x06843DA7060A026B,
	PLAYER_PED_ID = 0x096275889B8E0EE0,
	FREEZE_ENTITY_POSITION = 0x7D9EFB7AD6B19754,
	SET_FOCUS_AREA = 0x25F6EF88664540E2,
	HIDE_HUD_AND_RADAR_THIS_FRAME = 0x36CDD81627A6FCD2,
	DESTROY_CAM = 0x4E67E0B6D7FD5145,
	CLEAR_FOCUS = 0x86CCAF7CE493EFBE,
	IS_LOADING_SCREEN_VISIBLE = 0xB54ADBE65D528FCB,
	SET_OVERRIDE_WEATHER = 0xBE83CAE8ED77A94F,
	CLEAR_OVERRIDE_WEATHER = 0x80A398F16FFE3CC3,

};
#endif
#ifdef GTA_FIVE
static rage::Vector3 defaultCameraPos = { -2153.641f, 4597.957f, 116.662f };
static rage::Vector3 defaultCameraRot = { -8.601f, 0.0f, 253.026f };
static float defaultCameraFov = 45.0f;
#elif IS_RDR3
static rage::Vector3 defaultCameraPos = { 2709.881836f, -1407.226074f, 49.1040610f };
static rage::Vector3 defaultCameraRot = { 6.94207f, 0.000000f, 71.483643f };
static float defaultCameraFov = 55.0f;
static std::string statusText = "Loading...";
static bool shouldShowStatusText = false;
#endif

static void InvokeNUIScript(const std::string& eventName, rapidjson::Document& json)
{
	if (!frameOn)
	{
		return;
	}

	json.AddMember("eventName", rapidjson::Value(eventName.c_str(), json.GetAllocator()), json.GetAllocator());

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	
	if (json.Accept(writer))
	{
		// For SDK we will also send that as SDK_MESSAGE
		if (launch::IsSDKGuest())
		{
			static constexpr uint32_t SEND_SDK_MESSAGE = HashString("SEND_SDK_MESSAGE");

			NativeInvoke::Invoke<SEND_SDK_MESSAGE, const char*>(fmt::sprintf("{type:'game:loadingScreen',data:%s}", sb.GetString()).c_str());
		}

		nui::PostFrameMessage("loadingScreen", sb.GetString());
	}
}

static void DestroyFrame()
{
	if (frameOn)
	{
		g_doDrawBelowLoadingScreens = false;

		nui::DestroyFrame("loadingScreen");

		frameOn = false;
	}
}

int CountRelevantDataFileEntries();
extern fwEvent<int, const char*> OnReturnDataFileEntry;
extern fwEvent<int, void*, void*> OnInstrumentedFunctionCall;

#include <scrEngine.h>

class LoadsThread : public CfxThread
{
public:
	virtual void DoRun() override;

	virtual void Reset() override
	{
		doSetup = false;
		doShutdown = false;
		isShutdown = false;
		sh = false;
	}

public:
	bool doShutdown;
	bool doSetup;
	bool isShutdown;
	bool sh;

	int cam;
};

static LoadsThread loadsThread;

static bool autoShutdownNui = true;
static fx::TNativeHandler g_origShutdown;

#include <nutsnbolts.h>

static HookFunction hookFunction([]()
{
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		static bool endedLoadingScreens = false;

		auto endLoadingScreens = [=]()
		{
			if (!endedLoadingScreens)
			{
				endedLoadingScreens = true;

				DestroyFrame();

				nui::OverrideFocus(false);
			}
		};

		{
			auto handler = fx::ScriptEngine::GetNativeHandler(SHUTDOWN_LOADING_SCREEN);

			if (!handler)
			{
				trace("Couldn't find SHUTDOWN_LOADING_SCREEN to hook!\n");
				return;
			}

			g_origShutdown = handler;
			// we want call this as soon as posible

			fx::ScriptEngine::RegisterNativeHandler("SET_MANUAL_SHUTDOWN_LOADING_SCREEN_NUI", [](fx::ScriptContext& ctx)
			{
				//autoShutdownNui = !ctx.GetArgument<bool>(0);

				trace("SET_MANUAL_SHUTDOWN_LOADING_SCREEN_NUI is not supported at this time. Use `loadscreen_manual_shutdown 'yes'` in your fxmanifest.lua instead.\n");
			});

			fx::ScriptEngine::RegisterNativeHandler("SHUTDOWN_LOADING_SCREEN_NUI", [=](fx::ScriptContext& ctx)
			{
				endLoadingScreens();
			});

			fx::ScriptEngine::RegisterNativeHandler(SHUTDOWN_LOADING_SCREEN, [=](fx::ScriptContext& ctx)
			{
				handler(ctx);

				loadsThread.doSetup = true;
				g_doDrawBelowLoadingScreens = false;
#ifdef IS_RDR3
				shouldShowStatusText = false;
#endif

				if (autoShutdownNui)
				{
					endLoadingScreens();
				}
			});
		}

		{
			Instance<ICoreGameInit>::Get()->OnTriggerError.Connect([=](const std::string& errorMessage)
			{
				endLoadingScreens();

				return true;
			});
		}

		OnKillNetworkDone.Connect([=]()
		{
			nui::DestroyFrame("loadingScreen");
		});

		Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([]()
		{
			endedLoadingScreens = false;
			autoShutdownNui = true;
		});
#ifdef GTA_FIVE
		{
			// override LOAD_ALL_OBJECTS_NOW
			auto handler = fx::ScriptEngine::GetNativeHandler(LOAD_ALL_OBJECTS_NOW);

			if (handler)
			{
				fx::ScriptEngine::RegisterNativeHandler(LOAD_ALL_OBJECTS_NOW, [=](fx::ScriptContext& ctx)
				{
					if (!endedLoadingScreens)
					{
						trace("Skipping LOAD_ALL_OBJECTS_NOW as loading screens haven't ended yet!\n");
						return;
					}

					handler(ctx);
				});
			}
		}
#endif
		{
			static uint64_t fakeFadeOutTime;
			static int fakeFadeOutLength;

			auto handler = fx::ScriptEngine::GetNativeHandler(DO_SCREEN_FADE_OUT);

			fx::ScriptEngine::RegisterNativeHandler(DO_SCREEN_FADE_OUT, [=](fx::ScriptContext& ctx)
			{
				// reset so a spawnmanager-style wait for collision will be fine
				if (loadsThread.isShutdown)
				{
					// CLEAR_FOCUS
					NativeInvoke::Invoke<CLEAR_FOCUS, int>();
				}

				// IS_SCREEN_FADED_OUT
				if (NativeInvoke::Invoke<IS_SCREEN_FADED_OUT, bool>())
				{
					fakeFadeOutTime = GetTickCount64();
					fakeFadeOutLength = ctx.GetArgument<int>(0);
					return;
				}

				handler(ctx);
			});

			auto handlerIs = fx::ScriptEngine::GetNativeHandler(IS_SCREEN_FADING_OUT);

			fx::ScriptEngine::RegisterNativeHandler(IS_SCREEN_FADING_OUT, [=](fx::ScriptContext& ctx)
			{
				handlerIs(ctx);

				if ((GetTickCount64() - fakeFadeOutTime) < fakeFadeOutLength)
				{
					ctx.SetResult(1);
				}
			});
		}

		{
			static uint64_t fakeFadeOutTime;
			static int fakeFadeOutLength;

			auto handler = fx::ScriptEngine::GetNativeHandler(DO_SCREEN_FADE_IN);

			fx::ScriptEngine::RegisterNativeHandler(DO_SCREEN_FADE_IN, [=](fx::ScriptContext& ctx)
			{
				if (NativeInvoke::Invoke<IS_SCREEN_FADED_IN, bool>())
				{
					fakeFadeOutTime = GetTickCount64();
					fakeFadeOutLength = ctx.GetArgument<int>(0);
					return;
				}

				handler(ctx);
			});

			auto handlerIs = fx::ScriptEngine::GetNativeHandler(IS_SCREEN_FADING_IN);

			fx::ScriptEngine::RegisterNativeHandler(IS_SCREEN_FADING_IN, [=](fx::ScriptContext& ctx)
			{
				handlerIs(ctx);

				if ((GetTickCount64() - fakeFadeOutTime) < fakeFadeOutLength)
				{
					ctx.SetResult(1);
				}
			});
		}
	});
});

static void UpdateLoadTiming(uint64_t loadTimingIdentity)
{
	if (g_loadProfileConvar->GetValue())
	{
		auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()) - g_loadTimingBase;

		g_loadTiming.insert({ loadTimingIdentity, now });
	}
	else
	{
		auto curTiming = g_loadTiming[loadTimingIdentity];

		if (curTiming.count() != 0 && g_visitedTimings.find(loadTimingIdentity) == g_visitedTimings.end())
		{
			auto frac = (curTiming - g_loadTimingBase).count() / (double)(g_loadTiming[1] - g_loadTimingBase).count();

			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("loadFraction", rapidjson::Value(frac), doc.GetAllocator());

			InvokeNUIScript("loadProgress", doc);
#ifdef GTA_FIVE
			ActivateStatusText(va("Loading game (%.0f%%)", frac * 100.0), 5, 4);
#elif IS_RDR3
			statusText = fmt::format("Loading game ({:.0f}%%)\n", frac * 100.0);
#endif
			OnLookAliveFrame();

			g_visitedTimings.insert(loadTimingIdentity);
		}
	}
}

static bool ShouldSkipLoading()
{
	return !autoShutdownNui || Instance<ICoreGameInit>::Get()->HasVariable("localMode") || Instance<ICoreGameInit>::Get()->HasVariable("storyMode");
}

void LoadsThread::DoRun()
{
	if (ShouldSkipLoading())
	{
		if (doShutdown && !autoShutdownNui)
		{
			// Init the player for RAGE sake
			NativeInvoke::Invoke<SET_ENTITY_COORDS, int>(NativeInvoke::Invoke<PLAYER_PED_ID, int>(), 0.0f, 0.0f, 0.0f, false, false, false, false);

#ifdef IS_RDR3
			fx::ScriptContextBuffer ctx;
			g_origShutdown(ctx);
#endif

			doShutdown = false;
		}

		return;
	}

	if (!sh)
	{
		AttachScriptHandler();

		sh = true;
	}

	if (!GetThread()->GetScriptHandler())
	{
		if (doSetup)
		{
#ifdef GTA_FIVE
			DeactivateStatusText(1);
#elif IS_RDR3
			shouldShowStatusText = false;
#endif
			doSetup = false;
		}

		return;
	}

	if (doShutdown)
	{
#ifdef IS_RDR3
		// We wait because it take some to shutdown loading screen
		fx::ScriptContextBuffer ctx;
		g_origShutdown(ctx);
		if (NativeInvoke::Invoke<IS_LOADING_SCREEN_VISIBLE, bool>())
		{
			return;
		}
#endif

		cam = NativeInvoke::Invoke<CREATE_CAM, int>("DEFAULT_SCRIPTED_CAMERA", true);

		NativeInvoke::Invoke<SET_CAM_COORD, int>(cam, defaultCameraPos.x, defaultCameraPos.y, defaultCameraPos.z);

		NativeInvoke::Invoke<SET_CAM_ROT, int>(cam, defaultCameraRot.x, defaultCameraRot.y, defaultCameraRot.z, 0);

		NativeInvoke::Invoke<SET_CAM_FOV, int>(cam, defaultCameraFov);

		NativeInvoke::Invoke<RENDER_SCRIPT_CAMS, int>(true, false, 0, false, false);
#ifdef GTA_FIVE
		// SHUTDOWN_LOADING_SCREEN
		fx::ScriptContextBuffer ctx;
		g_origShutdown(ctx);
#endif
		NativeInvoke::Invoke<DO_SCREEN_FADE_IN, int>(0);
#ifdef GTA_FIVE
		NativeInvoke::Invoke<SET_WEATHER_TYPE_PERSIST, int>("EXTRASUNNY");
#elif IS_RDR3
		NativeInvoke::Invoke<SET_OVERRIDE_WEATHER, int>("SUNNY");
#endif
		NativeInvoke::Invoke<NETWORK_OVERRIDE_CLOCK_TIME, int>(12, 30, 0);


		NativeInvoke::Invoke<SET_ENTITY_COORDS, int>(NativeInvoke::Invoke<PLAYER_PED_ID, int>(), 0.0f, 0.0f, 0.0f);

		NativeInvoke::Invoke<FREEZE_ENTITY_POSITION, int>(NativeInvoke::Invoke<PLAYER_PED_ID, int>(), true);

		NativeInvoke::Invoke<SET_FOCUS_AREA, int>(defaultCameraPos.x, defaultCameraPos.y, defaultCameraPos.z, 0.0f, 0.0f, 0.0f);

		// do shutdown
		DestroyFrame();

		nui::OverrideFocus(false);

		isShutdown = true;
		doShutdown = false;
	}

	if (isShutdown)
	{
		NativeInvoke::Invoke<HIDE_HUD_AND_RADAR_THIS_FRAME, int>();
	}

	if (doSetup)
	{
		NativeInvoke::Invoke<RENDER_SCRIPT_CAMS, int>(false, false, 0, false, false);

		NativeInvoke::Invoke<DESTROY_CAM, int>(cam, false);

		NativeInvoke::Invoke<CLEAR_FOCUS, int>();
#ifdef GTA_FIVE
		NativeInvoke::Invoke<CLEAR_WEATHER_TYPE_PERSIST, int>();
#elif IS_RDR3
		NativeInvoke::Invoke<CLEAR_OVERRIDE_WEATHER, int>();
#endif

		cam = 0;
#ifdef GTA_FIVE
		DeactivateStatusText(1);
#elif IS_RDR3
		shouldShowStatusText = false;
#endif
		// done
		isShutdown = false;
		doSetup = false;
	}
}

extern void (*g_updateContentArray)(void*);

extern void** g_extraContentManagerLocation;
extern char g_extraContentManagerContentOffset;
extern uint32_t g_mountableContentSize;
extern uint32_t g_mountableContentFlagsOffset;

extern int dlcIdx;
extern int extraContentWrapperIdx;
extern std::map<uint32_t, std::string> g_dlcNameMap;
DLL_IMPORT fwEvent<const std::string&> OnScriptInitStatus;

static uint32_t g_upcomingDlcCallsCount = 0;

static uint32_t EstimateUpcomingDlcCalls()
{
	uint32_t count = 0;

	// This is an atArray<CMountableContent>. But since we don't know structure of CMountableContent
	// in compile time - use char just to be able to use atArray properties.
	atArray<char>* m_content = (atArray<char>*)((uintptr_t)*g_extraContentManagerLocation + g_extraContentManagerContentOffset);

	// We can not use atArray iterator because it relies on size of the stored type.
	for (uint16_t i = 0; i < m_content->GetCount(); ++i)
	{
		void* content = (void*)((uintptr_t)m_content->m_offset + i * g_mountableContentSize);

		bool datFileLoaded = (*(uint32_t*)((uintptr_t)content + g_mountableContentFlagsOffset)) & 1;
		if (!datFileLoaded)
		{
			++count;
		}
	}
	return count;
}

static const char* GetName(const rage::InitFunctionData& data)
{
	if (auto it = g_dlcNameMap.find(data.funcHash); it != g_dlcNameMap.end())
	{
		return it->second.c_str();
	}

	return data.GetName();
}

static InitFunction initFunction([] ()
{
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		rage::scrEngine::CreateThread(loadsThread.GetThread());
	});

	g_loadProfileConvar = std::make_shared<ConVar<bool>>("game_profileLoading", ConVar_UserPref, false);

	OnKillNetworkDone.Connect([] ()
	{
		DestroyFrame();
	});

	rapidjson::Document doc;

	FILE* f = _wfopen(MakeRelativeCitPath(L"citizen/load_profile.json").c_str(), L"rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);

		std::vector<char> data(ftell(f));
		fseek(f, 0, SEEK_SET);

		fread(data.data(), 1, data.size(), f);
		fclose(f);

		doc.Parse(data.data(), data.size());

		if (!doc.HasParseError())
		{
			for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); it++)
			{
				g_loadTiming.insert({ _strtoui64(it->name.GetString(), nullptr, 10), std::chrono::milliseconds(it->value.GetInt()) });
			}
		}
	}

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([] ()
	{
		frameOn = true;

		std::vector<std::string> loadingScreens = { "nui://game/ui/loadscreen/index.html" };

		Instance<fx::ResourceManager>::Get()->ForAllResources([&](fwRefContainer<fx::Resource> resource)
		{
			auto mdComponent = resource->GetComponent<fx::ResourceMetaDataComponent>();
			auto entries = mdComponent->GetEntries("loadscreen");

			if (entries.begin() != entries.end())
			{
				std::string path = entries.begin()->second;

				if (path.find("://") != std::string::npos)
				{
					loadingScreens.push_back(path);
				}
				else
				{
					loadingScreens.push_back("nui://" + resource->GetName() + "/" + path);
				}

				auto entriesTwo = mdComponent->GetEntries("loadscreen_manual_shutdown");
				if (entriesTwo.begin() != entriesTwo.end())
				{
					autoShutdownNui = false;
				}

				static ConVar<bool> uiLoadingCursor("ui_loadingCursor", ConVar_None, false);
				auto useCursor = mdComponent->GetEntries("loadscreen_cursor");
				if (useCursor.begin() != useCursor.end())
				{
					uiLoadingCursor.GetHelper()->SetRawValue(true);
				}
				else
				{
					uiLoadingCursor.GetHelper()->SetRawValue(false);
				}
			}
		});

		g_doDrawBelowLoadingScreens = true;
#ifdef IS_RDR3
		shouldShowStatusText = true;
#endif
		
		auto icgi = Instance<ICoreGameInit>::Get();
		std::string handoverBlob;

		if (icgi->GetData("handoverBlob", &handoverBlob))
		{
			nui::PostRootMessage(fmt::sprintf(R"({ "type": "setHandover", "data": %s })", handoverBlob));
		}

		if (loadingScreens.size() == 1)
		{
			Instance<ICoreGameInit>::Get()->SetVariable("noLoadingScreen");
		}
		else
		{
			Instance<ICoreGameInit>::Get()->ClearVariable("noLoadingScreen");
		}

		nui::CreateFrame("loadingScreen", loadingScreens.back());
		nui::OverrideFocus(true);

		nui::PostRootMessage(R"({ "type": "focusFrame", "frameName": "loadingScreen" })");
	}, 100);

	static bool isGameReload = false;

	rage::OnInitFunctionStart.Connect([] (rage::InitFunctionType type)
	{
		if (type == rage::INIT_AFTER_MAP_LOADED)
		{
			rapidjson::Document doc;
			doc.SetObject();

			InvokeNUIScript("endDataFileEntries", doc);
		}

		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());

			InvokeNUIScript("startInitFunction", doc);
		}

		if (type == rage::INIT_BEFORE_MAP_LOADED)
		{
			if (g_loadProfileConvar->GetValue())
			{
				g_loadTiming.clear();
				g_loadTimingBase = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
			}
			else
			{
				rapidjson::Document doc;
				doc.SetObject();
				doc.AddMember("loadFraction", 0.0, doc.GetAllocator());

				InvokeNUIScript("loadProgress", doc);
#ifdef GTA_FIVE
				DeactivateStatusText(4);
#endif
				g_visitedTimings.clear();
			}
		}
		else if (type == rage::INIT_SESSION)
		{
			if (g_loadProfileConvar->GetValue())
			{
				g_loadTiming[2] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()) - g_loadTimingBase;
			}
			else if (isGameReload)
			{
				rapidjson::Document doc;
				doc.SetObject();
				doc.AddMember("loadFraction", 0.0, doc.GetAllocator());

				InvokeNUIScript("loadProgress", doc);
#ifdef GTA_FIVE
				DeactivateStatusText(4);
#endif
				g_visitedTimings.clear();

				g_loadTimingBase = g_loadTiming[2];
			}
		}
	});

	static auto totalInitFunctionsCount = 0;

	rage::OnInitFunctionStartOrder.Connect([] (rage::InitFunctionType type, int order, int count)
	{
		if (type == rage::INIT_SESSION && order == 3)
		{
			// After disconnect most of the extra content is removed and will be added to the list again only on CExtraContentWrapper::Init call.
			// Update content array explicitly to be able to correctly estimate amount of additional content that will be loaded.
			// This is a noop if the content array is already updated.
			g_updateContentArray(*g_extraContentManagerLocation);
			g_upcomingDlcCallsCount = EstimateUpcomingDlcCalls();
			count += g_upcomingDlcCallsCount;
			// Account for FinalizeLoad log in OnInitFunctionEnd.
			count += 1;

			dlcIdx = 1;
		}

		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("order", order, doc.GetAllocator());
		doc.AddMember("count", count, doc.GetAllocator());

		InvokeNUIScript("startInitFunctionOrder", doc);

		totalInitFunctionsCount = count;
	});

	static auto lastIdx = 0;

	rage::OnInitFunctionInvoking.Connect([] (rage::InitFunctionType type, int idx, rage::InitFunctionData& data)
	{
		if (type == rage::INIT_SESSION && data.initOrder == 3 && data.funcHash == HashString("CExtraContentWrapper"))
		{
			extraContentWrapperIdx = idx;
		}

		if (type == rage::INIT_SESSION && data.initOrder == 3 && idx > extraContentWrapperIdx && data.shutdownOrder != 42)
		{
			idx += g_upcomingDlcCallsCount;
		}

		uint64_t loadTimingIdentity = data.funcHash | ((int64_t)type << 48);

		UpdateLoadTiming(loadTimingIdentity);

		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("name", rapidjson::Value(GetName(data), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("idx", idx + 1, doc.GetAllocator());
		doc.AddMember("count", totalInitFunctionsCount, doc.GetAllocator());


		InvokeNUIScript("initFunctionInvoking", doc);

		lastIdx = idx;
	});

	rage::OnInitFunctionInvoked.Connect([] (rage::InitFunctionType type, const rage::InitFunctionData& data)
	{
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("name", rapidjson::Value(GetName(data), doc.GetAllocator()), doc.GetAllocator());

		InvokeNUIScript("initFunctionInvoked", doc);
	});

	rage::OnInitFunctionEnd.Connect([] (rage::InitFunctionType type)
	{
		if (type == rage::INIT_BEFORE_MAP_LOADED)
		{
			primedMapLoad = true;
		}
		else if (type == rage::INIT_SESSION)
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
			doc.AddMember("name", rapidjson::Value("FinalizeLoad", doc.GetAllocator()), doc.GetAllocator());
			doc.AddMember("idx", lastIdx + 2, doc.GetAllocator());
			doc.AddMember("count", totalInitFunctionsCount, doc.GetAllocator());

			InvokeNUIScript("initFunctionInvoking", doc);

			if (g_loadProfileConvar->GetValue())
			{
				auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()) - g_loadTimingBase;

				g_loadTiming[1] = now;

				rapidjson::Document doc;
				doc.SetObject();

				for (auto& timingEntry : g_loadTiming)
				{
					auto [idx, count] = timingEntry;

					doc.AddMember(rapidjson::Value(fmt::sprintf("%lld", idx).c_str(), doc.GetAllocator()), uint32_t(count.count()), doc.GetAllocator());
				}

				rapidjson::StringBuffer sb;
				rapidjson::Writer w(sb);

				if (doc.Accept(w))
				{
					FILE* f = _wfopen(MakeRelativeCitPath(L"citizen/load_profile.json").c_str(), L"wb");
					
					if (f)
					{
						fwrite(sb.GetString(), 1, sb.GetSize(), f);
						fclose(f);
					}
				}
			}
			else
			{
				rapidjson::Document doc;
				doc.SetObject();
				doc.AddMember("loadFraction", 1.0, doc.GetAllocator());

				InvokeNUIScript("loadProgress", doc);
#ifdef GTA_FIVE
				DeactivateStatusText(4);
#endif
			}

			// for next time
			isGameReload = true;

			if (Instance<ICoreGameInit>::Get()->HasVariable("networkInited"))
			{
				DestroyFrame();
			}

			loadsThread.doShutdown = true;
		}

		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());

			InvokeNUIScript("endInitFunction", doc);
		}
	});

	auto printLog = [](const std::string& message)
	{
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("message", rapidjson::Value(message.c_str(), message.size(), doc.GetAllocator()), doc.GetAllocator());

		InvokeNUIScript("onLogLine", doc);
#ifdef GTA_FIVE
		if (!ShouldSkipLoading())
		{
			ActivateStatusText(message.c_str(), 5, 1);
		}
#elif IS_RDR3
		statusText = message;
#endif
	};
#ifdef GTA_FIVE
	OnHostStateTransition.Connect([printLog] (HostState newState, HostState oldState)
	{
		if (newState == HS_FATAL)
		{
			DestroyFrame();
		}
		else if (newState == HS_JOINING)
		{
			printLog("Entering session");
		}
		else if (newState == HS_WAIT_HOSTING)
		{
			printLog("Setting up game");
		}
		else if (newState == HS_JOIN_FAILURE)
		{
			printLog("Adjusting settings for best experience");
		}
		else if (newState == HS_HOSTING)
		{
			printLog("Initializing session");
		}
		else if (newState == HS_HOSTED || newState == HS_JOINED)
		{
			printLog("Starting game");
		}
	});
#elif IS_RDR3
	OnHostStateTransition.Connect([printLog] (HostState newState, HostState oldState)
	{
		if (newState == SESSION_STATE_ENTER)
		{
			printLog("Entering session");
		}
		else if (newState == SESSION_STATE_START_JOINING)
		{
			printLog("Setting up game");
		}
		else if (newState == SESSION_STATE_6)
		{
			printLog("Adjusting settings for best experience");
		}
		else if (newState == SESSION_STATE_JOINING)
		{
			printLog("Initializing session");
		}
		else if (newState == SESSION_STATE_JOINED)
		{
			printLog("Starting game");
		}
	});
#endif
	OnScriptInitStatus.Connect([printLog](const std::string& text)
	{
		printLog(text);
	});

	OnInstrumentedFunctionCall.Connect([] (int idx, void* origin, void* target)
	{
        static std::set<int> hadIndices;

		if (primedMapLoad)
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("count", CountRelevantDataFileEntries(), doc.GetAllocator());

			InvokeNUIScript("startDataFileEntries", doc);

			primedMapLoad = false;
		}

        if (hadIndices.find(idx) == hadIndices.end())
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("idx", idx, doc.GetAllocator());

			InvokeNUIScript("performMapLoadFunction", doc);

            hadIndices.insert(idx);
		}
	});

	OnReturnDataFileEntry.Connect([] (int type, const char* name)
	{
		// counting 0, 9 and 185 is odd due to multiple counts :/ (+ entries getting added all the time)

#ifdef GTA_FIVE
		static std::set<int> entryTypes = { 0, 9, 185, 3, 4, 20, 21, 28, 45, 48, 49, 51, 52, 53, 54, 55, 56, 59, 66, 71, 72, 73, 75, 76, 77, 84, 89, 97, 98, 99, 100, 106, 107, 112, 133, 184 };
#elif IS_RDR3
		static std::set<int> entryTypes = { 23 };
#endif 
		static thread_local std::set<std::pair<int, std::string>> hadEntries;

		if (entryTypes.find(type) != entryTypes.end())
		{
			bool isNew = false;

			if (hadEntries.find({ type, name }) == hadEntries.end())
			{
				hadEntries.insert({ type, name });

				if (type != 0 && type != 9 && type != 185)
				{
					isNew = true;
				}
			}

			uint64_t loadTimingIdentity = HashString(name) | (16 << 48);

			UpdateLoadTiming(loadTimingIdentity);

			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("type", type, doc.GetAllocator());
			doc.AddMember("isNew", isNew, doc.GetAllocator());
			doc.AddMember("name", rapidjson::Value(name, doc.GetAllocator()), doc.GetAllocator());

			InvokeNUIScript("onDataFileEntry", doc);
		}
	});

	OnPostFrontendRender.Connect([]()
	{
		if (!g_doDrawBelowLoadingScreens)
		{
			return;
		}

		SetTextureGtaIm(rage::grcTextureFactory::GetNoneTexture());

		auto oldRasterizerState = GetRasterizerState();
		SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

		auto oldBlendState = GetBlendState();
		SetBlendState(GetStockStateIdentifier(BlendStateNoBlend));

		auto oldDepthStencilState = GetDepthStencilState();
		SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

		PushDrawBlitImShader();

		rage::grcBegin(4, 4);

		CRect rect(0.0f, 0.0f, 6144.0f, 6144.0f);
		uint32_t color = 0x00000000;

		rage::grcVertex(rect.fX1, rect.fY1, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		rage::grcVertex(rect.fX2, rect.fY1, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		rage::grcVertex(rect.fX1, rect.fY2, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		rage::grcVertex(rect.fX2, rect.fY2, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);

		rage::grcEnd();

		PopDrawBlitImShader();

		SetRasterizerState(oldRasterizerState);

		SetBlendState(oldBlendState);

		SetDepthStencilState(oldDepthStencilState);
	}, INT32_MIN);



#ifdef IS_RDR3
	static bool showBusySpinner = true;
	static ConVar showBusySpinnerConVar("sv_showBusySpinnerOnLoadingScreen", ConVar_Replicated, true, &showBusySpinner);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || (shouldShowStatusText && showBusySpinner);
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!(shouldShowStatusText && showBusySpinner))
		{
			return;
		}

		auto& io = ImGui::GetIO();

		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x + (ImGui::GetMainViewport()->Size.x * 0.825f), ImGui::GetMainViewport()->Pos.y + (ImGui::GetMainViewport()->Size.y * 0.9f)), 0, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("DrawRDRLoadingState", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMouseInputs))
		{
			ImGui::Text(statusText.c_str());
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::End();
	});
#endif
});


