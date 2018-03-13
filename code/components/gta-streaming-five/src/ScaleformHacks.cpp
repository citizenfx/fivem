#include <StdInc.h>
#include <Hooking.h>

#include <nutsnbolts.h>
#include <sfFontStuff.h>

#include <Streaming.h>

struct GFxObjectInterface
{

};

struct GFxValue;

static hook::cdecl_stub<bool(GFxObjectInterface*, void*, GFxValue*, const char*)> __GFxObjectInterface_CreateEmptyMovieClip([]()
{
	return hook::get_pattern("4D 8B E0 4C 8B F9 48 85 DB 75 18 48", -0x25);
});

struct GFxDisplayInfo
{
	double X;
	double Y;
	double Rotation;
	double XScale;
	double YScale;
	double Alpha;
	bool Visible;
	double Z;
	double XRotation;
	double YRotation;
	double ZScale;
	double FOV;
	float ViewMatrix3D[4][4];
	float ProjectionMatrix3D[4][4];
	uint8_t VarsSet;
};

static hook::cdecl_stub<bool(GFxObjectInterface*, void*, const GFxDisplayInfo&)> __GFxObjectInterface_SetDisplayInfo([]()
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

struct GFxValue
{
	enum ValueTypeControl
	{
		VTC_ConvertBit = 0x80,
		VTC_ManagedBit = 0x40,

		VTC_TypeMask = VTC_ConvertBit | 0x0F,
	};

	enum ValueType
	{
		VT_Undefined = 0x00,
		VT_Null = 0x01,
		VT_Boolean = 0x02,
		VT_Number = 0x03,
		VT_String = 0x04,
		VT_StringW = 0x05,
		VT_Object = 0x06,
		VT_Array = 0x07,
		VT_DisplayObject = 0x08,

		VT_ConvertBoolean = VTC_ConvertBit | VT_Boolean,
		VT_ConvertNumber = VTC_ConvertBit | VT_Number,
		VT_ConvertString = VTC_ConvertBit | VT_String,
		VT_ConvertStringW = VTC_ConvertBit | VT_StringW
	};

	union ValueUnion
	{
		double          NValue;
		bool            BValue;
		const char*     pString;
		const char**    pStringManaged;
		const wchar_t*  pStringW;
		void*           pData;
	};

	inline GFxValue()
	{
		pObjectInterface = nullptr;
		Type = VT_Undefined;
		mValue.pData = nullptr;
	}

	inline GFxValue(const char* str)
	{
		pObjectInterface = nullptr;
		Type = VT_String;
		mValue.pString = str;
	}

	inline ~GFxValue()
	{
		if (Type & VTC_ManagedBit)
		{
			ReleaseManagedValue();
		}
	}

	inline bool IsDisplayObject() const
	{
		return (Type & VTC_TypeMask) == VT_DisplayObject;
	}

	inline const char* GetString() const
	{
		assert((Type & VTC_TypeMask) == VT_String);
		return (Type & VTC_ManagedBit) ? *mValue.pStringManaged : mValue.pString;
	}

	GFxObjectInterface* pObjectInterface;
	ValueType Type;
	ValueUnion mValue;

	inline void ReleaseManagedValue()
	{
		__GFxObjectInterface_ObjectRelease(pObjectInterface, this, mValue.pData);
		pObjectInterface = nullptr;
	}

	inline bool CreateEmptyMovieClip(GFxValue* movieClip, const char* instanceName)
	{
		return __GFxObjectInterface_CreateEmptyMovieClip(pObjectInterface, mValue.pData, movieClip, instanceName);
	}

	inline bool SetDisplayInfo(const GFxDisplayInfo& info)
	{
		return __GFxObjectInterface_SetDisplayInfo(pObjectInterface, mValue.pData, info);
	}

	inline bool Invoke(const char* name, GFxValue* presult, const GFxValue* pargs, int nargs)
	{
		return __GFxObjectInterface_Invoke(pObjectInterface, mValue.pData, presult, name, (GFxValue*)pargs, nargs, IsDisplayObject());
	}

	inline bool GetMember(const char* name, GFxValue* pval) const
	{
		return __GFxObjectInterface_GetMember(pObjectInterface, mValue.pData, name, pval, IsDisplayObject());
	}
};

class GFxMemoryHeap
{
public:
	virtual void m_00() = 0;
	virtual void m_08() = 0;
	virtual void m_10() = 0;
	virtual void m_18() = 0;
	virtual void m_20() = 0;
	virtual void m_28() = 0;
	virtual void m_30() = 0;
	virtual void m_38() = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void* Alloc(uint32_t size) = 0;
	virtual void m_58() = 0;
	virtual void Free(void* memory) = 0;
};

static GFxMemoryHeap** g_gfxMemoryHeap;// = (GFxMemoryHeap**)0x142CBB3E8;

class GFxRefCountBase
{
private:
	volatile int refCount;

public:
	virtual ~GFxRefCountBase() = default;

	inline void* operator new(size_t size)
	{
		return (*g_gfxMemoryHeap)->Alloc(size);
	}

	inline void operator delete(void* ptr)
	{
		(*g_gfxMemoryHeap)->Free(ptr);
	}

	inline void operator delete[](void* ptr)
	{
		(*g_gfxMemoryHeap)->Free(ptr);
	}
};

class GFxFunctionHandler : public GFxRefCountBase
{
public:
	struct Params
	{
		GFxValue* pRetVal;
		void* pMovie;
		GFxValue* pThis;
		GFxValue* pArgsWithThisRef;
		GFxValue* pArgs;
		uint32_t ArgCount;
		void* pUserData;
	};

	virtual ~GFxFunctionHandler() {}

	virtual void Call(const Params& params) = 0;
};

class GFxMovieRoot
{
public:
	virtual void m_00() = 0;
	virtual void m_08() = 0;
	virtual void m_10() = 0;
	virtual void m_18() = 0;
	virtual void m_20() = 0;
	virtual void m_28() = 0;
	virtual void m_30() = 0;
	virtual void m_38() = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void m_50() = 0;
	virtual void m_58() = 0;
	virtual void m_60() = 0;
	virtual void m_68() = 0;
	virtual void m_70() = 0;
	virtual void CreateFunction(GFxValue* value, GFxFunctionHandler* pfc, void* puserData = nullptr) = 0;
	virtual void SetVariable(const char* path, const GFxValue& value, int type) = 0;
};

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

		int movieId = params.pArgs[0].mValue.NValue;

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

	g_foregroundOverlay3D->CreateEmptyMovieClip(&overlayRootClip, "asTestClip3D");

	auto movie = _getScaleformMovie(*g_gfxId);

	auto fh = new OverlayMethodFunctionHandler();

	GFxValue overlayMethodValue;
	movie->CreateFunction(&overlayMethodValue, fh);

	movie->SetVariable("TIMELINE.OVERLAY_METHOD", overlayMethodValue, 1);
}

