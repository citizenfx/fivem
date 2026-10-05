#pragma once

#include <EventReassemblyAckInfo.h>
#include <mutex>

namespace fx
{
struct LatentEventAckWindow
{
	static constexpr uint64_t kWindowMs = 5000;
	static constexpr uint64_t kLogIntervalMs = 10000;

	void Observe(uint64_t now, EventReassemblyAckReason reason, bool softLimited, bool overflow)
	{
		if (!initialized || now - startedAt >= kWindowMs)
		{
			counts = {};
			softLimits = overflows = 0;
			maxGapMs = 0;
			startedAt = now;
		}
		lastGapMs = initialized ? now - lastAckAt : 0;
		if (lastGapMs > maxGapMs)
		{
			maxGapMs = lastGapMs;
		}
		initialized = true;
		lastAckAt = now;
		++counts[static_cast<size_t>(reason)];
		softLimits += softLimited ? 1 : 0;
		overflows += overflow ? 1 : 0;
	}

	bool ShouldLog(uint64_t now, bool overflow)
	{
		const auto index = overflow ? 1 : 0;
		if (logged[index] && now - lastLogAt[index] < kLogIntervalMs)
		{
			return false;
		}
		logged[index] = true;
		lastLogAt[index] = now;
		return true;
	}

	std::array<uint64_t, static_cast<size_t>(EventReassemblyAckReason::Count)> counts{};
	uint64_t startedAt = 0;
	uint64_t lastAckAt = 0;
	uint64_t lastGapMs = 0;
	uint64_t maxGapMs = 0;
	uint64_t softLimits = 0;
	uint64_t overflows = 0;
	bool initialized = false;

private:
	std::array<uint64_t, 2> lastLogAt{};
	std::array<bool, 2> logged{};
};

class LatentEventAckLogBudget
{
public:
	static constexpr uint32_t kLinesPerSecondPerKind = 4;

	bool Consume(uint64_t now, bool overflow)
	{
		std::lock_guard lock(m_mutex);
		auto& bucket = m_buckets[overflow ? 1 : 0];
		if (now - bucket.startedAt >= 1000)
		{
			bucket.startedAt = now;
			bucket.lines = 0;
		}
		if (bucket.lines >= kLinesPerSecondPerKind)
		{
			return false;
		}
		++bucket.lines;
		return true;
	}

private:
	struct Bucket
	{
		uint64_t startedAt = 0;
		uint32_t lines = 0;
	};
	std::mutex m_mutex;
	std::array<Bucket, 2> m_buckets{};
};
}
