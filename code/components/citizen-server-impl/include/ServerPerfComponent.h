#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>

#include <prometheus/registry.h>

namespace fx
{
struct ServerPerformanceMetric
{
	double currentMilliseconds = 0.0;
	double averageMilliseconds = 0.0;
	double maximumMilliseconds = 0.0;
	uint64_t sampleCount = 0;
};

struct ServerPerformanceSnapshot
{
	ServerPerformanceMetric tick;
	ServerPerformanceMetric script;
};

class ServerPerfComponent : public fwRefCountable
{
public:
	inline ServerPerfComponent()
	{
		m_registry = std::make_shared<prometheus::Registry>();
	}

	inline const std::shared_ptr<prometheus::Registry>& GetRegistry() const
	{
		return m_registry;
	}

	inline void ObserveServerTick(std::chrono::steady_clock::duration duration)
	{
		std::lock_guard lock(m_metricsMutex);
		m_tickMetrics.Append(duration);
	}

	inline void ObserveScriptTick(std::chrono::steady_clock::duration duration)
	{
		std::lock_guard lock(m_metricsMutex);
		m_scriptMetrics.Append(duration);
	}

	inline ServerPerformanceSnapshot GetPerformanceSnapshot() const
	{
		std::lock_guard lock(m_metricsMutex);
		return { m_tickMetrics.GetSnapshot(), m_scriptMetrics.GetSnapshot() };
	}

private:
	static constexpr size_t SampleWindow = 64;

	struct RollingMetrics
	{
		void Append(std::chrono::steady_clock::duration duration)
		{
			const double milliseconds = std::chrono::duration<double, std::milli>(duration).count();

			if (size == SampleWindow)
			{
				sum -= samples[next];
			}
			else
			{
				++size;
			}

			samples[next] = milliseconds;
			sum += milliseconds;
			next = (next + 1) % SampleWindow;
			current = milliseconds;
			++count;
		}

		ServerPerformanceMetric GetSnapshot() const
		{
			if (size == 0)
			{
				return {};
			}

			return {
				current,
				sum / size,
				*std::max_element(samples.begin(), samples.begin() + size),
				count
			};
		}

		std::array<double, SampleWindow> samples {};
		size_t next = 0;
		size_t size = 0;
		double current = 0.0;
		double sum = 0.0;
		uint64_t count = 0;
	};

	std::shared_ptr<prometheus::Registry> m_registry;
	mutable std::mutex m_metricsMutex;
	RollingMetrics m_tickMetrics;
	RollingMetrics m_scriptMetrics;
};
}

DECLARE_INSTANCE_TYPE(fx::ServerPerfComponent);
