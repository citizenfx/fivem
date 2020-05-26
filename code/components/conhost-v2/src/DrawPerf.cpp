#include "StdInc.h"
#include <ConsoleHost.h>

DLL_EXPORT fwEvent<int, int> OnPushNetMetrics;

#ifdef GTA_FIVE
#include <DrawCommands.h>

#include <imgui.h>

#include <CoreConsole.h>

#include <mmsystem.h>

#include <wrl.h>
#include <pdh.h>

#include <winternl.h>
#include <d3dkmthk.h>

#pragma comment(lib, "pdh.lib")

static InitFunction initFunction([]()
{
	static bool drawFpsEnabled;
	static bool streamingListEnabled;
	static bool streamingMemoryEnabled;

	static ConVar<bool> drawFps("cl_drawPerf", ConVar_Archive, false, &drawFpsEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || drawFpsEnabled;
	});

	static int gPing;
	static int gPl;

	OnPushNetMetrics.Connect([](int ping, int pl)
	{
		gPing = ping;
		gPl = pl;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!drawFpsEnabled)
		{
			return;
		}

		auto& io = ImGui::GetIO();

		static std::chrono::high_resolution_clock::duration previous;
		static uint32_t index;
		static std::chrono::microseconds previousTimes[6];

		auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
		auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(t - previous);
		previous = t;

		previousTimes[index % std::size(previousTimes)] = frameTime;
		index++;

		ImGui::SetNextWindowPos(ImVec2(-1, -1), 0, ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

		if (ImGui::Begin("DrawPerf", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
		{
			std::vector<std::string> metrics;

			// FPS
			if (index > 6)
			{
				std::chrono::microseconds total{ 0 };

				for (int i = 0; i < std::size(previousTimes); i++)
				{
					total += previousTimes[i];
				}

				if (total.count() == 0)
				{
					total = { 1 };
				}

				uint64_t fps = ((uint64_t)1000000 * 1000) * std::size(previousTimes) / total.count();
				fps = (fps + 500) / 1000;

				metrics.push_back(fmt::sprintf("FPS: %llu", fps));
			}

			// Latency
			metrics.push_back(fmt::sprintf("Ping: %dms", gPing));

			// Packet loss
			metrics.push_back(fmt::sprintf("PL: %d%%", gPl));

			// CPU usage
			static PDH_HQUERY cpuQuery;
			static PDH_HCOUNTER cpuTotal;

			// from https://stackoverflow.com/a/64166
			if (!cpuQuery)
			{
				PdhOpenQuery(NULL, NULL, &cpuQuery);
				PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
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
			
			metrics.push_back(fmt::sprintf("CPU: %.0f%%", counterValCpu.doubleValue));

			// GPU usage/temperature
			static auto device = GetD3D11Device();

			if (device && IsWindows10OrGreater())
			{
				static LUID adapterLuid;

				if (adapterLuid.HighPart == 0 && adapterLuid.LowPart == 0)
				{
					adapterLuid.HighPart = -1;
					adapterLuid.LowPart = -1;

					Microsoft::WRL::ComPtr<IDXGIDevice> deviceRef;
					if (SUCCEEDED(device->QueryInterface(deviceRef.GetAddressOf())))
					{
						Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
						if (SUCCEEDED(deviceRef->GetAdapter(&adapter)))
						{
							DXGI_ADAPTER_DESC adapterDesc;
							if (SUCCEEDED(adapter->GetDesc(&adapterDesc)))
							{
								adapterLuid = adapterDesc.AdapterLuid;
							}
						}
					}
				}

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

					metrics.push_back(fmt::sprintf("GPU: %.0f%%", gpuValue));

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
							metrics.push_back(fmt::sprintf("GPU Temp: %.0f\xC2\xB0""C", adapterPerfData.Temperature / 10.f));
						}
					}
				}
			}

			int i = 0;

			for (auto& metric : metrics)
			{
				ImGui::Text("%s", metric.c_str());
				i++;

				ImGui::SameLine(i * 150.f);
				
				if (i != metrics.size())
				{
					// hacky separator
					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					ImVec2 p = ImGui::GetCursorScreenPos();
					draw_list->AddLine(ImVec2(p.x - 10.f, p.y - 9999.f), ImVec2(p.x - 10.f, p.y + 9999.f), ImGui::GetColorU32(ImGuiCol_Border));
				}
			}
		}

		ImGui::PopStyleVar();
		ImGui::End();
	});
});
#endif
