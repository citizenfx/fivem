#include "StdInc.h"
#include <ConsoleHost.h>

DLL_EXPORT fwEvent<int, int> OnPushNetMetrics;

#ifndef IS_FXSERVER
#include <DrawCommands.h>

#include <imgui.h>

#include <CoreConsole.h>

#include <mmsystem.h>

#include <wrl.h>
#include <pdh.h>

#include <winternl.h>

#ifndef GTA_FIVE
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")
#endif

#include <d3dkmthk.h>

#include "FpsTracker.h"

#pragma comment(lib, "pdh.lib")

static auto GetAdapter()
{
#ifdef GTA_FIVE
	static auto device = GetD3D11Device();

	Microsoft::WRL::ComPtr<IDXGIDevice> deviceRef;
	if (SUCCEEDED(device->QueryInterface(deviceRef.GetAddressOf())))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		if (SUCCEEDED(deviceRef->GetAdapter(&adapter)))
		{
			return adapter;
		}
	}

	return Microsoft::WRL::ComPtr<IDXGIAdapter>{ nullptr };
#else
	Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
	CreateDXGIFactory1(IID_IDXGIFactory1, &dxgiFactory);

	Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(dxgiFactory.As(&factory6)))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

		for (UINT adapterIndex = 0;
			 DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
			 adapterIndex++)
		{
			return adapter;
		}
	}
	return Microsoft::WRL::ComPtr<IDXGIAdapter1>{ nullptr };
#endif
}

static LUID GetAdapterLUID()
{
	LUID adapterLuid;
	adapterLuid.HighPart = -1;
	adapterLuid.LowPart = -1;

	auto adapter = GetAdapter();

	DXGI_ADAPTER_DESC desc;
	if (adapter && SUCCEEDED(adapter->GetDesc(&desc)))
	{
		adapterLuid = desc.AdapterLuid;
	}

	return adapterLuid;
}

