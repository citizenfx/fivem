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

struct AutoConnect
{
	AutoConnect()
	{
	
	}

	template<typename TEvent, typename TFunc>
	AutoConnect(TEvent& ev, TFunc&& func)
		: AutoConnect(ev, std::move(func), 0)
	{
	}

	template<typename TEvent, typename TFunc>
	AutoConnect(TEvent& ev, TFunc&& func, int order)
	{
		auto cookie = ev.Connect(std::move(func), order);
		dtor = [&ev, cookie]()
		{
			ev.Disconnect(cookie);
		};
	}

	template<typename TEvent, typename TFunc>
	AutoConnect(const std::shared_ptr<bool>& defuser, TEvent& ev, TFunc&& func)
		: AutoConnect(defuser, ev, std::move(func), 0)
	{
	}

	template<typename TEvent, typename TFunc>
	AutoConnect(const std::shared_ptr<bool>& defuser, TEvent& ev, TFunc&& func, int order)
		: AutoConnect(ev, func, order)
	{
		this->defuser = defuser;
	}

	~AutoConnect()
	{
		if (dtor)
		{
			if (!defuser || !*defuser)
			{
				dtor();
			}

			dtor = {};
		}
	}

	AutoConnect(const AutoConnect& other) = delete;
	AutoConnect(AutoConnect&& other) = default;

private:
	std::function<void()> dtor;
	std::shared_ptr<bool> defuser;
};

namespace fx
{
struct ResourceMonitorImpl : public ResourceMonitorImplBase
{
	ResourceMonitorImpl()
		: tickIndex(std::make_shared<size_t>(0)), scriptFrameMetrics(tickIndex), gameFrameMetrics(tickIndex)
	{
		
	}

	virtual ~ResourceMonitorImpl() = default;

	std::shared_ptr<size_t> tickIndex;

	tbb::concurrent_unordered_map<std::string, std::optional<ResourceMetrics>> metrics;
	TickMetrics<64> scriptFrameMetrics;
	TickMetrics<64> gameFrameMetrics;

	double avgScriptMs = 0.0;
	double avgFrameMs = 0.0;

	fx::ResourceMonitor::ResourceDatas resourceDatas;
	bool shouldRecalculateDatas = true;

	std::list<AutoConnect> events;
	std::unordered_map<fx::Resource*, std::list<AutoConnect>> resEvents;
	std::unordered_map<fx::Resource*, std::list<AutoConnect>> resEvents2;

	std::chrono::microseconds scriptBeginTime;

	void RecalculateResourceDatas();
};
}

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

void fx::ResourceMonitorImpl::RecalculateResourceDatas()
{
	resourceDatas.clear();

	auto avgScriptTime = scriptFrameMetrics.GetAverage();
	avgScriptMs = (avgScriptTime.count() / 1000.0);

	auto avgFrameTime = gameFrameMetrics.GetAverage();
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
		auto metric = metrics.find(resourceName);
		double avgTickMs = -1.0;
		double avgFrameFraction = -1.0f;
		int64_t memorySize = -1;
		int64_t streamingSize = -1;

		if (metric != metrics.end() && metric->second)
		{
			const auto& [key, valueRef] = *metric;
			const auto& value = *valueRef;

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

			resourceDatas.emplace_back(resource->GetName(), avgTickMs, avgFrameFraction, memorySize, streamingSize, value.ticks);
		}
	}
}

