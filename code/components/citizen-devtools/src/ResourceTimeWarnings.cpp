#include "StdInc.h"

#include <Resource.h>
#include <ResourceManager.h>
#include <ConsoleHost.h>
#include <ResourceScriptingComponent.h>

#if __has_include(<ResourceGameLifetimeEvents.h>)
#include <ResourceGameLifetimeEvents.h>
#endif

#include <CoreConsole.h>

#include <imgui.h>

#include <chrono>

#if __has_include(<scrThread.h>)
#include <scrThread.h>
#include <scrEngine.h>
#endif

#ifdef GTA_FIVE
#include <Streaming.h>
#include <ScriptHandlerMgr.h>

#include <nutsnbolts.h>
#endif

DLL_IMPORT ImFont* GetConsoleFontTiny();

using namespace std::chrono_literals;

inline std::chrono::microseconds usec()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

template<int SampleCount>
struct TickMetrics
{
	int curTickTime = 0;
	std::chrono::microseconds tickTimes[SampleCount];

	inline void Append(std::chrono::microseconds time)
	{
		tickTimes[curTickTime++] = time;

		if (curTickTime >= _countof(tickTimes))
		{
			curTickTime = 0;
		}
	}

	inline std::chrono::microseconds GetAverage() const
	{
		std::chrono::microseconds avgTickTime(0);

		for (auto tickTime : tickTimes)
		{
			avgTickTime += tickTime;
		}

		avgTickTime /= std::size(tickTimes);

		return avgTickTime;
	}

	inline void Reset()
	{
		for (auto& tt : tickTimes)
		{
			tt = std::chrono::microseconds{ 0 };
		}
	}
};

struct ResourceMetrics
{
	std::chrono::microseconds tickStart;
	TickMetrics<64> ticks;

	std::chrono::microseconds memoryLastFetched;

	int64_t memorySize;

#if __has_include(<scrThread.h>)
	GtaThread* gtaThread;
#endif
};

