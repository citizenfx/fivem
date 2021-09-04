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
	int curTickTime = 0;
	std::chrono::microseconds tickTimes[MaxSampleCount];

	inline void Append(std::chrono::microseconds time)
	{
		tickTimes[curTickTime++] = time;

		if (curTickTime >= std::size(tickTimes))
		{
			curTickTime = 0;
		}
	}

	inline std::chrono::microseconds GetAverage() const
	{
		std::chrono::microseconds avgTickTime(0);

		for (size_t i = 0; i < SampleCount; i++)
		{
			// to prevent inducing underflow, we start off by std::size(tickTimes)
			size_t tickIdx = (std::size(tickTimes) + size_t(curTickTime) - i) % std::size(tickTimes);
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
	std::chrono::microseconds tickStart;
	TickMetrics<64, 200> ticks;

	std::chrono::microseconds memoryLastFetched;

	int64_t memorySize;

#if __has_include(<scrThread.h>)
	GtaThread* gtaThread;
#endif
};

namespace fx
{
	class ResourceMonitor
	{
	public:
		typedef std::vector<std::tuple<std::string, double, double, int64_t, int64_t, std::reference_wrapper<const TickMetrics<64, 200>>>> ResourceDatas;

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
		static DEVTOOLS_EXPORT ResourceMonitor* GetCurrent();

		static DEVTOOLS_EXPORT fwEvent<const std::string&> OnWarning;
		static DEVTOOLS_EXPORT fwEvent<> OnWarningGone;

	private:
		bool m_shouldGetMemory = false;
	};
}
