#include <StdInc.h>

#include <ResourceMonitor.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <ConsoleHost.h>
#include <ResourceScriptingComponent.h>

#if __has_include(<ResourceGameLifetimeEvents.h>)
#include <ResourceGameLifetimeEvents.h>
#endif

#include <CoreConsole.h>

#include <chrono>

#ifdef GTA_FIVE
#include <Streaming.h>
#include <ScriptHandlerMgr.h>

#include <nutsnbolts.h>
#endif

using namespace std::chrono_literals;


static fx::ResourceMonitor* g_globalResourceMonitor;

static tbb::concurrent_unordered_map<std::string, std::optional<ResourceMetrics>> g_metrics;
static TickMetrics<64> g_scriptFrameMetrics;
static TickMetrics<64> g_gameFrameMetrics;

static double g_avgScriptMs;
static double g_avgFrameMs;

static fx::ResourceMonitor::ResourceDatas g_resourceDatas;
static bool g_shouldRecalculateDatas = true;


inline std::chrono::microseconds usec()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
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

static void RecalculateResourceDatas()
{
	g_resourceDatas.clear();

	auto avgScriptTime = g_scriptFrameMetrics.GetAverage();
	g_avgScriptMs = (avgScriptTime.count() / 1000.0);

	auto avgFrameTime = g_gameFrameMetrics.GetAverage();
	double g_avgFrameMs = (avgFrameTime.count() / 1000.0);

	std::map<std::string, fwRefContainer<fx::Resource>> resourceList;

	fx::ResourceManager::GetCurrent()->ForAllResources([&resourceList](fwRefContainer<fx::Resource> resource)
	{
		resourceList.insert({ resource->GetName(), resource });
	});

	if (resourceList.size() < 2)
	{
		return;
	}

	for (const auto& [resourceName, resource] : resourceList)
	{
		auto metric = g_metrics.find(resourceName);
		double avgTickMs = -1.0;
		double avgFrameFraction = -1.0f;
		int64_t memorySize = -1;
		int64_t streamingSize = -1;

		if (metric != g_metrics.end() && metric->second)
		{
			const auto& [key, valueRef] = *metric;
			auto value = *valueRef;

			auto avgTickTime = value.ticks.GetAverage();
			avgTickMs = (avgTickTime.count() / 1000.0);

			if (g_avgScriptMs != 0.0f)
			{
				avgFrameFraction = (avgTickMs / g_avgScriptMs);
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

			g_resourceDatas.emplace_back(resource->GetName(), avgTickMs, avgFrameFraction, memorySize, streamingSize);
		}
	}
}

namespace fx
{
	ResourceMonitor::ResourceMonitor()
	{
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

			g_gameFrameMetrics.Append(frameTime);
		});

		OnDeleteResourceThread.Connect([](rage::scrThread* thread)
		{
			for (auto& metric : g_metrics)
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
				g_metrics[resource->GetName()] = ResourceMetrics{};

				g_metrics[resource->GetName()]->ticks.Reset();
			});

#if __has_include(<scrThread.h>)
			resource->OnActivate.Connect([resource]()
			{
					g_metrics[resource->GetName()]->gtaThread = (GtaThread*)rage::scrEngine::GetActiveThread();
			},
			9999);
#endif

			resource->OnTick.Connect([resource]()
			{
				g_metrics[resource->GetName()]->tickStart = usec();
			},
			-99999999);

			resource->OnTick.Connect([resource]()
			{
				auto& metric = *g_metrics[resource->GetName()];
				metric.ticks.Append(usec() - metric.tickStart);

				if ((usec() - metric.memoryLastFetched) > 500ms)
				{
					int64_t totalBytes = GetTotalBytes(resource);

					metric.memorySize = totalBytes;
					metric.memoryLastFetched = usec();
				}
			},
			99999999);

			resource->OnStop.Connect([resource]()
			{
				g_metrics[resource->GetName()] = {};
			});

#if __has_include(<scrThread.h>) && __has_include(<ResourceGameLifeTimeEvents.h>)
			resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnBeforeGameShutdown.Connect([resource]()
			{
				auto m = g_metrics[resource->GetName()];

				if (m)
				{
					m->gtaThread = nullptr;
				}
			},
			-50);

			resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnGameDisconnect.Connect([resource]()
			{
				auto m = g_metrics[resource->GetName()];

				if (m)
				{
					m->gtaThread = nullptr;
				}
			},
			-50);
#endif
		});

		fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
		{
			static std::chrono::microseconds scriptBeginTime;

			manager->OnTick.Connect([]()
			{
				scriptBeginTime = usec();
			},
			INT32_MIN);

			manager->OnTick.Connect([]()
			{
				auto scriptEndTime = usec() - scriptBeginTime;
				g_scriptFrameMetrics.Append(scriptEndTime);

				g_shouldRecalculateDatas = true;
			},
			INT32_MAX);

			manager->OnTick.Connect([]()
			{
				bool showWarning = false;
				std::string warningText;

				for (const auto& [key, metricRef] : g_metrics)
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

					if (metric.memorySize > (75 * 1024 * 1024))
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
					OnWarning(warningText);
				}
				else
				{
					OnWarningGone();
				}
			});
		});
	}

	ResourceMonitor::ResourceDatas& ResourceMonitor::GetResourceDatas()
	{
		if (g_shouldRecalculateDatas)
		{
			RecalculateResourceDatas();
		}

		return g_resourceDatas;
	}

	double ResourceMonitor::GetAvgScriptMs()
	{
		return g_avgScriptMs;
	}

	double ResourceMonitor::GetAvgFrameMs()
	{
		return g_avgFrameMs;
	}

	ResourceMonitor* ResourceMonitor::GetCurrent()
	{
		if (!g_globalResourceMonitor)
		{
			g_globalResourceMonitor = new ResourceMonitor();
		}

		return g_globalResourceMonitor;
	}

	fwEvent<const std::string&> ResourceMonitor::OnWarning;
	fwEvent<> ResourceMonitor::OnWarningGone;
}