static ImVec4 GetColorForRange(float min, float max, float num)
{
	float avg = (min + max) / 2.0f;
	float scale = 1.0f / (avg - min);

	if (num < min)
	{
		return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (num >= max)
	{
		return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (num >= avg)
	{
		return ImVec4(1.0f, 1.0f - ((num - avg) * scale), 0.0f, 1.0f);
	}
	else
	{
		return ImVec4(((num - min) * scale), 1.0f, 0.0f, 1.0f);
	}
}

static int64_t GetTotalBytes(const fwRefContainer<fx::Resource>& resource)
{
	int64_t totalBytes = 0;

	auto scripting = resource->GetComponent<fx::ResourceScriptingComponent>();
	scripting->ForAllRuntimes([&totalBytes](fx::OMPtr<IScriptRuntime> scRt)
	{
		fx::OMPtr<IScriptMemInfoRuntime> miRt;

		if (FX_SUCCEEDED(scRt.As<IScriptMemInfoRuntime>(&miRt)))
		{
			if (FX_SUCCEEDED(miRt->RequestMemoryUsage()))
			{
				int64_t bytes = 0;

				if (FX_SUCCEEDED(miRt->GetMemoryUsage(&bytes)))
				{
					totalBytes += bytes;
				}
			}
		}
	});

	return totalBytes;
}

#ifdef GTA_FIVE
size_t CountDependencyMemory(streaming::Manager* streaming, uint32_t strIdx);

static size_t GetStreamingUsageForThread(GtaThread* thread)
{
	size_t memory = 0;

	if (thread)
	{
		if (thread->GetScriptHandler())
		{	
			thread->GetScriptHandler()->ForAllResources([&](rage::scriptResource* resource)
			{
				uint32_t strIdx = -1;
				resource->GetStreamingIndex(&strIdx);

				if (strIdx != -1)
				{
					memory += CountDependencyMemory(streaming::Manager::GetInstance(), strIdx);
				}
			});
		}
	}

	return memory;
}
#endif

static InitFunction initFunction([]()
{
	static bool resourceTimeWarningShown;
	static std::chrono::microseconds warningLastShown;
	static std::string resourceTimeWarningText;
	static std::mutex mutex;

	static bool taskMgrEnabled;

	static ConVar<bool> taskMgrVar("resmon", ConVar_Archive, false, &taskMgrEnabled);

	static tbb::concurrent_unordered_map<std::string, std::optional<ResourceMetrics>> metrics;
	static TickMetrics<64> scriptFrameMetrics;
	static TickMetrics<64> gameFrameMetrics;

#ifdef GTA_FIVE
	static auto frameBegin = usec();

	OnBeginGameFrame.Connect([]()
	{
		frameBegin = usec();
	});

	OnEndGameFrame.Connect([]()
	{
		auto now = usec();
		auto frameTime = now - frameBegin;

		gameFrameMetrics.Append(frameTime);
	});

	OnDeleteResourceThread.Connect([](rage::scrThread* thread)
	{
		for (auto& metric : metrics)
		{
			if (metric.second)
			{
				if (metric.second->gtaThread == thread)
				{
					metric.second->gtaThread = nullptr;
				}
			}
		}
	});
#endif

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnStart.Connect([resource]()
		{
			metrics[resource->GetName()] = ResourceMetrics{};

			metrics[resource->GetName()]->ticks.Reset();
		});

#if __has_include(<scrThread.h>)
		resource->OnActivate.Connect([resource]()
		{
			metrics[resource->GetName()]->gtaThread = (GtaThread*)rage::scrEngine::GetActiveThread();
		}, 9999);
#endif

		resource->OnTick.Connect([resource]()
		{
			metrics[resource->GetName()]->tickStart = usec();
		}, -99999999);

		resource->OnTick.Connect([resource]()
		{
			auto& metric = *metrics[resource->GetName()];
			metric.ticks.Append(usec() - metric.tickStart);

			if ((usec() - metric.memoryLastFetched) > (!taskMgrEnabled ? 20s : 500ms))
			{
				int64_t totalBytes = GetTotalBytes(resource);

				metric.memorySize = totalBytes;
				metric.memoryLastFetched = usec();
			}
		}, 99999999);

		resource->OnStop.Connect([resource]()
		{
			metrics[resource->GetName()] = {};
		});

#if __has_include(<scrThread.h>) && __has_include(<ResourceGameLifeTimeEvents.h>)
		resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnBeforeGameShutdown.Connect([resource]()
		{
			auto m = metrics[resource->GetName()];

			if (m)
			{
				m->gtaThread = nullptr;
			}
		}, -50);

		resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnGameDisconnect.Connect([resource]()
		{
			auto m = metrics[resource->GetName()];

			if (m)
			{
				m->gtaThread = nullptr;
			}
		}, -50);
#endif
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		static std::chrono::microseconds scriptBeginTime;

		manager->OnTick.Connect([]()
		{
			scriptBeginTime = usec();
		}, INT32_MIN);

		manager->OnTick.Connect([]()
		{
			auto scriptEndTime = usec() - scriptBeginTime;
			scriptFrameMetrics.Append(scriptEndTime);
		}, INT32_MAX);

		manager->OnTick.Connect([]()
		{
			bool showWarning = false;
			std::string warningText;

			for (const auto& [ key, metricRef ] : metrics)
			{
				if (!metricRef)
				{
					continue;
				}

				auto& metric = *metricRef;
				auto avgTickTime = metric.ticks.GetAverage();

				if (avgTickTime > 6ms)
				{
					float fpsCount = (60 - (1000.f / (16.67f + (avgTickTime.count() / 1000.0))));

					showWarning = true;
					warningText += fmt::sprintf("%s is taking %.2f ms (or -%.1f FPS @ 60 Hz)\n", key, avgTickTime.count() / 1000.0, fpsCount);
				}

				if (metric.memorySize > (50 * 1024 * 1024))
				{
					showWarning = true;
					warningText += fmt::sprintf("%s is using %.2f MiB of RAM\n", key, metric.memorySize / 1024.0 / 1024.0);
				}
			}

#if 0
			auto avgFrameTime = gameFrameMetrics.GetAverage();
			auto avgScriptTime = scriptFrameMetrics.GetAverage();

			double avgFrameMs = (avgFrameTime.count() / 1000.0);
			double avgScriptMs = (avgScriptTime.count() / 1000.0);

			if (avgFrameTime > 0us)
			{
				double avgFrameFraction = (avgScriptMs / avgFrameMs);

				bool wouldBeOver60 = false;

				// if <60 FPS (minus render frame queueing guess)
				if (avgFrameMs >= 15.66)
				{
					// and without scripts it would be 60 FPS
					if ((avgFrameMs - avgScriptMs) < 15.6666)
					{
						// use a 30% threshold in that case
						wouldBeOver60 = (avgFrameFraction >= 0.3);
					}
				}

				// 60%, when a frame takes more than 8.33ms (<120 FPS)
				if ((avgFrameFraction >= 0.6 && avgFrameMs >= 8.33) || wouldBeOver60)
				{
					std::vector<std::tuple<uint64_t, std::string>> topEntries;

					for (const auto& [key, metricRef] : metrics)
					{
						if (!metricRef)
						{
							continue;
						}

						auto& metric = *metricRef;
						auto avgTickTime = metric.ticks.GetAverage();

						topEntries.emplace_back(avgTickTime.count(), key);
					}

					std::sort(topEntries.begin(), topEntries.end());

					std::string topList = "";
					int c = 0;

					for (auto it = topEntries.rbegin(); it != topEntries.rend(); it++)
					{
						if (c >= 3)
						{
							break;
						}

						topList += ((c != 0) ? " " : "") + std::get<1>(*it);
						c++;
					}

					showWarning = true;
					warningText += fmt::sprintf("Total script tick time of %.2fms is %.1f percent of total frame time (%.2fms)%s\nTop resources: [%s]\n", avgScriptMs, avgFrameFraction * 100.0, avgFrameMs, wouldBeOver60 ? "\nOptimizing slow scripts might bring you above 60 FPS. Open the Resource Monitor in F8 to begin." : "", topList);
				}
			}
#endif

			if (showWarning)
			{
				std::unique_lock<std::mutex> lock(mutex);
				
				if (!resourceTimeWarningShown)
				{
					warningLastShown = usec();
				}
				
				// warn again 10 minutes later
				if ((usec() - warningLastShown) > 600s)
				{
					warningLastShown = usec();
				}

				resourceTimeWarningShown = true;
				resourceTimeWarningText = warningText;
			}
			else
			{
				if (resourceTimeWarningShown && (usec() - warningLastShown) < 17s)
				{
					warningLastShown = usec() - 17s;
				}

				resourceTimeWarningShown = false;
			}
		});
	});

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || ((usec() - warningLastShown) < 20s) || taskMgrEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
#ifndef IS_FXSERVER
		if ((usec() - warningLastShown) < 20s)
		{
			ImGui::PushFont(GetConsoleFontTiny());

			const float DISTANCE = 10.0f;
			ImVec2 window_pos = ImVec2(ImGui::GetMainViewport()->Pos.x + ImGui::GetIO().DisplaySize.x - DISTANCE, ImGui::GetMainViewport()->Pos.y + ImGui::GetIO().DisplaySize.y - DISTANCE);
			ImVec2 window_pos_pivot = ImVec2(1.0f, 1.0f);
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

			ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
			if (ImGui::Begin("Time Warning", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
			{
				ImGui::Text("/!\\ Resource time warning");
				ImGui::Separator();
				ImGui::Text(resourceTimeWarningText.c_str());
				ImGui::Separator();
				ImGui::Text("Please contact the server owner to resolve this issue.");
			}
			ImGui::End();
			ImGui::PopFont();
		}
#endif

		if (taskMgrEnabled)
		{
			if (ImGui::Begin("Resource Monitor", &taskMgrEnabled) && ImGui::BeginTable("##resmon", 5, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("Resource");
				ImGui::TableSetupColumn("CPU msec", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Time %", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Streaming", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableHeadersRow();

				std::map<std::string, fwRefContainer<fx::Resource>> resourceList;

				fx::ResourceManager::GetCurrent()->ForAllResources([&resourceList](fwRefContainer<fx::Resource> resource)
				{
					resourceList.insert({ resource->GetName(), resource });
				});

				if (resourceList.size() < 2)
				{
					ImGui::EndTable();
					ImGui::End();
					return;
				}

				auto avgScriptTime = scriptFrameMetrics.GetAverage();
				double avgScriptMs = (avgScriptTime.count() / 1000.0);

				auto avgFrameTime = gameFrameMetrics.GetAverage();
				double avgFrameMs = (avgFrameTime.count() / 1000.0);

				ImGui::TableNextRow();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f)));

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("[Total CPU]");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.2f", avgScriptMs);

				ImGui::TableSetColumnIndex(2);

				if (avgFrameMs == 0.0f)
				{
					ImGui::Text("-");
				}
				else
				{
					ImGui::Text("%.1f%%", (avgScriptMs / avgFrameMs) * 100.0);
				}

				std::vector<std::tuple<std::string, double, double, int64_t, int64_t>> resourceDatas;

				for (const auto& [ resourceName, resource ] : resourceList)
				{
					auto metric = metrics.find(resourceName);
					double avgTickMs = -1.0;
					double avgFrameFraction = -1.0f;
					int64_t memorySize = -1;
					int64_t streamingSize = -1;

					if (metric != metrics.end() && metric->second)
					{
						const auto& [key, valueRef] = *metric;
						auto value = *valueRef;

						auto avgTickTime = value.ticks.GetAverage();
						avgTickMs = (avgTickTime.count() / 1000.0);

						if (avgScriptMs != 0.0f)
						{
							avgFrameFraction = (avgTickMs / avgScriptMs);
						}

						if (value.memorySize != 0)
						{
							memorySize = value.memorySize;
						}

#ifdef GTA_FIVE
						auto streamingUsage = GetStreamingUsageForThread(value.gtaThread);

						if (streamingUsage > 0)
						{
							streamingSize = streamingUsage;
						}
#endif

						resourceDatas.emplace_back(resource->GetName(), avgTickMs, avgFrameFraction, memorySize, streamingSize);
					}
				}

				{
					auto sortSpecs = ImGui::TableGetSortSpecs();

					if (sortSpecs && sortSpecs->SpecsCount > 0)
					{
						std::sort(resourceDatas.begin(), resourceDatas.end(), [sortSpecs](const auto& left, const auto& right)
						{
							auto compare = [](const auto& left, const auto& right)
							{
								if (left < right)
									return -1;

								if (left > right)
									return 1;

								return 0;
							};

							for (int n = 0; n < sortSpecs->SpecsCount; n++)
							{
								const ImGuiTableSortSpecsColumn* sortSpec = &sortSpecs->Specs[n];
								int delta = 0;
								switch (sortSpec->ColumnIndex)
								{
									case 0:
										delta = compare(std::get<0>(left), std::get<0>(right));
										break;
									case 1:
										delta = compare(std::get<1>(left), std::get<1>(right));
										break;
									case 2:
										delta = compare(std::get<2>(left), std::get<2>(right));
										break;
									case 3:
										delta = compare(std::get<3>(left), std::get<3>(right));
										break;
									case 4:
										delta = compare(std::get<4>(left), std::get<4>(right));
										break;
								}
								if (delta > 0)
									return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
								if (delta < 0)
									return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
							}

							return (left < right);
						});
					}
				}

				for (const auto& [resourceName, avgTickMs, avgFrameFraction, memorySize, streamingUsage] : resourceDatas)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%s", resourceName.c_str());

					ImGui::TableSetColumnIndex(1);

					if (avgTickMs >= 0.0)
					{
						ImGui::TextColored(GetColorForRange(1.0f, 8.0f, avgTickMs), "%.2f ms", avgTickMs);
					}
					else
					{
						ImGui::Text("-");
					}

					ImGui::TableSetColumnIndex(2);

					if (avgFrameFraction < 0.0f)
					{
						ImGui::Text("-");
					}
					else
					{
						ImGui::Text("%.2f%%", avgFrameFraction * 100.0);
					}

					ImGui::TableSetColumnIndex(3);

					int64_t totalBytes = memorySize;

					if (totalBytes == 0 || totalBytes == -1)
					{
						ImGui::Text("?");
					}
					else
					{
						std::string humanSize = fmt::sprintf("%d B", totalBytes);

						if (totalBytes > (1024 * 1024 * 1024))
						{
							humanSize = fmt::sprintf("%.2f GiB", totalBytes / 1024.0 / 1024.0 / 1024.0);
						}
						else if (totalBytes > (1024 * 1024))
						{
							humanSize = fmt::sprintf("%.2f MiB", totalBytes / 1024.0 / 1024.0);
						}
						else if (totalBytes > 1024)
						{
							humanSize = fmt::sprintf("%.2f KiB", totalBytes / 1024.0);
						}

						ImGui::Text("%s+", humanSize.c_str());
					}

					ImGui::TableSetColumnIndex(4);

					if (streamingUsage > 0)
					{
						std::string humanSize = fmt::sprintf("%d B", streamingUsage);

						if (streamingUsage > (1024 * 1024 * 1024))
						{
							humanSize = fmt::sprintf("%.2f GiB", streamingUsage / 1024.0 / 1024.0 / 1024.0);
						}
						else if (streamingUsage > (1024 * 1024))
						{
							humanSize = fmt::sprintf("%.2f MiB", streamingUsage / 1024.0 / 1024.0);
						}
						else if (streamingUsage > 1024)
						{
							humanSize = fmt::sprintf("%.2f KiB", streamingUsage / 1024.0);
						}

						ImGui::Text("%s", humanSize.c_str());
					}
					else
					{
						ImGui::Text("-");
					}
				}

				ImGui::EndTable();
			}

			ImGui::End();
		}
	});
});
