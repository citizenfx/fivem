/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#if __has_include(<scrThread.h>)
#include <scrThread.h>
#include <scrEngine.h>
#endif

#ifdef COMPILING_CITIZEN_DEVTOOLS
#define DEVTOOLS_EXPORT DLL_EXPORT
#else
#define DEVTOOLS_EXPORT DLL_IMPORT
#endif

template<int SampleCount, int MaxSampleCount = SampleCount>
struct TickMetrics
{
	uint64_t lastTickTime = 0;
	std::shared_ptr<uint64_t> curTickTime = 0;
	std::chrono::microseconds tickTimes[MaxSampleCount];

	TickMetrics(const std::shared_ptr<uint64_t>& curTickTime)
		: curTickTime(curTickTime)
	{
		for (auto& tt : tickTimes)
		{
			tt = std::chrono::microseconds{ 0 };
		}
	}

	inline void Append(std::chrono::microseconds time)
	{
		if (!curTickTime)
		{
			return;
		}

		auto ctt = *curTickTime;
		auto cti = ctt % std::size(tickTimes);
		auto lti = (lastTickTime + 1) % std::size(tickTimes);

		// clear out any times between last and current

		// no wraparound
		if (lti < cti)
		{
			for (auto idx = lti; idx < cti; idx++)
			{
				tickTimes[idx] = std::chrono::microseconds{ 0 };
			}
		}
		else if (lti > cti)
		{
			// wraparound: clear end
			for (auto idx = lti; idx < std::size(tickTimes); idx++)
			{
				tickTimes[idx] = std::chrono::microseconds{ 0 };
			}

			// clear start
			for (auto idx = 0; idx < cti; idx++)
			{
				tickTimes[idx] = std::chrono::microseconds{ 0 };
			}
		}

		lastTickTime = ctt;
		tickTimes[cti] = time;
	}

	inline std::chrono::microseconds GetAverage() const
	{
		std::chrono::microseconds avgTickTime(0);

		if (!curTickTime)
		{
			return avgTickTime;
		}

		for (size_t i = 0; i < SampleCount; i++)
		{
			// to prevent inducing underflow, we start off by std::size(tickTimes)
			size_t tickIdx = (std::size(tickTimes) + size_t(*curTickTime) - i) % std::size(tickTimes);
			avgTickTime += tickTimes[tickIdx];
		}

		avgTickTime /= SampleCount;

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
	std::shared_ptr<TickMetrics<64, 200>> ticks;
	std::shared_ptr<TickMetrics<64, 200>> totalTicks;

	std::chrono::microseconds memoryLastFetched{ 0 };

	int64_t memorySize = 0;

#if __has_include(<scrThread.h>)
	GtaThread* gtaThread = nullptr;
#endif

	ResourceMetrics()
		: ticks(std::make_shared<TickMetrics<64, 200>>(nullptr)),
		  totalTicks(std::make_shared<TickMetrics<64, 200>>(nullptr))
	{
	
	}

	ResourceMetrics(const std::shared_ptr<uint64_t>& curTickTime)
		: ticks(std::make_shared<TickMetrics<64, 200>>(curTickTime)),
		  totalTicks(std::make_shared<TickMetrics<64, 200>>(curTickTime))
	{
	
	}
};

namespace fx
{
	class Resource;

	class ResourceMonitorImplBase
	{
	public:
		virtual ~ResourceMonitorImplBase() = default;
	};

	class ResourceMonitorImpl;

	class DEVTOOLS_EXPORT ResourceMonitor
	{
	public:
		// TODO: use an std::string_view to remove the unnecesary copy, and make use of correct spelling
		using ResourceDatas = std::vector<std::tuple<std::string, double, double, int64_t, int64_t, std::shared_ptr<const TickMetrics<64, 200>>, double, std::shared_ptr<const TickMetrics<64, 200>>>>;

	public:
		ResourceMonitor();

		virtual ResourceDatas& GetResourceDatas();
		virtual double GetAvgScriptMs();
		virtual double GetAvgFrameMs();

		inline void SetShouldGetMemory(bool should)
		{
			m_shouldGetMemory = should;
		}

	public:
		static fwEvent<const std::string&> OnWarning;
		static fwEvent<> OnWarningGone;

	private:
		bool m_shouldGetMemory = false;

		std::unordered_map<fx::Resource*, std::tuple<std::chrono::microseconds, std::chrono::microseconds>> m_pendingMetrics;

		ResourceMonitorImpl* GetImpl();

		std::unique_ptr<ResourceMonitorImplBase> m_implStorage;
	};
}
