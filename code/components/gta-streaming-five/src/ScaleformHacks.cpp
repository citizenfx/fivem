#include <StdInc.h>
#include <Hooking.h>

#include <nutsnbolts.h>
#include <sfFontStuff.h>
#include <sfDefinitions.h>

#include <CoreConsole.h>
#include <Streaming.h>
#include <CrossBuildRuntime.h>

static hook::cdecl_stub<bool(GFxObjectInterface*, void*, GFxValue*, const char*, int)> __GFxObjectInterface_CreateEmptyMovieClip([]()
{
	return hook::get_pattern("4D 8B E0 4C 8B F9 48 85 DB 75 18 48", -0x25);
});

static hook::cdecl_stub<bool(GFxObjectInterface*, void*, GFxValue::DisplayInfo*)> __GFxObjectInterface_GetDisplayInfo([]()
{
	return hook::get_pattern("83 65 9B 00 83 65 97 00 F2", -0x60);
});

static hook::cdecl_stub<bool(GFxObjectInterface*, void*, const GFxValue::DisplayInfo&)> __GFxObjectInterface_SetDisplayInfo([]()
{
	return hook::get_pattern("48 8B 5A 08 0F 29 70 B8 0F 29 78 A8", -0x1D);
});

static hook::cdecl_stub<void(GFxObjectInterface*, GFxValue*, void*)> __GFxObjectInterface_ObjectRelease([]()
{
	return hook::get_pattern("83 F8 04 74 6B 83 F8 05 74 58", -0x11);
});

static hook::cdecl_stub<bool(GFxObjectInterface*, void*, GFxValue*, const char*, GFxValue*, int, bool)> __GFxObjectInterface_Invoke([]()
{
	return hook::get_pattern("33 DB 49 8B F1 48 8B FA 4C 8B E1 38 5D 7F 74 32", -0x21);
});

static hook::cdecl_stub<bool(GFxObjectInterface*, void*, const char*, GFxValue*, bool)> __GFxObjectInterface_GetMember([]()
{
	return hook::get_pattern("38 9C 24 80 00 00 00 74 56 48 8B 42 08", -0x26);
});

void GFxValue::ReleaseManagedValue()
{
	__GFxObjectInterface_ObjectRelease(pObjectInterface, this, mValue.pData);
	pObjectInterface = nullptr;
}

bool GFxValue::CreateEmptyMovieClip(GFxValue* movieClip, const char* instanceName, int depth)
{
	// CreateEmptyMovieClip will fail if depth >0x7EFFFFFD
	if (depth > 0x7EFFFFFD)
	{
		depth = 0x7EFFFFFD;
	}

	return __GFxObjectInterface_CreateEmptyMovieClip(pObjectInterface, mValue.pData, movieClip, instanceName, depth);
}

bool GFxValue::GetDisplayInfo(GFxValue::DisplayInfo* info)
{
	return __GFxObjectInterface_GetDisplayInfo(pObjectInterface, mValue.pData, info);
}

bool GFxValue::SetDisplayInfo(const GFxValue::DisplayInfo& info)
{
	return __GFxObjectInterface_SetDisplayInfo(pObjectInterface, mValue.pData, info);
}

bool GFxValue::Invoke(const char* name, GFxValue* presult, const GFxValue* pargs, int nargs)
{
	return __GFxObjectInterface_Invoke(pObjectInterface, mValue.pData, presult, name, (GFxValue*)pargs, nargs, IsDisplayObject());
}

bool GFxValue::GetMember(const char* name, GFxValue* pval) const
{
	return __GFxObjectInterface_GetMember(pObjectInterface, mValue.pData, name, pval, IsDisplayObject());
}

static GMemoryHeap** g_gfxMemoryHeap;// = (GFxMemoryHeap**)0x142CBB3E8;

void* GRefCountBase::operator new(size_t size)
{
	//assert(size <= static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
	return (*g_gfxMemoryHeap)->Alloc(static_cast<uint32_t>(size));
}

void GRefCountBase::operator delete(void* ptr)
{
	(*g_gfxMemoryHeap)->Free(ptr);
}

void GRefCountBase::operator delete[](void* ptr)
{
	(*g_gfxMemoryHeap)->Free(ptr);
}

static hook::cdecl_stub<void(GFxValue* value, uint32_t movie)> _getScaleformASRoot([]()
{
	return hook::get_pattern("48 8B D9 83 FA FF 74 23 8B CA", -0xE);
});

