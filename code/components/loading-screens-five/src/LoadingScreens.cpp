#include <StdInc.h>
#include <CefOverlay.h>

#include <ICoreGameInit.h>

#include <Hooking.h>

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

#include <Rect.h>
#include <DrawCommands.h>

#include <Error.h>

static std::shared_ptr<ConVar<bool>> g_loadProfileConvar;
static std::map<uint64_t, std::chrono::milliseconds> g_loadTiming;
static std::chrono::milliseconds g_loadTimingBase;
static std::set<uint64_t> g_visitedTimings;

// 1365
// 1493
// 1604
#define NUM_DLC_CALLS 32

using fx::Resource;

static bool g_doDrawBelowLoadingScreens;
static bool frameOn = false;
static bool primedMapLoad = false;

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

class LoadsThread : public GtaThread
{
public:
	virtual void DoRun() override;

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) override
	{
		doSetup = false;
		doShutdown = false;
		isShutdown = false;
		sh = false;

		return GtaThread::Reset(scriptHash, pArgs, argCount);
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
			// override SHUTDOWN_LOADING_SCREEN
			auto handler = fx::ScriptEngine::GetNativeHandler(0x078EBE9809CCD637);

			if (!handler)
			{
				FatalError("Couldn't find SHUTDOWN_LOADING_SCREEN to hook!");
			}

			g_origShutdown = *handler;

			fx::ScriptEngine::RegisterNativeHandler("SET_MANUAL_SHUTDOWN_LOADING_SCREEN_NUI", [](fx::ScriptContext& ctx)
			{
				//autoShutdownNui = !ctx.GetArgument<bool>(0);

				trace("SET_MANUAL_SHUTDOWN_LOADING_SCREEN_NUI is not supported at this time. Use `loadscreen_manual_shutdown 'yes'` in your fxmanifest.lua instead.\n");
			});

			fx::ScriptEngine::RegisterNativeHandler("SHUTDOWN_LOADING_SCREEN_NUI", [=](fx::ScriptContext& ctx)
			{
				loadsThread.doSetup = true;

				endLoadingScreens();
			});

			fx::ScriptEngine::RegisterNativeHandler(0x078EBE9809CCD637, [=](fx::ScriptContext& ctx)
			{
				(*handler)(ctx);

				loadsThread.doSetup = true;

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

		{
			// override LOAD_ALL_OBJECTS_NOW
			auto handler = fx::ScriptEngine::GetNativeHandler(0xBD6E84632DD4CB3F);

			if (!handler)
			{
				FatalError("Couldn't find LOAD_ALL_OBJECTS_NOW to hook!");
			}

			fx::ScriptEngine::RegisterNativeHandler(0xBD6E84632DD4CB3F, [=](fx::ScriptContext& ctx)
			{
				if (!endedLoadingScreens)
				{
					trace("Skipping LOAD_ALL_OBJECTS_NOW as loading screens haven't ended yet!\n");
					return;
				}

				(*handler)(ctx);
			});
		}

		{
			static uint64_t fakeFadeOutTime;
			static int fakeFadeOutLength;

			// override DO_SCREEN_FADE_OUT
			auto handler = fx::ScriptEngine::GetNativeHandler(0x891B5B39AC6302AF);

			fx::ScriptEngine::RegisterNativeHandler(0x891B5B39AC6302AF, [=](fx::ScriptContext& ctx)
			{
				// reset so a spawnmanager-style wait for collision will be fine
				if (loadsThread.isShutdown)
				{
					// CLEAR_FOCUS
					NativeInvoke::Invoke<0x31B73D1EA9F01DA2, int>();
				}

				// IS_SCREEN_FADED_OUT
				if (NativeInvoke::Invoke<0xB16FCE9DDC7BA182, bool>())
				{
					fakeFadeOutTime = GetTickCount64();
					fakeFadeOutLength = ctx.GetArgument<int>(0);
					return;
				}

				(*handler)(ctx);
			});

			// override IS_SCREEN_FADING_OUT
			auto handlerIs = fx::ScriptEngine::GetNativeHandler(0x797AC7CB535BA28F);

			fx::ScriptEngine::RegisterNativeHandler(0x797AC7CB535BA28F, [=](fx::ScriptContext& ctx)
			{
				(*handlerIs)(ctx);

				if ((GetTickCount64() - fakeFadeOutTime) < fakeFadeOutLength)
				{
					ctx.SetResult(1);
				}
			});
		}

		{
			static uint64_t fakeFadeOutTime;
			static int fakeFadeOutLength;

			// override DO_SCREEN_FADE_IN
			auto handler = fx::ScriptEngine::GetNativeHandler(0xD4E8E24955024033);

			fx::ScriptEngine::RegisterNativeHandler(0xD4E8E24955024033, [=](fx::ScriptContext& ctx)
			{
				// IS_SCREEN_FADED_IN
				if (NativeInvoke::Invoke<0x5A859503B0C08678, bool>())
				{
					fakeFadeOutTime = GetTickCount64();
					fakeFadeOutLength = ctx.GetArgument<int>(0);
					return;
				}

				(*handler)(ctx);
			});

			// override IS_SCREEN_FADING_IN
			auto handlerIs = fx::ScriptEngine::GetNativeHandler(0x5C544BC6C57AC575);

			fx::ScriptEngine::RegisterNativeHandler(0x5C544BC6C57AC575, [=](fx::ScriptContext& ctx)
			{
				(*handlerIs)(ctx);

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
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("loadFraction", rapidjson::Value((curTiming - g_loadTimingBase).count() / (double)(g_loadTiming[1] - g_loadTimingBase).count()), doc.GetAllocator());

			InvokeNUIScript("loadProgress", doc);

			g_visitedTimings.insert(loadTimingIdentity);
		}
	}
}

void LoadsThread::DoRun()
{
	if (!autoShutdownNui)
	{
		return;
	}

	if (!sh)
	{
		CGameScriptHandlerMgr::GetInstance()->AttachScript(this);

		sh = true;
	}

	if (!GetScriptHandler())
	{
		if (doSetup)
		{
			// 1604
			((void(*)(int))hook::get_adjusted(0x1401C3438))(1);

			doSetup = false;
		}

		return;
	}

	if (doShutdown)
	{
		// CREATE_CAM
		cam = NativeInvoke::Invoke<0xC3981DCE61D9E13F, int>("DEFAULT_SCRIPTED_CAMERA", true);

		// SET_CAM_COORD
		NativeInvoke::Invoke<0x4D41783FB745E42E, int>(cam, -2153.641f, 4597.957f, 116.662f);

		// SET_CAM_ROT
		NativeInvoke::Invoke<0x85973643155D0B07, int>(cam, -8.601f, 0.0f, 253.026f, 0);

		// SET_CAM_FOV
		NativeInvoke::Invoke<0xB13C14F66A00D047, int>(cam, 45.0f);

		// RENDER_SCRIPT_CAMS
		NativeInvoke::Invoke<0x07E5B515DB0636FC, int>(true, false, 0, false, false);

		// LOAD_SCENE(?)
		NativeInvoke::Invoke<0x4448EB75B4904BDB, int>(-2153.641f, 4597.957f, 116.662f);

		// SHUTDOWN_LOADING_SCREEN
		fx::ScriptContextBuffer ctx;
		g_origShutdown(ctx);

		// DO_SCREEN_FADE_IN(0)
		NativeInvoke::Invoke<0xD4E8E24955024033, int>(0);

		// SET_WEATHER_TYPE_PERSIST
		NativeInvoke::Invoke<0xED712CA327900C8A, int>("EXTRASUNNY");

		// NETWORK_OVERRIDE_CLOCK_TIME
		NativeInvoke::Invoke<0xE679E3E06E363892, int>(12, 30, 0);

		// SET_ENTITY_COORDS so we can start streaming
		NativeInvoke::Invoke<0x06843DA7060A026B, int>(NativeInvoke::Invoke<0xD80958FC74E988A6, int>(), 0.0f, 0.0f, 0.0f);

		// FREEZE_ENTITY_POSITION
		NativeInvoke::Invoke<0x428CA6DBD1094446, int>(NativeInvoke::Invoke<0xD80958FC74E988A6, int>(), true);

		// focus coords (_SET_FOCUS_AREA)
		NativeInvoke::Invoke<0xBB7454BAFF08FE25, int>(-2153.641f, 4597.957f, 116.662f, 0.0f, 0.0f, 0.0f);

		// do shutdown
		DestroyFrame();

		nui::OverrideFocus(false);

		isShutdown = true;
		doShutdown = false;
	}

	if (isShutdown)
	{
		// HIDE_HUD_AND_RADAR_THIS_FRAME
		NativeInvoke::Invoke<0x719FF505F097FD20, int>();
	}

	if (doSetup)
	{
		// RENDER_SCRIPT_CAMS
		NativeInvoke::Invoke<0x07E5B515DB0636FC, int>(false, false, 0, false, false);

		// DESTROY_CAM
		NativeInvoke::Invoke<0x865908C81A2C22E9, int>(cam, false);

		// CLEAR_FOCUS
		NativeInvoke::Invoke<0x31B73D1EA9F01DA2, int>();

		// CLEAR_WEATHER_TYPE_PERSIST
		NativeInvoke::Invoke<0xCCC39339BEF76CF5, int>();

		cam = 0;

		// 1604
		((void(*)(int))hook::get_adjusted(0x1401C3438))(1);

		// done
		isShutdown = false;
		doSetup = false;
	}
}

extern int dlcIdx;
DLL_IMPORT fwEvent<const std::string&> OnScriptInitStatus;

static InitFunction initFunction([] ()
{
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		rage::scrEngine::CreateThread(&loadsThread);
	});

	g_loadProfileConvar = std::make_shared<ConVar<bool>>("game_profileLoading", ConVar_Archive, false);

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
			}
		});

		g_doDrawBelowLoadingScreens = true;

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

				g_visitedTimings.clear();

				g_loadTimingBase = g_loadTiming[2];
			}
		}
	});

	rage::OnInitFunctionStartOrder.Connect([] (rage::InitFunctionType type, int order, int count)
	{
		if (type == rage::INIT_SESSION && order == 3)
		{
			count += NUM_DLC_CALLS;

			dlcIdx = 0;
		}

		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("order", order, doc.GetAllocator());
		doc.AddMember("count", count, doc.GetAllocator());

		InvokeNUIScript("startInitFunctionOrder", doc);
	});

	rage::OnInitFunctionInvoking.Connect([] (rage::InitFunctionType type, int idx, rage::InitFunctionData& data)
	{
		if (type == rage::INIT_SESSION && data.initOrder == 3 && idx >= 15 && data.shutdownOrder != 42)
		{
			idx += NUM_DLC_CALLS;
		}

		uint64_t loadTimingIdentity = data.funcHash | ((int64_t)type << 48);

		UpdateLoadTiming(loadTimingIdentity);

		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("name", rapidjson::Value(data.GetName(), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("idx", idx, doc.GetAllocator());

		InvokeNUIScript("initFunctionInvoking", doc);
	});

	rage::OnInitFunctionInvoked.Connect([] (rage::InitFunctionType type, const rage::InitFunctionData& data)
	{
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("name", rapidjson::Value(data.GetName(), doc.GetAllocator()), doc.GetAllocator());

		InvokeNUIScript("initFunctionInvoked", doc);
	});

	rage::OnInitFunctionEnd.Connect([] (rage::InitFunctionType type)
	{
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());

			InvokeNUIScript("endInitFunction", doc);
		}

		if (type == rage::INIT_BEFORE_MAP_LOADED)
		{
			primedMapLoad = true;
		}
		else if (type == rage::INIT_SESSION)
		{
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
			}

			// for next time
			isGameReload = true;

			if (Instance<ICoreGameInit>::Get()->HasVariable("networkInited"))
			{
				DestroyFrame();
			}

			loadsThread.doShutdown = true;
		}
	});

	auto printLog = [](const std::string& message)
	{
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("message", rapidjson::Value(message.c_str(), message.size(), doc.GetAllocator()), doc.GetAllocator());

		InvokeNUIScript("onLogLine", doc);

		if (autoShutdownNui)
		{
			// 1604
			((void(*)(const char*, int, int))hook::get_adjusted(0x1401C3578))(message.c_str(), 5, 1);
		}
	};

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

		static std::set<int> entryTypes = { 0, 9, 185, 3, 4, 20, 21, 28, 45, 48, 49, 51, 52, 53, 54, 55, 56, 59, 66, 71, 72, 73, 75, 76, 77, 84, 89, 97, 98, 99, 100, 106, 107, 112, 133, 184 };
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
});