static InitFunction initFunction([]()
{
	static bool drawPerfEnabled = false;
	static ConVar<bool> drawPerf("cl_drawPerf", ConVar_Archive, false, &drawPerfEnabled);

	static std::vector<std::tuple<std::shared_ptr<ConVar<bool>>, std::function<std::string()>>> drawPerfModules;
	auto addDrawPerfModule = [](const std::string& convar, const std::string& label, std::function<std::string()>&& fn)
	{
		drawPerfModules.emplace_back(std::make_shared<ConVar<bool>>(convar, ConVar_Archive, true), std::move(fn));

		// enable it for console usage
		seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "system.extConsole" }, se::Object{ fmt::sprintf("command.%s", convar) }, se::AccessType::Allow);

		// add to devgui
		console::GetDefaultContext()->AddToBuffer(fmt::sprintf("devgui_convar \"Overlays/Performance/%s\" %s\n", label, convar));
	};

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || drawPerfEnabled;
	});

	static int gPing;
	static int gPl;

	OnPushNetMetrics.Connect([](int ping, int pl)
	{
		gPing = ping;
		gPl = pl;
	});

	static FpsTracker fpsTracker;

	ConHost::OnDrawGui.Connect([]()
	{
		if (!drawPerfEnabled)
		{
			return;
		}

		fpsTracker.Tick();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y), 0, ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

		std::vector<std::string> metrics;

		for (const auto& [var, module] : drawPerfModules)
		{
			if (var->GetValue())
			{
				auto result = module();

				if (!result.empty())
				{
					metrics.emplace_back(std::move(result));
				}
			}
		}

		if (!metrics.empty() && ImGui::Begin("DrawPerf", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
		{
			int i = 0;
			float spacing = ImGui::GetStyle().ItemSpacing.x;
			float pos = spacing;

			for (auto& metric : metrics)
			{
				auto textSize = ImGui::CalcTextSize(metric.c_str());
				ImGui::Text("%s", metric.c_str());
				pos += (spacing * 2) + std::max(textSize.x, spacing * 18);

				i++;

				ImGui::SameLine(pos);

				if (i != metrics.size())
				{
					// hacky separator
					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					ImVec2 p = ImGui::GetCursorScreenPos();
					draw_list->AddLine(ImVec2(p.x - spacing, p.y - 9999.f), ImVec2(p.x - spacing, p.y + 9999.f), ImGui::GetColorU32(ImGuiCol_Border));
				}
			}
		}

		ImGui::PopStyleVar();
		ImGui::End();
	});

	addDrawPerfModule("cl_drawPerfFPS", "FPS", []() -> std::string
	{
		// FPS
		if (fpsTracker.CanGet())
		{
			return fmt::sprintf("FPS: %llu", fpsTracker.Get());
		}

		return "";
	});

	addDrawPerfModule("cl_drawPing", "Ping", []
	{
		// Latency
		return fmt::sprintf("Ping: %dms", gPing);
	});

	addDrawPerfModule("cl_drawPacketLoss", "Packet Loss", []
	{
		// Packet loss
		return fmt::sprintf("PL: %d%%", gPl);
	});

	addDrawPerfModule("cl_drawCpuUsage", "CPU Usage", []
	{
		// CPU usage
		static PDH_HQUERY cpuQuery;
		static PDH_HCOUNTER cpuTotal;

		// from https://stackoverflow.com/a/64166
		if (!cpuQuery)
		{
			PdhOpenQuery(NULL, NULL, &cpuQuery);
			PdhAddEnglishCounter(cpuQuery, L"\\Processor Information(_Total)\\% Processor Time", NULL, &cpuTotal);
			PdhCollectQueryData(cpuQuery);
		}

		static PDH_FMT_COUNTERVALUE counterValCpu;
		static DWORD lastCpuQuery;

		if ((timeGetTime() - lastCpuQuery) > 500)
		{
			PdhCollectQueryData(cpuQuery);
			PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterValCpu);

			lastCpuQuery = timeGetTime();
		}

		return fmt::sprintf("CPU: %.0f%%", counterValCpu.doubleValue);
	});

	addDrawPerfModule("cl_drawGpuUsage", "GPU Usage", []() -> std::string
	{
		// GPU usage/temperature
		if (IsWindows10OrGreater())
		{
			static auto adapterLuid = GetAdapterLUID();

			if (adapterLuid.HighPart != -1 && adapterLuid.LowPart != -1)
			{
				// GPU usage
				static PDH_HQUERY gpuQuery;
				static PDH_HCOUNTER gpuTotal;

				// from https://stackoverflow.com/a/64166
				if (!gpuQuery)
				{
					PdhOpenQuery(NULL, NULL, &gpuQuery);
					PdhAddEnglishCounter(gpuQuery, L"\\GPU Engine(*)\\Utilization Percentage", NULL, &gpuTotal);
					PdhCollectQueryData(gpuQuery);
				}

				static double gpuValue;
				static DWORD lastGpuQuery;

				if ((timeGetTime() - lastGpuQuery) > 500)
				{
					PdhCollectQueryData(gpuQuery);

					DWORD bufferSize = 0;
					DWORD numItems = 0;

					PdhGetRawCounterArrayW(gpuTotal, &bufferSize, &numItems, NULL);

					static std::vector<uint8_t> lastBuffer;
					static DWORD lastItems;

					std::vector<uint8_t> gpuBuffer(bufferSize);
					PdhGetRawCounterArrayW(gpuTotal, &bufferSize, &numItems, (PDH_RAW_COUNTER_ITEM_W*)gpuBuffer.data());

					static DWORD counterType = -1;

					if (counterType == -1)
					{
						DWORD bufferSize = 0;
						PdhGetCounterInfoW(gpuTotal, FALSE, &bufferSize, NULL);

						if (bufferSize > 0)
						{
							std::vector<uint8_t> gpuBuffer(bufferSize);
							PdhGetCounterInfoW(gpuTotal, FALSE, &bufferSize, (PDH_COUNTER_INFO_W*)gpuBuffer.data());

							auto e = (PDH_COUNTER_INFO_W*)&gpuBuffer[0];
							counterType = e->dwType;
						}
					}

					if (counterType != -1)
					{
						PDH_RAW_COUNTER_ITEM_W* items = (PDH_RAW_COUNTER_ITEM_W*)&gpuBuffer[0];
						PDH_RAW_COUNTER_ITEM_W* lastItemsRef = (PDH_RAW_COUNTER_ITEM_W*)&lastBuffer[0];

						auto ref = fmt::sprintf(L"luid_0x%08X_0x%08X", adapterLuid.HighPart, adapterLuid.LowPart);
						auto pidRef = fmt::sprintf(L"pid_%d", GetCurrentProcessId());

						PDH_FMT_COUNTERVALUE counterValGpu;
						gpuValue = 0;

						for (DWORD i = 0; i < numItems; i++)
						{
							if (wcsstr(items[i].szName, ref.c_str()) && wcsstr(items[i].szName, L"_engtype_3D")) // && wcsstr(items[i].szName, pidRef.c_str()))
							{
								int li = -1;

								for (DWORD j = 0; j < lastItems; j++)
								{
									if (wcscmp(lastItemsRef[j].szName, items[i].szName) == 0)
									{
										li = j;
										break;
									}
								}

								if (li >= 0)
								{
									PdhFormatFromRawValue(counterType, PDH_FMT_DOUBLE, NULL, &items[i].RawValue, &lastItemsRef[li].RawValue, &counterValGpu);

									gpuValue += counterValGpu.doubleValue;
								}
							}
						}

						lastBuffer = std::move(gpuBuffer);
						lastItems = numItems;
					}

					lastGpuQuery = timeGetTime();
				}

				return fmt::sprintf("GPU: %.0f%%", gpuValue);
			}
		}

		return "";
	});

	addDrawPerfModule("cl_drawGpuTemp", "GPU Temperature", []() -> std::string
	{
		// GPU usage/temperature
		if (IsWindows10OrGreater())
		{
			static auto adapterLuid = GetAdapterLUID();

			// GPU temperature
			static D3DKMT_HANDLE adapterHandle;

			if (adapterHandle == 0)
			{
				D3DKMT_OPENADAPTERFROMLUID openReq;
				openReq.AdapterLuid = adapterLuid;

				if (SUCCEEDED(D3DKMTOpenAdapterFromLuid(&openReq)))
				{
					adapterHandle = openReq.hAdapter;
				}
			}

			if (adapterHandle)
			{
				D3DKMT_QUERYADAPTERINFO queryAdapterInfo;
				D3DKMT_ADAPTER_PERFDATA adapterPerfData;

				memset(&adapterPerfData, 0, sizeof(D3DKMT_ADAPTER_PERFDATA));
				memset(&queryAdapterInfo, 0, sizeof(D3DKMT_QUERYADAPTERINFO));

				queryAdapterInfo.hAdapter = adapterHandle;
				queryAdapterInfo.Type = KMTQAITYPE_ADAPTERPERFDATA;
				queryAdapterInfo.pPrivateDriverData = &adapterPerfData;
				queryAdapterInfo.PrivateDriverDataSize = sizeof(adapterPerfData);

				if (SUCCEEDED(D3DKMTQueryAdapterInfo(&queryAdapterInfo)))
				{
					float fracTemp = adapterPerfData.Temperature / 10.f;

					// some (AMD?) GPU drivers return temperatures of '6900 C' here, so we divide those to be within range again
					if (fracTemp > 200) // 200 is a bit outrageous
					{
						fracTemp /= 100.0f;
					}

					return fmt::sprintf("GPU Temp: %.0f\xC2\xB0"
										"C",
					fracTemp);
				}
			}
		}

		return "";
	});

	addDrawPerfModule("cl_drawGpuMemoryUsage", "GPU Memory", []() -> std::string
	{
		if (IsWindows10OrGreater())
		{
			auto adapter = GetAdapter();
			Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter3;

			if (adapter && SUCCEEDED(adapter.As(&adapter3)))
			{
				DXGI_ADAPTER_DESC desc;
				adapter->GetDesc(&desc);

				DXGI_QUERY_VIDEO_MEMORY_INFO vmiLocal;
				if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vmiLocal)))
				{
					auto humanizeBytes = [](LONGLONG bytes)
					{
						return fmt::sprintf("%.1f", bytes / 1024.0 / 1024.0 / 1024.0);
					};

					static auto adapterLuid = GetAdapterLUID();
					static uint64_t gpuValue = 0;

					if (adapterLuid.HighPart != -1 && adapterLuid.LowPart != -1)
					{
						// GPU usage
						static PDH_HQUERY gpuQuery;
						static PDH_HCOUNTER gpuTotal;

						if (!gpuQuery)
						{
							PdhOpenQuery(NULL, NULL, &gpuQuery);
							PdhAddEnglishCounter(gpuQuery, L"\\GPU Adapter Memory(*)\\Dedicated Usage", NULL, &gpuTotal);
							PdhCollectQueryData(gpuQuery);
						}

						static DWORD lastGpuQuery;

						if ((timeGetTime() - lastGpuQuery) > 500)
						{
							PdhCollectQueryData(gpuQuery);

							DWORD bufferSize = 0;
							DWORD numItems = 0;

							PdhGetRawCounterArrayW(gpuTotal, &bufferSize, &numItems, NULL);

							std::vector<uint8_t> gpuBuffer(bufferSize);
							PdhGetRawCounterArrayW(gpuTotal, &bufferSize, &numItems, (PDH_RAW_COUNTER_ITEM_W*)gpuBuffer.data());

							static DWORD counterType = -1;

							if (counterType == -1)
							{
								DWORD bufferSize = 0;
								PdhGetCounterInfoW(gpuTotal, FALSE, &bufferSize, NULL);

								if (bufferSize > 0)
								{
									std::vector<uint8_t> gpuBuffer(bufferSize);
									PdhGetCounterInfoW(gpuTotal, FALSE, &bufferSize, (PDH_COUNTER_INFO_W*)gpuBuffer.data());

									auto e = (PDH_COUNTER_INFO_W*)&gpuBuffer[0];
									counterType = e->dwType;
								}
							}

							if (counterType != -1)
							{
								PDH_RAW_COUNTER_ITEM_W* items = (PDH_RAW_COUNTER_ITEM_W*)&gpuBuffer[0];

								auto ref = fmt::sprintf(L"luid_0x%08X_0x%08X", adapterLuid.HighPart, adapterLuid.LowPart);

								PDH_FMT_COUNTERVALUE counterValGpu;
								gpuValue = 0;

								for (DWORD i = 0; i < numItems; i++)
								{
									if (wcsstr(items[i].szName, ref.c_str()))
									{
										PdhFormatFromRawValue(counterType, PDH_FMT_LARGE, NULL, &items[i].RawValue, NULL, &counterValGpu);

										gpuValue = counterValGpu.largeValue;
									}
								}
							}

							lastGpuQuery = timeGetTime();
						}
					}

					return fmt::sprintf("VRAM: %s/%s/%s GB", humanizeBytes(vmiLocal.CurrentUsage), humanizeBytes(gpuValue), humanizeBytes(desc.DedicatedVideoMemory));
				}
			}
		}

		return "";
	});
}, INT32_MAX);
#endif
