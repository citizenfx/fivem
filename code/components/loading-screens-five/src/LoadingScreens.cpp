#include <StdInc.h>
#include <CefOverlay.h>

#include <ICoreGameInit.h>

#include <Resource.h>

#include <gameSkeleton.h>

#include <HostSystem.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>

#include <ScriptEngine.h>

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

static HookFunction hookFunction([]()
{
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		static bool endedLoadingScreens = false;
		static bool autoShutdownNui = true;

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

			fx::ScriptEngine::RegisterNativeHandler("SET_MANUAL_SHUTDOWN_LOADING_SCREEN_NUI", [](fx::ScriptContext& ctx)
			{
				autoShutdownNui = !ctx.GetArgument<bool>(0);
			});

			fx::ScriptEngine::RegisterNativeHandler("SHUTDOWN_LOADING_SCREEN_NUI", [=](fx::ScriptContext& ctx)
			{
				endLoadingScreens();
			});

			fx::ScriptEngine::RegisterNativeHandler(0x078EBE9809CCD637, [=](fx::ScriptContext& ctx)
			{
				(*handler)(ctx);

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

extern int dlcIdx;

static InitFunction initFunction([] ()
{
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
		}
	});

	OnHostStateTransition.Connect([] (HostState newState, HostState oldState)
	{
		auto printLog = [] (const std::string& message)
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("message", rapidjson::Value(message.c_str(), message.size(), doc.GetAllocator()), doc.GetAllocator());

			InvokeNUIScript("onLogLine", doc);
		};

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
