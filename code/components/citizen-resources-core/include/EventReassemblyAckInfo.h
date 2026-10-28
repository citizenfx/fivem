#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace fx
{
enum class EventReassemblyAckReason : uint8_t
{
	Accepted,
	Duplicate,
	InvalidEnvelope,
	UnknownEvent,
	UnknownTarget,
	FragmentCountMismatch,
	UnsentFragment,
	RetiredCompleted,
	RetiredExpired,
	RetiredDisconnected,
	Count
};

inline constexpr std::array<const char*, static_cast<size_t>(EventReassemblyAckReason::Count)> kEventReassemblyAckReasonNames{
	"accepted", "duplicate", "invalid_envelope", "unknown_event", "unknown_target",
	"fragment_count_mismatch", "unsent_fragment", "retired_completed", "retired_expired", "retired_disconnected"
};

struct EventReassemblyAckInfo
{
	EventReassemblyAckReason reason = EventReassemblyAckReason::InvalidEnvelope;
	bool hasTransfer = false;
	int64_t activeOutboundEvents = -1;
	std::array<char, 65> eventName{};
	uint64_t payloadBytes = 0;
	uint64_t observedSends = 0;
	uint64_t observedRetries = 0;
	uint64_t lastSendAgeMs = 0;
	uint64_t noProgressAgeMs = 0;
	uint64_t retiredAgeMs = 0;
	uint32_t expectedFragments = 0;
	uint32_t acknowledgedFragments = 0;
	uint32_t pendingTargets = 0;
	int bytesPerSecond = 0;
};

class EventReassemblyAckHistory
{
public:
	static constexpr size_t kCapacity = 1024;
	static constexpr uint64_t kLifetimeMs = 120000;

	void Remember(uint64_t eventId, int source, uint64_t now, const EventReassemblyAckInfo& info)
	{
		m_entries[Index(eventId, source)] = { true, eventId, source, now, info };
	}

	bool Find(uint64_t eventId, int source, uint32_t fragments, uint64_t now, EventReassemblyAckInfo& info) const
	{
		const auto& entry = m_entries[Index(eventId, source)];
		if (!entry.valid || entry.eventId != eventId || entry.source != source || now < entry.retiredAt ||
			now - entry.retiredAt >= kLifetimeMs || fragments != entry.info.expectedFragments)
		{
			return false;
		}
		info = entry.info;
		info.retiredAgeMs = now - entry.retiredAt;
		return true;
	}

private:
	struct Entry
	{
		bool valid = false;
		uint64_t eventId = 0;
		int source = 0;
		uint64_t retiredAt = 0;
		EventReassemblyAckInfo info;
	};

	static size_t Index(uint64_t eventId, int source)
	{
		uint64_t key = eventId ^ (uint64_t(static_cast<uint32_t>(source)) * 0x9e3779b97f4a7c15ULL);
		key ^= key >> 33;
		key *= 0xff51afd7ed558ccdULL;
		key ^= key >> 33;
		return key % kCapacity;
	}

	std::array<Entry, kCapacity> m_entries{};
};
}
