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

namespace fx
{
	class ResourceMonitor
	{
	public:
		typedef std::vector<std::tuple<std::string, double, double, int64_t, int64_t>> ResourceDatas;

	public:
		ResourceMonitor();
		~ResourceMonitor();

		virtual ResourceDatas& GetResourceDatas();
		virtual double GetAvgScriptMs();
		virtual double GetAvgFrameMs();

	public:
		static DEVTOOLS_EXPORT ResourceMonitor* GetCurrent();

		static DEVTOOLS_EXPORT fwEvent<const std::string&> OnWarning;
		static DEVTOOLS_EXPORT fwEvent<> OnWarningGone;
	};
}
