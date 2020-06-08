#include "StdInc.h"

#include <Resource.h>
#include <ResourceManager.h>
#include <ConsoleHost.h>
#include <ResourceScriptingComponent.h>
#include <ResourceGameLifetimeEvents.h>

#include <CoreConsole.h>

#include <imgui.h>
#include <imguilistview.h>

#include <chrono>

#include <ScriptHandlerMgr.h>
#include <scrThread.h>
#include <scrEngine.h>

#include <Streaming.h>

using namespace std::chrono_literals;

inline std::chrono::microseconds usec()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

struct ResourceMetrics
{
	std::chrono::microseconds tickStart;

	int curTickTime = 0;
	std::chrono::microseconds tickTimes[64];

	std::chrono::microseconds memoryLastFetched;

	int64_t memorySize;

	GtaThread* gtaThread;
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

static InitFunction initFunction([]()
{
	static bool resourceTimeWarningShown;
	static std::chrono::microseconds warningLastShown;
	static std::string resourceTimeWarningText;
	static std::mutex mutex;

	static bool taskMgrEnabled;

	static ConVar<bool> taskMgrVar("resmon", ConVar_Archive, false, &taskMgrEnabled);

	static tbb::concurrent_unordered_map<std::string, std::optional<ResourceMetrics>> metrics;

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

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnStart.Connect([resource]()
		{
			metrics[resource->GetName()] = ResourceMetrics{};

			for (auto& tt : metrics[resource->GetName()]->tickTimes)
			{
				tt = std::chrono::microseconds{ 0 };
			}
		});

		resource->OnActivate.Connect([resource]()
		{
			metrics[resource->GetName()]->gtaThread = (GtaThread*)rage::scrEngine::GetActiveThread();
		}, 9999);

		resource->OnTick.Connect([resource]()
		{
			metrics[resource->GetName()]->tickStart = usec();
		}, -99999999);

		resource->OnTick.Connect([resource]()
		{
			auto& metric = *metrics[resource->GetName()];
			metric.tickTimes[metric.curTickTime++] = usec() - metric.tickStart;

			if (metric.curTickTime >= _countof(metric.tickTimes))
			{
				metric.curTickTime = 0;
			}

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
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
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

				std::chrono::microseconds avgTickTime(0);

				for (auto tickTime : metric.tickTimes)
				{
					avgTickTime += tickTime;
				}

				avgTickTime /= std::size(metric.tickTimes);

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

			if (showWarning)
			{
				std::unique_lock<std::mutex> lock(mutex);
				
				resourceTimeWarningShown = true;
				warningLastShown = usec();
				resourceTimeWarningText = warningText;
			}
			else
			{
				resourceTimeWarningShown = false;
			}
		});
	});

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || ((usec() - warningLastShown) < 1s) || taskMgrEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if ((usec() - warningLastShown) < 1s)
		{
			const float DISTANCE = 10.0f;
			ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - DISTANCE, ImGui::GetIO().DisplaySize.y - DISTANCE);
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
				ImGui::End();
			}
		}

		if (taskMgrEnabled)
		{
			if (ImGui::Begin("Resource Monitor"))
			{
				ImGui::Columns(4);

				ImGui::Text("Resource");
				ImGui::NextColumn();

				ImGui::Text("CPU msec");
				ImGui::NextColumn();

				ImGui::Text("Memory");
				ImGui::NextColumn();

				ImGui::Text("Streaming");
				ImGui::NextColumn();

				std::map<std::string, fwRefContainer<fx::Resource>> resourceList;

				Instance<fx::ResourceManager>::Get()->ForAllResources([&resourceList](fwRefContainer<fx::Resource> resource)
				{
					resourceList.insert({ resource->GetName(), resource });
				});

				for (const auto& resourcePair : resourceList)
				{
					auto[resourceName, resource] = resourcePair;

					ImGui::Text("%s", resource->GetName().c_str());
					ImGui::NextColumn();

					auto metric = metrics.find(resourceName);

					if (metric != metrics.end() && metric->second)
					{
						const auto& [ key, valueRef ] = *metric;
						auto value = *valueRef;

						std::chrono::microseconds avgTickTime(0);

						for (auto tickTime : value.tickTimes)
						{
							avgTickTime += tickTime;
						}

						avgTickTime /= std::size(value.tickTimes);

						ImGui::TextColored(GetColorForRange(1.0f, 8.0f, avgTickTime.count() / 1000.0), "%.2f ms", avgTickTime.count() / 1000.0);

						ImGui::NextColumn();

						int64_t totalBytes = value.memorySize;

						if (totalBytes == 0)
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

						ImGui::NextColumn();

						auto streamingUsage = GetStreamingUsageForThread(value.gtaThread);

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

						ImGui::NextColumn();
					}
					else
					{
						ImGui::Text("?");
						ImGui::NextColumn();

						ImGui::Text("?");
						ImGui::NextColumn();

						ImGui::Text("?");
						ImGui::NextColumn();
					}
				}
			}

			ImGui::End();
		}
	});
});