static hook::cdecl_stub<GFxMovieRoot*(uint32_t movie)> _getScaleformMovie([]()
{
	return hook::get_call(hook::get_pattern("48 8B D9 83 FA FF 74 23 8B CA", 10));
});

static std::map<int, std::shared_ptr<GFxValue>> g_overlayClips;

class OverlayMethodFunctionHandler : public GFxFunctionHandler
{
public:
	virtual void Call(const Params& params) override
	{
		if (params.ArgCount < 2)
		{
			return;
		}

		int movieId = static_cast<int>(params.pArgs[0].GetNumber());

		auto& clipVal = g_overlayClips[movieId];

		if (clipVal)
		{
			const char* fn = params.pArgs[1].GetString();

			GFxValue pTimeline;

			if (clipVal->GetMember("TIMELINE", &pTimeline))
			{
				pTimeline.Invoke(va("%s", fn), params.pRetVal, &params.pArgs[2], params.ArgCount - 2);
			}
		}
	}
};

static GFxValue overlayRootClip;
static void(*g_origSetupTerritories)();

static GFxValue* g_foregroundOverlay3D;
static uint32_t* g_gfxId;

static void SetupTerritories()
{
	g_origSetupTerritories();
	
	overlayRootClip = {};

	g_foregroundOverlay3D->CreateEmptyMovieClip(&overlayRootClip, "asTestClip3D", -1);

	auto movie = _getScaleformMovie(*g_gfxId);

	auto fh = new OverlayMethodFunctionHandler();

	GFxValue overlayMethodValue;
	movie->CreateFunction(&overlayMethodValue, fh);

	movie->SetVariable("TIMELINE.OVERLAY_METHOD", overlayMethodValue, 1);
}

struct MinimapOverlayLoadRequest
{
	int sfwId;
	int depth;
};

static std::map<std::string, MinimapOverlayLoadRequest> g_minimapOverlayLoadQueue;
static std::set<int> g_minimapOverlayRemoveQueue;
static int g_minimapOverlaySwfId;

static hook::cdecl_stub<void(const char*, bool)> _gfxPushString([]()
{
	return hook::get_call(hook::get_pattern("EB 0C 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 33", 9));
});

static hook::cdecl_stub<void(int)> _gfxPushInt([]()
{
	return hook::get_pattern("48 63 D0 48 69 D2 ? 01 00 00 80 3C 0A FF 74 41", -0x14);
});

static hook::cdecl_stub<bool(uint32_t, int, const char*, int, int)> _setupGfxCall([]()
{
	return hook::get_pattern("8B F9 85 C9 78 73 0F", -0x1C);
});

namespace sf
{
	int AddMinimapOverlay(const std::string& swfName, int depth)
	{
		auto id = ++g_minimapOverlaySwfId;

		g_minimapOverlayLoadQueue.insert({ swfName, { id, depth } });

		return id;
	}

	void RemoveMinimapOverlay(int swfId)
	{
		g_minimapOverlayRemoveQueue.insert(swfId);
	}

	bool HasMinimapLoaded(int swfId)
	{
		return (g_overlayClips.find(swfId) != g_overlayClips.end());
	}

	bool CallMinimapOverlay(int minimap, const std::string& functionName)
	{
		if (_setupGfxCall(*g_gfxId, 1, "OVERLAY_METHOD", -1, -1))
		{
			_gfxPushInt(minimap);
			_gfxPushString(functionName.c_str(), true);
			return true;
		}

		return false;
	}

	void SetMinimapOverlayDisplay(int minimap, float x, float y, float xScale, float yScale, float alpha)
	{
		auto& clip = g_overlayClips[minimap];

		if (clip)
		{
			GFxValue::DisplayInfo dispInfo;
			dispInfo.VarsSet = 0x1 | 0x2 | 0x8 | 0x10 | 0x20; // x, y, xscale, yscale, alpha
			dispInfo.X = x;
			dispInfo.Y = y;
			dispInfo.XScale = xScale;
			dispInfo.YScale = yScale;
			dispInfo.Alpha = alpha;

			clip->SetDisplayInfo(dispInfo);
		}
	}
}

