#include <StdInc.h>
#include <CustomText.h>

#include <DrawCommands.h>
#include <nutsnbolts.h>

#include <fiDevice.h>
#include <Hooking.h>

#include <MinHook.h>

#include <boost/algorithm/string.hpp>

static bool IsNewInstall()
{
	if (GetFileAttributes(MakeRelativeCitPath(L"citizen/new_vid_behavior.txt").c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		return true;
	}

	auto cachePath = MakeRelativeCitPath(L"crashes");

	HANDLE hFile = CreateFileW(cachePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		FILETIME ctime;
		GetFileTime(hFile, &ctime, NULL, NULL);

		CloseHandle(hFile);

		ULARGE_INTEGER t;
		t.LowPart = ctime.dwLowDateTime;
		t.HighPart = ctime.dwHighDateTime;

		return (t.QuadPart > 132356430250000000ULL);
	}

	return false;
}

static bool isNewSettingFile;

static hook::cdecl_stub<bool(void*, void*)> _saveSettings([]()
{
	return hook::get_pattern("66 39 34 48 75 F7 8D 41 01 48 8B CE", -0x55);
});

static void SetDefaults(char* settings)
{
	// default to borderless
	*(int*)(settings + 260) = 2;

	// and vsync off
	*(int*)(settings + 264) = 0;

	// and screen-native width
	POINT p = { 0, 0 };
	HMONITOR monitor = MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);

	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfoW(monitor, &mi);

	*(int*)(settings + 248) = mi.rcMonitor.right - mi.rcMonitor.left;
	*(int*)(settings + 252) = mi.rcMonitor.bottom - mi.rcMonitor.top;
}

static void (*g_origLoadSettingsFromParams)(void*);

static void LoadSettingsFromParams(char* settings)
{
	g_origLoadSettingsFromParams(settings);

	if (isNewSettingFile)
	{
		SetDefaults(settings);

		_saveSettings(settings, settings + 8);
	}
}

static void (*g_origResetSettings)(void*);

static void ResetSettings(char* settings)
{
	g_origResetSettings(settings);

	SetDefaults(settings);
}

extern DLL_IMPORT fwEvent<bool*> OnFlipModelHook;

#include <wrl.h>

namespace WRL = Microsoft::WRL;

#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

static HookFunction hookFunction([]()
{
	// settings.xml moving for new installs
	if (IsNewInstall())
	{
		auto location = hook::get_address<const char**>(hook::get_pattern("48 8D 54 24 30 48 2B D1 8A 01 88 04 0A", -4));
		hook::put(location, "fxd:/gta5_settings.xml");

		MH_Initialize();
		MH_CreateHook(hook::get_pattern("84 C0 74 16 8B 45 ? 85 C0", -0x3C), LoadSettingsFromParams, (void**)&g_origLoadSettingsFromParams);
		MH_CreateHook(hook::get_call(hook::get_pattern("89 8D ? ? 00 00 74 0A 48 8B CB E8 ? ? ? ? EB", 11)), ResetSettings, (void**)&g_origResetSettings);
		MH_EnableHook(MH_ALL_HOOKS);

		// flip model is currently disabled as it breaks some cases of fullscreen resizing (open in borderless -> alt-enter -> alt-tab)
		// due to it being non-trivial to get the game to call ResizeBuffers.
		//
		// this needs some remote debugging attempt in order to correctly breakpoint all the state changes involved as fullscreen/focus are global
		// and therefore the debugger can't be used to investigate on a live system.
#if 0
		OnFlipModelHook.Connect([](bool* flip)
		{
			if (!*flip)
			{
				{
					WRL::ComPtr<IDXGIFactory> dxgiFactory;
					CreateDXGIFactory(IID_IDXGIFactory, &dxgiFactory);

					WRL::ComPtr<IDXGIAdapter1> adapter;
					WRL::ComPtr<IDXGIFactory6> factory6;
					HRESULT hr = dxgiFactory.As(&factory6);
					if (SUCCEEDED(hr))
					{
						std::wstring hpGpu;
						std::wstring lpGpu;

						for (UINT adapterIndex = 0;
							 DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
							 adapterIndex++)
						{
							DXGI_ADAPTER_DESC1 desc;
							adapter->GetDesc1(&desc);

							if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
							{
								// Don't select the Basic Render Driver adapter.
								continue;
							}

							hpGpu = desc.Description;
							break;
						}

						for (UINT adapterIndex = 0;
							 DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_MINIMUM_POWER, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
							 adapterIndex++)
						{
							DXGI_ADAPTER_DESC1 desc;
							adapter->GetDesc1(&desc);

							if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
							{
								// Don't select the Basic Render Driver adapter.
								continue;
							}

							lpGpu = desc.Description;
							break;
						}

						// if they're the same GPU, or only one exists
						if (hpGpu.empty() || lpGpu.empty() || lpGpu == hpGpu)
						{
							*flip = true;
						}
					}
				}
			}
		});
#endif

		rage::fiDevice::OnInitialMount.Connect([]()
		{
			auto targetDevice = rage::fiDevice::GetDevice("fxd:/gta5_settings.xml", true);

			if (targetDevice)
			{
				auto handle = targetDevice->Open("fxd:/gta5_settings.xml", true);

				if (handle != -1)
				{
					targetDevice->Close(handle);
					return;
				}
			}

			isNewSettingFile = true;

			// migrate old settings
			auto origDevice = rage::fiDevice::GetDevice("user:/settings.xml", true);

			if (origDevice)
			{
				auto handle = origDevice->Open("user:/settings.xml", true);

				if (handle != -1)
				{
					auto len = origDevice->GetFileLength(handle);
					std::vector<char> data(len);

					origDevice->Read(handle, data.data(), len);

					origDevice->Close(handle);

					auto targetDevice = rage::fiDevice::GetDevice("fxd:/gta5_settings.xml", false);

					if (targetDevice)
					{
						auto handle = targetDevice->Create("fxd:/gta5_settings.xml");

						if (handle != -1)
						{
							targetDevice->Write(handle, data.data(), data.size());
							targetDevice->Close(handle);
						}
					}
				}
			}
		}, INT32_MAX);
	}

	// move settings around

	// aspect ratios
	hook::put<int>(hook::get_pattern("48 8D 68 A1 48 81 EC A0 00 00 00 8B D9 B9 89", 14), 0x8A);

	// window modes
	hook::put<int>(hook::get_pattern("45 8D 7C 24 02 8D 43 01 48 C1 E6 05", -0x50), 0x8A);

	// resolutions
	hook::put<int>(hook::get_pattern("48 8D 68 A9 48 81 EC 90 00 00 00 44 8B F1 B9 89", 15), 0x8A);

	// refresh rates
	hook::put<int>(hook::get_pattern("48 81 EC 80 00 00 00 8B D9 B9 89 00 00 00 44 8A E2", 10), 0x8A);

	// scaling rates
	{
		auto location = hook::get_pattern<char>("B9 8A 00 00 00 44 8A F2 E8", 1);
		hook::put<int>(location, 0x89);

		const char* repStr = "MO_GFX_1o1";
		auto mem = (char*)hook::AllocateStubMemory(strlen(repStr) + 1);
		strcpy(mem, repStr);

		hook::put<int32_t>(location + 0x4C, mem - (location + 0x4C) - 4);
	}
});

