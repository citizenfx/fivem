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

struct Stopwatch;

namespace fx
{
struct ResourceMonitorImpl : public ResourceMonitorImplBase
{
	ResourceMonitorImpl()
		: tickIndex(std::make_shared<uint64_t>(0)), scriptFrameMetrics(tickIndex), gameFrameMetrics(tickIndex)
	{
		
	}

	virtual ~ResourceMonitorImpl() = default;

	std::shared_ptr<uint64_t> tickIndex;

	std::mutex metricsMutex;
	tbb::concurrent_unordered_map<Resource*, std::shared_ptr<ResourceMetrics>> metrics;
	TickMetrics<64> scriptFrameMetrics;
	TickMetrics<64> gameFrameMetrics;

	double avgScriptMs = 0.0;
	double avgFrameMs = 0.0;

	fx::ResourceMonitor::ResourceDatas resourceDatas;
	bool shouldRecalculateDatas = true;

	std::list<AutoConnect> events;
	std::unordered_map<fx::Resource*, std::list<AutoConnect>> resEvents;
	std::unordered_map<fx::Resource*, std::list<AutoConnect>> resEvents2;

	// a stack of nested resource activations
	std::deque<fx::Resource*> resStack;

	// current resource stopwatches
	tbb::concurrent_unordered_map<fx::Resource*, std::pair<std::atomic<size_t>, Stopwatch>> resWatches;

	std::chrono::microseconds scriptBeginTime{ 0 };

	auto GetMetricFor(const fwRefContainer<fx::Resource>& resource)
	{
		std::unique_lock _(metricsMutex);
		return metrics[resource.GetRef()];
	}

	void RecalculateResourceDatas();
};
}

inline std::chrono::microseconds usec()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

struct Stopwatch
{
	void Start()
	{
		start = resume = usec();
		total = self = std::chrono::microseconds{ 0 };
	}

	void Pause()
	{
		if (resume.count() == 0)
		{
			return;
		}

		auto now = usec();
		self += now - resume;
		resume = std::chrono::microseconds{ 0 };
	}

	void Resume()
	{
		if (resume.count() != 0)
		{
			return;
		}

		resume = usec();
	}

	void Stop()
	{
		if (start.count() == 0)
		{
			return;
		}

		auto now = usec();

		if (resume.count() != 0)
		{
			self += now - resume;
		}

		total += now - start;
		start = resume = std::chrono::microseconds{ 0 };
	}

	auto GetTotal() const
	{
		return total;
	}

	auto GetSelf() const
	{
		return self;
	}

private:
	// the *total* between Start and Stop
	std::chrono::microseconds total{ 0 };

	// the total where this was not paused
	std::chrono::microseconds self{ 0 };

	// timer to indicate the last Resume call
	std::chrono::microseconds resume{ 0 };