static std::map<std::string, int> g_swfLoadQueue;
static std::set<int> g_swfRemoveQueue;
static int g_swfId;

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
	int AddMinimapOverlay(const std::string& swfName)
	{
		auto id = ++g_swfId;
		g_swfLoadQueue.insert({ swfName, id });

		return id;
	}

	void RemoveMinimapOverlay(int swfId)
	{
		g_swfRemoveQueue.insert(swfId);
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
			GFxDisplayInfo dispInfo;
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

static HookFunction hookFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		std::set<std::string> toRemove;

		auto cstreaming = streaming::Manager::GetInstance();

		for (const auto& swf : g_swfLoadQueue)
		{
			auto val = std::make_shared<GFxValue>();
			overlayRootClip.CreateEmptyMovieClip(val.get(), va("id%d", swf.second));

			GFxValue rv;

			GFxValue args(swf.first.c_str());
			val->Invoke("loadMovie", &rv, &args, 1);

			g_overlayClips[swf.second] = val;

			toRemove.insert(swf.first);
		}

		for (const auto& swf : toRemove)
		{
			g_swfLoadQueue.erase(swf);
		}

		for (int swfKey : g_swfRemoveQueue)
		{
			auto swf = g_overlayClips[swfKey];

			if (swf)
			{
				swf->Invoke("removeMovieClip", nullptr, nullptr, 0);
			}
		}

		g_swfRemoveQueue.clear();
	});

	{
		auto location = hook::get_pattern<char>("74 3B E8 ? ? ? ? 33 C9 E8", 0x31);

		g_foregroundOverlay3D = hook::get_address<decltype(g_foregroundOverlay3D)>(hook::get_call(location) + 0x1C);

		hook::set_call(&g_origSetupTerritories, location);
		hook::call(location, SetupTerritories);
	}

	g_gfxId = hook::get_address<uint32_t*>(hook::get_pattern("66 C7 45 E4 01 01 E8", 0x2A));
	g_gfxMemoryHeap = hook::get_address<GFxMemoryHeap**>(hook::get_pattern("F0 FF 4A 08 75 0E 48 8B 0D", 9));
});