namespace sf::logging
{
	const char* const g_scriptTypes[] = {
		"GENERIC_TYPE",
		"SCRIPT_TYPE",
		"HUD_TYPE",
		"MINIMAP_TYPE",
		"WEB_TYPE",
		"CUTSCENE_TYPE",
		"PAUSE_TYPE",
		"STORE",
		"GAMESTREAM",
		"SF_BASE_CLASS_VIDEO_EDITOR",
		"SF_BASE_CLASS_MOUSE",
		"SF_BASE_CLASS_TEXT_INPUT",
	};

	static bool g_scaleformDebugLog = 0;
	static ConVar<bool> g_scaleformDebugLogVar("game_enableScaleformDebugLog", ConVar_Archive | ConVar_UserPref, false, &g_scaleformDebugLog);

	static void (*g_origSfCallGameFromFlash)(void* _this, void* movieView, const char* methodName, const GFxValue* args, uint32_t argCount);
	static void SfCallGameFromFlash(void* _this, void* movieView, const char* methodName, const GFxValue* args, uint32_t argCount)
	{
		// only print debug log calls
		if ((g_scaleformDebugLog && argCount >= 2 && strcmp(methodName, "DEBUG_LOG") == 0))
		{
			if (args[0].GetType() == GFxValue::ValueType::VT_Number)
			{
				if (args[1].GetType() == GFxValue::ValueType::VT_String)
				{
					uint32_t scriptType = (int32_t)args[0].GetNumber(); // uint32 requires only 1 check
					trace("[%s] %s\n", scriptType < std::size(g_scriptTypes) ? g_scriptTypes[scriptType] : "UNKNOWN", args[1].GetString());
				}
			}
		}

		g_origSfCallGameFromFlash(_this, movieView, methodName, args, argCount);
	}
}

static HookFunction hookFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		std::set<std::string> toRemoveFromMinimapOverlayLoadQueue;

		auto cstreaming = streaming::Manager::GetInstance();

		for (const auto& [gfxFileName, request] : g_minimapOverlayLoadQueue)
		{
			auto swf = std::make_shared<GFxValue>();

			auto instanceName = va("id%d", request.sfwId);

			overlayRootClip.CreateEmptyMovieClip(swf.get(), instanceName, request.depth);

			GFxValue result;
			GFxValue args(gfxFileName.c_str());

			swf->Invoke("loadMovie", &result, &args, 1);

			g_overlayClips[request.sfwId] = swf;

			toRemoveFromMinimapOverlayLoadQueue.insert(gfxFileName);
		}

		for (const auto& gfxFileName : toRemoveFromMinimapOverlayLoadQueue)
		{
			g_minimapOverlayLoadQueue.erase(gfxFileName);
		}

		for (int swfId : g_minimapOverlayRemoveQueue)
		{
			if (auto swf = g_overlayClips[swfId]; swf)
			{
				swf->Invoke("removeMovieClip", nullptr, nullptr, 0);
			}
		}

		g_minimapOverlayRemoveQueue.clear();
	});

	{
		auto location = hook::get_pattern<char>("74 3B E8 ? ? ? ? 33 C9 E8", 0x31);

		g_foregroundOverlay3D = hook::get_address<decltype(g_foregroundOverlay3D)>(hook::get_call(location) + (xbr::IsGameBuildOrGreater<2372>() ? 0x18 : 0x1C));

		hook::set_call(&g_origSetupTerritories, location);
		hook::call(location, SetupTerritories);
	}

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		g_gfxId = hook::get_address<uint32_t*>(hook::get_pattern("C6 45 F0 01 48 89 4D D0 48 8D 4D", 0x17));
	}
	else
	{
		g_gfxId = hook::get_address<uint32_t*>(hook::get_pattern("66 C7 45 E4 01 01 E8", 0x2A));
	}

	g_gfxMemoryHeap = hook::get_address<GMemoryHeap**>(hook::get_pattern("F0 FF 4A 08 75 0E 48 8B 0D", 9));


	// scaleform -> game call hook, for debug logging
	{
		void** sfCallGameFromFlashVTable = hook::get_address<void**>(hook::get_pattern<char>("48 8D 0D ? ? ? ? C7 40 ? ? ? ? ? 48 89 08 89 68 08", 3));
		hook::put(&sf::logging::g_origSfCallGameFromFlash, sfCallGameFromFlashVTable[1]);
		hook::put(&sfCallGameFromFlashVTable[1], &sf::logging::SfCallGameFromFlash);
	}
});