namespace fx
{
	ResourceMonitor::ResourceMonitor()
	{
		m_implStorage = std::make_unique<ResourceMonitorImpl>();

#ifdef GTA_FIVE
		static auto frameBegin = usec();

		GetImpl()->events.emplace_back(OnBeginGameFrame, [this]()
		{
			if (!m_shouldGetTime)
			{
				return;
			}

			frameBegin = usec();
		});

		GetImpl()->events.emplace_back(OnEndGameFrame, [this]()
		{
			if (!m_shouldGetTime)
			{
				return;
			}

			auto now = usec();
			auto frameTime = now - frameBegin;

			GetImpl()->gameFrameMetrics.Append(frameTime);
		});

		GetImpl()->events.emplace_back(OnDeleteResourceThread, [this](rage::scrThread* thread)
		{
			for (auto& metric : GetImpl()->metrics)
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

		auto resourceManager = fx::ResourceManager::GetCurrent();

		GetImpl()->events.emplace_back(
		resourceManager->OnTick, [this]()
		{
			if (!m_pendingMetrics.empty())
			{
				auto now = usec();

				for (const auto& [resource, duration] : m_pendingMetrics)
				{
					auto& metric = *GetImpl()->metrics[resource->GetName()];
					metric.ticks.Append(duration);

					if ((now - metric.memoryLastFetched) > 500ms && m_shouldGetMemory)
					{
						int64_t totalBytes = GetTotalBytes(resource);

						metric.memorySize = totalBytes;
						metric.memoryLastFetched = now;
					}
				}

				m_pendingMetrics.clear();
			}
		},
		INT32_MAX);

		auto setupResource = [this](fx::Resource* resource)
		{
			auto& resEvents = GetImpl()->resEvents[resource];
			
			{
				auto& resEvents2 = GetImpl()->resEvents2[resource];

				// we use an external defuser here so if OnRemove got used, we do not try to Disconnect on the dead resource when
				// the AutoEvent is removed
				auto defuser = std::make_shared<bool>(false);

				resEvents2.emplace_back(defuser, resource->OnRemove, [this, resource, defuser]()
				{
					auto impl = GetImpl();

					impl->metrics[resource->GetName()] = {};
					impl->resEvents.erase(resource);

					*defuser = true;
				});
			}

			auto processStart = [this, resource]()
			{
				GetImpl()->metrics[resource->GetName()] = ResourceMetrics{ GetImpl()->tickIndex };

				GetImpl()->metrics[resource->GetName()]->ticks.Reset();
			};

			resEvents.emplace_back(resource->OnStart, [processStart]()
			{
				processStart();
			});

			if (resource->GetState() == ResourceState::Started)
			{
				processStart();
			}

#if __has_include(<scrThread.h>)
			resEvents.emplace_back(resource->OnActivate, [this, resource]()
			{
				GetImpl()->metrics[resource->GetName()]->gtaThread = (GtaThread*)rage::scrEngine::GetActiveThread();
			},
			9999);
#endif

			// #TODOTICKLESS: OnActivate tick attribution stack instead on OnEnter/OnLeave only
			auto tickStart = std::make_shared<std::chrono::microseconds>();

			resEvents.emplace_back(resource->OnEnter, [tickStart]()
			{
				*tickStart = usec();
			},
			-99999999);

			resEvents.emplace_back(resource->OnLeave, [this, resource, tickStart]()
			{
				auto now = usec();
				m_pendingMetrics[resource] += now - *tickStart;
			},
			99999999);

			resEvents.emplace_back(resource->OnStop, [this, resource]()
			{
				GetImpl()->metrics[resource->GetName()] = {};
			});

#if __has_include(<scrThread.h>) && __has_include(<ResourceGameLifeTimeEvents.h>)
			resEvents.emplace_back(
			resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnBeforeGameShutdown, [this, resource]()
			{
				auto m = GetImpl()->metrics[resource->GetName()];

				if (m)
				{
					m->gtaThread = nullptr;
				}
			},
			-50);

			resEvents.emplace_back(
			resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnGameDisconnect, [this, resource]()
			{
				auto m = GetImpl()->metrics[resource->GetName()];

				if (m)
				{
					m->gtaThread = nullptr;
				}
			},
			-50);
#endif
		};

		GetImpl()->events.emplace_back(fx::Resource::OnInitializeInstance, [setupResource](fx::Resource* resource)
		{
			setupResource(resource);
		});

		resourceManager->ForAllResources([&setupResource](const fwRefContainer<fx::Resource>& resource)
		{
			setupResource(resource.GetRef());
		});

		GetImpl()->events.emplace_back(resourceManager->OnTick, [this]()
		{
			(*(GetImpl()->tickIndex))++;
			GetImpl()->scriptBeginTime = usec();
		},
		INT32_MIN);

		GetImpl()->events.emplace_back(resourceManager->OnTick, [this]()
		{
			auto scriptEndTime = usec() - GetImpl()->scriptBeginTime;
			GetImpl()->scriptFrameMetrics.Append(scriptEndTime);

			GetImpl()->shouldRecalculateDatas = true;
		},
		INT32_MAX);

		GetImpl()->events.emplace_back(resourceManager->OnTick, [this]()
		{
			bool showWarning = false;
			std::string warningText;

			for (const auto& [key, metricRef] : GetImpl()->metrics)
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
			}

			if (showWarning)
			{
				OnWarning(warningText);
			}
			else
			{
				OnWarningGone();
			}
		});
	}

	ResourceMonitor::ResourceDatas& ResourceMonitor::GetResourceDatas()
	{
		if (GetImpl()->shouldRecalculateDatas)
		{
			GetImpl()->RecalculateResourceDatas();
		}

		return GetImpl()->resourceDatas;
	}

	double ResourceMonitor::GetAvgScriptMs()
	{
		return GetImpl()->avgScriptMs;
	}

	double ResourceMonitor::GetAvgFrameMs()
	{
		return GetImpl()->avgFrameMs;
	}

	ResourceMonitorImpl* ResourceMonitor::GetImpl()
	{
		return static_cast<ResourceMonitorImpl*>(m_implStorage.get());
	}

	fwEvent<const std::string&> ResourceMonitor::OnWarning;
	fwEvent<> ResourceMonitor::OnWarningGone;
}
