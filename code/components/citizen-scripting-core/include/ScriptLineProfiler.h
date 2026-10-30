#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ComponentExport.h"

// Line-level script profiler: script runtimes attribute CPU time to source lines and to
// outgoing calls (natives, exports, MySQL queries, events), and tools read it back as snapshots.
namespace fx::lineprof
{
enum class SampleKind : uint8_t
{
	Lua, // script code at a line
	Runtime, // citizen runtime glue (scheduler.lua, msgpack), blamed on the calling line
	Native, // native call made from a line
	Export, // export/function reference into another resource
	MySQL, // query sent to a MySQL resource, or Lua time spent in its client library
	Event, // TriggerEvent/TriggerClientEvent made from a line
	Json, // json.encode/json.decode made from a line

	Count
};

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) const char* GetKindName(SampleKind kind);

struct SampleKey
{
	uint32_t generation; // runtime instance, so raw source pointers from an old Lua state can't collide
	const void* source;
	int32_t lineDefined;
	int32_t line;
	SampleKind kind;
	uint64_t detailId;

	bool operator==(const SampleKey& other) const
	{
		return generation == other.generation && source == other.source && lineDefined == other.lineDefined && line == other.line && kind == other.kind && detailId == other.detailId;
	}
};

struct SampleKeyHash
{
	size_t operator()(const SampleKey& key) const
	{
		size_t h = std::hash<const void*>()(key.source);
		h ^= (size_t(key.line) << 20) ^ (size_t(key.lineDefined) << 1) ^ (size_t(key.kind) << 60) ^ size_t(key.generation) << 40;
		h ^= std::hash<uint64_t>()(key.detailId) + 0x9e3779b97f4a7c15 + (h << 6) + (h >> 2);
		return h;
	}
};

struct SampleInfo
{
	std::string source; // chunk name without '@', e.g. 'vrp/base.lua'
	std::string function; // best-effort function name
	int32_t lineDefined = 0;
	int32_t lastLineDefined = 0;
	int32_t line = 0;
	SampleKind kind = SampleKind::Lua;
	std::string detail; // native name, export target, SQL text, event name
};

struct SampleStats
{
	uint64_t selfNs = 0; // time spent in this entry, excluding nested script runtimes
	uint64_t inclNs = 0; // including nested runtimes (e.g. event handlers in other resources)
	uint64_t maxNs = 0; // longest single call (calls only)
	uint64_t count = 0; // calls, or samples for code lines
	uint64_t allocBytes = 0; // Lua memory allocated here (whether or not it was freed later)

	// MySQL calls: time from sending the query until its callback ran
	uint64_t waitNs = 0;
	uint64_t waitCount = 0;
	uint64_t maxWaitNs = 0;
};

struct SampleEntry
{
	SampleInfo info;
	SampleStats stats;
};

class COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) ResourceProfile
{
public:
	explicit ResourceProfile(const std::string& name);

	inline const std::string& GetName() const
	{
		return m_name;
	}

	// infoFn is only invoked the first time a key is seen, so it may do expensive lookups.
	template<typename TInfoFn>
	inline void Record(const SampleKey& key, TInfoFn&& infoFn, uint64_t selfNs, uint64_t inclNs, bool isCall, uint64_t allocBytes = 0)
	{
		std::lock_guard _(m_mutex);

		auto it = m_samples.find(key);

		if (it == m_samples.end())
		{
			SampleEntry entry;
			infoFn(entry.info);

			it = m_samples.emplace(key, std::move(entry)).first;
		}

		auto& stats = it->second.stats;
		stats.selfNs += selfNs;
		stats.inclNs += inclNs;
		stats.allocBytes += allocBytes;
		stats.count++;

		if (isCall && inclNs > stats.maxNs)
		{
			stats.maxNs = inclNs;
		}

		m_kindTotals[size_t(key.kind)] += selfNs;
		m_kindAlloc[size_t(key.kind)] += allocBytes;
	}

	// adds time to a key only if it is still present (it may have been cleared by a reset)
	inline bool AddToExisting(const SampleKey& key, uint64_t ns, uint64_t allocBytes)
	{
		std::lock_guard _(m_mutex);

		auto it = m_samples.find(key);

		if (it == m_samples.end())
		{
			return false;
		}

		it->second.stats.selfNs += ns;
		it->second.stats.inclNs += ns;
		it->second.stats.allocBytes += allocBytes;
		m_kindAlloc[size_t(key.kind)] += allocBytes;
		m_kindTotals[size_t(key.kind)] += ns;

		return true;
	}

	inline void AddWait(const SampleKey& key, uint64_t ns)
	{
		std::lock_guard _(m_mutex);

		auto it = m_samples.find(key);

		if (it == m_samples.end())
		{
			return;
		}

		auto& stats = it->second.stats;
		stats.waitNs += ns;
		stats.waitCount++;
		stats.maxWaitNs = std::max(stats.maxWaitNs, ns);

		m_waitNs += ns;
		m_waitCount++;
	}

	void Snapshot(std::vector<SampleEntry>& out);

	void GetKindTotals(uint64_t (&out)[size_t(SampleKind::Count)]);

	void GetWaitTotals(uint64_t& waitNs, uint64_t& waitCount);

	void GetKindAlloc(uint64_t (&out)[size_t(SampleKind::Count)]);

	void Clear();

private:
	std::string m_name;

	std::mutex m_mutex;
	std::unordered_map<SampleKey, SampleEntry, SampleKeyHash> m_samples;
	uint64_t m_kindTotals[size_t(SampleKind::Count)] = {};

	uint64_t m_waitNs = 0;
	uint64_t m_waitCount = 0;

	uint64_t m_kindAlloc[size_t(SampleKind::Count)] = {};
};

// read on every script entry/native call, so it's a plain exported flag rather than a call
extern COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) bool g_enabled;

// Lua instructions between line samples
extern COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) int g_sampleInterval;

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) void SetEnabled(bool enabled);

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) void Reset();

// wall time profiled since the last reset, excluding time spent stopped
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) uint64_t GetRecordedNs();

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) std::shared_ptr<ResourceProfile> GetResourceProfile(const std::string& resourceName);

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) std::vector<std::shared_ptr<ResourceProfile>> GetResourceProfiles();

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) uint32_t NewGeneration();

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) bool IsMySQLResource(std::string_view resourceName);

inline uint64_t Now()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
}
