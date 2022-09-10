#include <StdInc.h>

#include <Hooking.h>

#include <CoreConsole.h>
#include <DrawCommands.h>

#include <MinHook.h>
#include <CustomText.h>

#include <Streaming.h>

#include <dxgi1_4.h>
#include <wrl.h>
#include <CrossBuildRuntime.h>

namespace WRL = Microsoft::WRL;

constexpr uint64_t MB = 1024 * 1024;

// use 1000 for one so we catch 'hardware reserved' memory as well
constexpr uint64_t GB = 1000 * MB;

static int extRamMode = 0;
static uint64_t* g_vramLocation;

static int* _budgetScale;

static auto& GetBudgetVar()
{
	static ConVar<int> var("vid_budgetScale", ConVar_Archive, 0, _budgetScale);
	return var;
}

static void SetGamePhysicalBudget(uint64_t budget)
{
	static uint64_t baseBudget;

	if (budget == 0)
	{
		budget = baseBudget;
	}
	else
	{
		baseBudget = budget;
	}

	float multiplier = (GetBudgetVar().GetValue() / 8.0f) + 1.0f;

	// externally, there are 4 texture settings:
	// 0: 'normal'
	// 1: 'high'
	// 2: 'very high'
	// 3: no override
	//
	// however, these are implemented by means of an internal flag which is the amount of mips to cut off, as follows:
	// 0: 'low' (non-existent setting, cut off 2 mips)
	// 1: 'normal' (cut off 1 mip)
	// 2: 'high' (cut off no mips)
	// x: 'very high' (cut off no mips, and allow +hi TXDs)
	//
	// the issue is R* seems to have thought in a few cases that the internal texture setting flag mapped to normal, high, very high,
	// and not unused, normal, high/very high, where 'very high' is just 'high' with the +hi loading flag enabled.
	//
	// instead, we'll just keep high and very high the same, but move the 'low' flag to 'normal' and lower the 'low' size some more in case
	// anyone feels like enabling the real 'low'
	for (int i = 0; i < 80; i += 4)
	{
		g_vramLocation[i + 3] = budget * multiplier;
		g_vramLocation[i + 2] = budget * multiplier;
		g_vramLocation[i + 1] = (budget * multiplier) / 1.5;
		g_vramLocation[i] = (budget * multiplier) / 2;
	}
}

static hook::cdecl_stub<void(bool)> _updateVideoMemoryBar([]()
{
	return hook::get_pattern("BE 06 00 00 00 8B CE E8", -0x6D);
});

static void bigUpdate(int who, int what)
{
	GetBudgetVar().GetHelper()->SetRawValue(what);
	SetGamePhysicalBudget(0);

	_updateVideoMemoryBar(0);
}

static uint64_t (*g_origSettingsVramTex)(void* self, int quality, void* settings);
static uint64_t SettingsVramTex(void* self, int quality, void* settings)
{
	float multiplier = (GetBudgetVar().GetValue() / 8.0f) + 1.0f;
	g_origSettingsVramTex(self, quality, settings);

	// 1 GB is the approximate difference between default 'fake settings' amount and our 3 GB assumption
	return g_vramLocation[quality + 1] - (1 * GB);
}

static bool g_localContention = false;
static bool g_nonLocalContention = false;

static void UpdateMemoryPressure()
{
	auto device = GetD3D11Device();
	WRL::ComPtr<IDXGIDevice> dxgiDevice;
	if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxgiDevice))))
	{
		return;
	}

	WRL::ComPtr<IDXGIAdapter> adapter;
	if (FAILED(dxgiDevice->GetAdapter(&adapter)))
	{
		return;
	}

	WRL::ComPtr<IDXGIAdapter3> adapter3;

	if (FAILED(adapter.As(&adapter3)))
	{
		return;
	}

	DXGI_QUERY_VIDEO_MEMORY_INFO vmiLocal;
	if (FAILED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vmiLocal)))
	{
		return;
	}

	DXGI_QUERY_VIDEO_MEMORY_INFO vmiNonLocal;
	if (FAILED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &vmiNonLocal)))
	{
		return;
	}

	g_localContention = (vmiLocal.Budget > (1 * GB)) && (int64_t(vmiLocal.Budget) - int64_t(vmiLocal.CurrentUsage)) < (200 * MB);
	g_nonLocalContention = (vmiNonLocal.Budget > (1 * GB)) && ((int64_t(vmiNonLocal.Budget) - int64_t(vmiNonLocal.CurrentUsage)) < (200 * MB));
}