	// timer to indicate the last Start call
	std::chrono::microseconds start{ 0 };
};

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
	resourceDatas.clear(); // datas, plural form of the plural form of datum, hmmm...

	// do division by multiplication
	const double inverse1000 = 1.0 / 1000.0;

	avgScriptMs = scriptFrameMetrics.GetAverage().count() * inverse1000;
	avgFrameMs = gameFrameMetrics.GetAverage().count() * inverse1000;

	double avgScriptMsInverse = avgScriptMs != 0.0 ? 1.0 / avgScriptMs : 0.0;
	double avgFrameMsInverse = avgFrameMs != 0.0 ? 1.0 / avgFrameMs : 0.0;

	// TODO: get a *direct* copy of the resources vector
	std::vector<fwRefContainer<fx::Resource>> resourceList;
	fx::ResourceManager::GetCurrent()->ForAllResources([&resourceList](fwRefContainer<fx::Resource> resource)
	{
		resourceList.emplace_back(resource);
	});

	if (resourceList.size() >= 2)
	{
		for (const auto& resource : resourceList)
		{
			auto metric = metrics.find(resource.GetRef());
			if (metric != metrics.end() && metric->second)
			{
				const auto& value = *metric->second;

				double avgTickMs = value.ticks->GetAverage().count() * inverse1000;
				double avgTotalMs = value.totalTicks->GetAverage().count() * inverse1000;
				double avgFrameFraction = avgTickMs * avgScriptMsInverse;

				int64_t memorySize = value.memorySize != 0 ? value.memorySize : -1;
				int64_t streamingSize = -1;
#ifdef GTA_FIVE
				auto streamingUsage = GetStreamingUsageForThread(value.gtaThread);

				if (streamingUsage > 0)
				{
					streamingSize = streamingUsage;
				}
#endif

				resourceDatas.emplace_back(resource->GetName(), avgTickMs, avgFrameFraction, memorySize, streamingSize, value.ticks, avgTotalMs, value.totalTicks);
			}
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
			frameBegin = usec();
		});

		GetImpl()->events.emplace_back(OnEndGameFrame, [this]()
		{
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
		resourceManager->OnTick, [this, resourceManager]()
		{
			auto now = usec();
				
			resourceManager->ForAllResources([this, now](const fwRefContainer<fx::Resource>& resource)
			{
				auto& metricRef = GetImpl()->GetMetricFor(resource);
					
				if (metricRef)
				{
					auto& metric = *metricRef;

					if (auto it = m_pendingMetrics.find(resource.GetRef()); it != m_pendingMetrics.end())
					{
						metric.ticks->Append(std::get<0>(it->second));
						metric.totalTicks->Append(std::get<1>(it->second));

						if ((now - metric.memoryLastFetched) > 500ms && m_shouldGetMemory)
						{
							int64_t totalBytes = GetTotalBytes(resource);

							metric.memorySize = totalBytes;
							metric.memoryLastFetched = now;
						}
					}
					else
					{
						metric.ticks->Append(std::chrono::microseconds{ 0 });
						metric.totalTicks->Append(std::chrono::microseconds{ 0 });
					}
				}
			});

			m_pendingMetrics.clear();
		},
		INT32_MAX);

		auto setupResource = [this](fx::Resource* resource)
		{
			// add resource
			(void)GetImpl()->resWatches[resource];

			auto& resEvents = GetImpl()->resEvents[resource];
			
			{
				auto& resEvents2 = GetImpl()->resEvents2[resource];

				// we use an external defuser here so if OnRemove got used, we do not try to Disconnect on the dead resource when
				// the AutoEvent is removed
				auto defuser = std::make_shared<bool>(false);

				resEvents2.emplace_back(defuser, resource->OnRemove, [this, resource, defuser]()
				{
					auto impl = GetImpl();

					{
						std::unique_lock _(impl->metricsMutex);
						impl->metrics[resource] = {};
					}

					impl->resEvents.erase(resource);
					(void)impl->resWatches[resource];

					*defuser = true;
				});
			}

			auto processStart = [this, resource]()
			{
				auto impl = GetImpl();

				auto m = std::make_shared<ResourceMetrics>(impl->tickIndex);
				m->ticks->Reset();
				m->totalTicks->Reset();

				std::unique_lock _(impl->metricsMutex);
				impl->metrics[resource] = std::move(m);
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
				auto impl = GetImpl();

				if (auto metric = impl->GetMetricFor(resource))
				{
					metric->gtaThread = (GtaThread*)rage::scrEngine::GetActiveThread();
				}
			},
			9999);
#endif

			resEvents.emplace_back(resource->OnActivate, [this, resource]()
			{
				auto impl = GetImpl();

				// pause the current top resource
				if (!impl->resStack.empty())
				{
					auto topResource = impl->resStack.front();

					auto watch = impl->resWatches.find(topResource);
					if (watch != impl->resWatches.end())
					{
						watch->second.second.Pause();
					}
				}

				// push this resource
				impl->resStack.push_front(resource);

				// check if it was already in, if not, start the stopwatch
				auto& watch = impl->resWatches[resource];
				if (++watch.first == 1)
				{
					watch.second.Start();
				}
			},
			-99999999);

			resEvents.emplace_back(resource->OnDeactivate, [this, resource]()
			{
				auto impl = GetImpl();

				// broken? (double pop)
				if (impl->resStack.empty())
				{
					//impl->resSet.clear();
					return;
				}

				// pop (this) resource
				impl->resStack.pop_front();

				// resume the last resource
				if (!impl->resStack.empty())
				{
					auto topResource = impl->resStack.front();
					
					auto& watch = impl->resWatches[topResource];
					watch.second.Resume();
				}

				// remove one of us from the set
				auto& watch = impl->resWatches[resource];
				size_t count = --watch.first;

				// if we're gone, stop and append
				if (count == 0)
				{
					watch.second.Stop();

					auto& pending = m_pendingMetrics[resource];
					std::get<0>(pending) += watch.second.GetSelf();
					std::get<1>(pending) += watch.second.GetTotal();
				}
			},
			99999999);

			resEvents.emplace_back(resource->OnStop, [this, resource]()
			{
				auto impl = GetImpl();

				std::unique_lock _(impl->metricsMutex);
				impl->metrics[resource] = {};
			});

#if __has_include(<scrThread.h>) && __has_include(<ResourceGameLifeTimeEvents.h>)
			resEvents.emplace_back(
			resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnBeforeGameShutdown, [this, resource]()
			{
				auto m = GetImpl()->GetMetricFor(resource);

				if (m)
				{
					m->gtaThread = nullptr;
				}
			},
			-50);

			resEvents.emplace_back(
			resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnGameDisconnect, [this, resource]()
			{
				auto m = GetImpl()->GetMetricFor(resource);

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

			for (const auto& [resource, metricRef] : GetImpl()->metrics)
			{
				if (metricRef)
				{
					auto avgTickTime = metricRef->ticks->GetAverage();

					if (avgTickTime > 6ms)
					{
						const double targetFPS = 60.0;
						const double fixedDeltaTime = 1000.0 / targetFPS;
						const double fixedDeltaTimeInverse = 1.0 / fixedDeltaTime;

						double elapsedTimeMs = avgTickTime.count() * (1.0 / 1000.0);
						float elapsedFPS = elapsedTimeMs * fixedDeltaTimeInverse;

						showWarning = true;
						warningText += fmt::sprintf("%s is taking %.2f ms (or -%.1f FPS @ 60 Hz)\n", resource->GetName(), elapsedTimeMs, elapsedFPS);
					}
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