static InitFunction initFunction([]()
{
	game::AddCustomText("GFX_SCALING", "Render Resolution");

	OnMainGameFrame.Connect([]()
	{
		static int lastResX, lastResY;
		int resX, resY;

		GetGameResolution(resX, resY);

		if (lastResX != resX || lastResY != resY)
		{
			lastResX = resX;
			lastResY = resY;

			struct ResPair
			{
				std::string_view label;
				std::string_view origLabel;
				double ratio;
			};

			ResPair resPairs[] = {
				{ "MO_GFX_1o1", "", 1.0 },
				{ "MO_GFX_1o2", "1/2", 1.0 / 2.0 },
				{ "MO_GFX_2o3", "2/3", 2.0 / 3.0 },
				{ "MO_GFX_3o4", "3/4", 3.0 / 4.0 },
				{ "MO_GFX_5o6", "5/6", 5.0 / 6.0 },
				{ "MO_GFX_5o4", "5/4", 5.0 / 4.0 },
				{ "MO_GFX_3o2", "3/2", 3.0 / 2.0 },
				{ "MO_GFX_7o4", "7/4", 7.0 / 4.0 },
				{ "MO_GFX_2o1", "2/1", 2.0 / 1.0 },
				{ "MO_GFX_5o2", "5/2", 5.0 / 2.0 },
			};

			for (auto& pair : resPairs)
			{
				game::AddCustomText(std::string{ pair.label }, fmt::sprintf("%ix%i%s",
					(int)round(resX * pair.ratio),
					(int)round(resY * pair.ratio),
					pair.origLabel.empty() ? "" : fmt::sprintf(" (%s)", pair.origLabel)));
			}

			HMONITOR monitor = MonitorFromWindow(FindWindow(L"grcWindow", NULL), MONITOR_DEFAULTTOPRIMARY);

			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfoW(monitor, &mi);

			bool isFullSize = (resX == (mi.rcMonitor.right - mi.rcMonitor.left)) && (resY == (mi.rcMonitor.bottom - mi.rcMonitor.top));

			game::AddCustomText("VID_SCR_BORDERLESS", isFullSize ? "Full Screen" : "Windowed (Borderless)");
			game::AddCustomText("VID_FULLSCREEN", "Full Screen (Exclusive)");
		}
	});
});