static uint64_t (*g_origGetAvailableMemoryForStreamer)(void* self);
static uint64_t _getAvailableMemoryForStreamer(void* self)
{
	if (auto strMgr = streaming::Manager::GetInstance())
	{
		// loaded list length (max is ~32k)
		if (*(uint32_t*)((char*)strMgr + 264) >= 30000)
		{
			return 0;
		}
	}

	// if there's memory pressure, don't.
	if (g_localContention || g_nonLocalContention)
	{
		// update so we can instantly tell if freeing helped
		UpdateMemoryPressure();

		return 0;
	}

	return g_origGetAvailableMemoryForStreamer(self);
}

static HookFunction hookFunction([]()
{
	game::AddCustomText("GFX_BUDGET", "Extended Texture Budget");

	{
		auto isStereo = (int*)hook::AllocateStubMemory(4);
		_budgetScale = (int*)hook::AllocateStubMemory(4);
		*_budgetScale = 0;
		*isStereo = 1;

		auto location = xbr::IsGameBuildOrGreater<2545>() ? hook::get_pattern<char>("84 C0 0F 84 4B 01 00 00 0F B6", -0x46) : hook::get_pattern<char>("84 C0 0F 84 4A 01 00 00 0F B6", -0x46);
		hook::nop(location + 0x48, 6);
		hook::put<int32_t>(location + 0xA6, (intptr_t)_budgetScale - ((intptr_t)location + 0xA6 + 4));
		hook::put<int32_t>(location + 0xBA, (intptr_t)isStereo - ((intptr_t)location + 0xBA + 5));
		hook::call(location + 0x101, bigUpdate);
	}

	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("B9 84 04 00 00 41 B9 6B", -0x3A), SettingsVramTex, (void**)&g_origSettingsVramTex);
		MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 0C 3B 48 3B C1")), _getAvailableMemoryForStreamer, (void**)&g_origGetAvailableMemoryForStreamer);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	MEMORYSTATUSEX msex = { 0 };
	msex.dwLength = sizeof(msex);

	GlobalMemoryStatusEx(&msex);

	uint32_t allocatorReservation = 0;

	if (msex.ullTotalPhys >= 16 * GB)
	{
		trace(">16 GiB of system RAM, increasing streaming allocator size to 2 GiB\n");

		allocatorReservation = 0x7FFFFFFF;
		extRamMode = 2;
	}
	else if (msex.ullTotalPhys >= 12 * GB)
	{
		trace(">12 GiB of system RAM, increasing streaming allocator size to 1.5 GiB\n");

		allocatorReservation = 0x60000000;
		extRamMode = 1;
	}

	g_vramLocation = hook::get_address<uint64_t*>(hook::get_pattern("4C 63 C0 48 8D 05 ? ? ? ? 48 8D 14", 6));

	OnPostFrontendRender.Connect([]
	{
		static uint64_t lastUpdate = GetTickCount64();

		if ((GetTickCount64() - lastUpdate) >= 2000)
		{
			UpdateMemoryPressure();
			lastUpdate = GetTickCount64();
		}
	});

	// the full code will 100% break 4/4GB systems
	if (extRamMode == 0)
	{
		SetGamePhysicalBudget(3 * GB);

		return;
	}

	// extend grcResourceCache pool a bit
	{
		auto location = hook::get_pattern<char>("BA 00 00 05 00 48 8B C8 44 88");
		hook::put<uint32_t>(location + 1, 0xA0000);
		hook::put<uint32_t>(location + 23, 0xA001B);
	}

	// increase allocator amount
	auto location = hook::get_pattern("41 B8 00 00 00 40 48 8B D5 89", 2);

	if (allocatorReservation)
	{
		hook::put<uint32_t>(location, allocatorReservation);
	}

	SetGamePhysicalBudget(3 * GB);
});
