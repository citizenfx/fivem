#include <StdInc.h>
#include <FontRenderer.h>
#include <DrawCommands.h>
#include <CefOverlay.h>

#include <unordered_set>

#include <dxgi.h>
#include <dxgi1_6.h>
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")

static uint64_t g_nuiInitTimestamp;

void MarkNuiLoaded()
{
	g_nuiInitTimestamp = GetTickCount64();
}

namespace WRL = Microsoft::WRL;

static InitFunction initFunction([]
{
	nui::OnDrawBackground.Connect([](bool isMainUI)
	{
		if (!isMainUI)
		{
			// reset the timestamp if we were out of main UI
			g_nuiInitTimestamp = 0;
			return;
		}

		if (!g_nuiInitTimestamp || (GetTickCount64() - g_nuiInitTimestamp) < 7500)
		{
			return;
		}

		std::string message = R"(If you see this, the user interface isn't able to show.

This usually indicates an environment issue breaking DirectX Graphics Infrastructure (DXGI) shared resources, often caused by misconfigured mobile GPU setups, improperly installed graphics mods, or other system configuration.

)";

		// list GPUs
		static auto gpuList = ([]
		{
			std::string gpuList;
			int gpuIdx = 0;

			std::unordered_set<std::wstring> seenGpus;

			{
				WRL::ComPtr<IDXGIFactory1> dxgiFactory;
				CreateDXGIFactory1(IID_IDXGIFactory1, &dxgiFactory);

				WRL::ComPtr<IDXGIAdapter1> adapter;
				WRL::ComPtr<IDXGIFactory6> factory6;
				HRESULT hr = dxgiFactory.As(&factory6);
				if (SUCCEEDED(hr))
				{
					for (auto type : { DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, DXGI_GPU_PREFERENCE_MINIMUM_POWER })
					{
						auto listStart = gpuIdx;

						for (UINT adapterIndex = 0;
							 DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, type, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
							 adapterIndex++)
						{
							DXGI_ADAPTER_DESC1 desc;
							adapter->GetDesc1(&desc);

							if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
							{
								// Don't select the Basic Render Driver adapter.
								continue;
							}

							if (seenGpus.find(desc.Description) != seenGpus.end())
							{
								continue;
							}

							seenGpus.insert(desc.Description);
							gpuList += fmt::sprintf("GPU #%d: %s (%04x:%04x - %s)\n", ++gpuIdx, ToNarrow(desc.Description), desc.VendorId, desc.DeviceId, type == DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE ? "High Performance" : "Minimum Power");
						}

						if (gpuIdx != listStart)
						{
							gpuList += "\n";
						}
					}
				}
			}

			return gpuList;
		})();

		message += gpuList;

		message += "\nVisit https://aka.cfx.re/no-ui for more information and steps to fix this.";

		TheFonts->DrawRectangle(CRect(0.0f, 0.0f, 8191.0f, 8191.0f), CRGBA(0, 0, 0, 200));
		TheFonts->DrawText(ToWide(":("), CRect(30.0f, 30.0f, 780.0f, 580.0f), CRGBA(255, 255, 255, 255), 36.0f, 1.0f, "Segoe UI");
		TheFonts->DrawText(ToWide(message), CRect(30.0f, 90.0f, 780.0f, 580.0f), CRGBA(255, 255, 255, 255), 18.0f, 1.0f, "Segoe UI");
		TheFonts->DrawPerFrame();
	},
	100);
});
