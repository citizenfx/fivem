#include "StdInc.h"
#include "ScriptLineProfiler.h"

#include <CoreConsole.h>

#include <shared_mutex>

namespace fx::lineprof
{
bool g_enabled;
int g_sampleInterval = 1000;

static std::shared_mutex g_profilesMutex;
static std::unordered_map<std::string, std::shared_ptr<ResourceProfile>> g_profiles;

static std::atomic<uint32_t> g_generation;

static std::mutex g_clockMutex;
static uint64_t g_recordedNs;
static uint64_t g_recordStart;

static std::mutex g_mysqlMutex;
static std::vector<std::string> g_mysqlResources{ "oxmysql", "mysql-async", "ghmattimysql" };

const char* GetKindName(SampleKind kind)
{
	switch (kind)
	{
		case SampleKind::Lua:
			return "Lua";
		case SampleKind::Runtime:
			return "Runtime";
		case SampleKind::Native:
			return "Native";
		case SampleKind::Export:
			return "Export";
		case SampleKind::MySQL:
			return "MySQL";
		case SampleKind::Event:
			return "Event";
		case SampleKind::Json:
			return "JSON";
	}

	return "?";
}

ResourceProfile::ResourceProfile(const std::string& name)
	: m_name(name)
{
}

void ResourceProfile::Snapshot(std::vector<SampleEntry>& out)
{
	std::lock_guard _(m_mutex);

	out.reserve(out.size() + m_samples.size());

	for (const auto& [key, entry] : m_samples)
	{
		out.push_back(entry);
	}
}

void ResourceProfile::GetKindTotals(uint64_t (&out)[size_t(SampleKind::Count)])
{
	std::lock_guard _(m_mutex);

	std::copy(std::begin(m_kindTotals), std::end(m_kindTotals), std::begin(out));
}

void ResourceProfile::GetWaitTotals(uint64_t& waitNs, uint64_t& waitCount)
{
	std::lock_guard _(m_mutex);

	waitNs = m_waitNs;
	waitCount = m_waitCount;
}

void ResourceProfile::GetKindAlloc(uint64_t (&out)[size_t(SampleKind::Count)])
{
	std::lock_guard _(m_mutex);

	std::copy(std::begin(m_kindAlloc), std::end(m_kindAlloc), std::begin(out));
}

void ResourceProfile::Clear()
{
	std::lock_guard _(m_mutex);

	m_samples.clear();
	std::fill(std::begin(m_kindTotals), std::end(m_kindTotals), 0);
	m_waitNs = 0;
	m_waitCount = 0;
	std::fill(std::begin(m_kindAlloc), std::end(m_kindAlloc), 0);
}

void SetEnabled(bool enabled)
{
	std::lock_guard _(g_clockMutex);

	if (enabled == g_enabled)
	{
		return;
	}

	if (enabled)
	{
		g_recordStart = Now();
	}
	else
	{
		g_recordedNs += Now() - g_recordStart;
	}

	g_enabled = enabled;
}

void Reset()
{
	{
		std::shared_lock _(g_profilesMutex);

		for (auto& [name, profile] : g_profiles)
		{
			profile->Clear();
		}
	}

	std::lock_guard _(g_clockMutex);
	g_recordedNs = 0;
	g_recordStart = Now();
}

uint64_t GetRecordedNs()
{
	std::lock_guard _(g_clockMutex);

	return g_recordedNs + (g_enabled ? (Now() - g_recordStart) : 0);
}

std::shared_ptr<ResourceProfile> GetResourceProfile(const std::string& resourceName)
{
	{
		std::shared_lock _(g_profilesMutex);

		if (auto it = g_profiles.find(resourceName); it != g_profiles.end())
		{
			return it->second;
		}
	}

	std::unique_lock _(g_profilesMutex);

	auto& profile = g_profiles[resourceName];

	if (!profile)
	{
		profile = std::make_shared<ResourceProfile>(resourceName);
	}

	return profile;
}

std::vector<std::shared_ptr<ResourceProfile>> GetResourceProfiles()
{
	std::shared_lock _(g_profilesMutex);

	std::vector<std::shared_ptr<ResourceProfile>> profiles;
	profiles.reserve(g_profiles.size());

	for (auto& [name, profile] : g_profiles)
	{
		profiles.push_back(profile);
	}

	return profiles;
}

uint32_t NewGeneration()
{
	return ++g_generation;
}

bool IsMySQLResource(std::string_view resourceName)
{
	std::lock_guard _(g_mysqlMutex);

	for (const auto& name : g_mysqlResources)
	{
		if (name == resourceName)
		{
			return true;
		}
	}

	return false;
}

static void DumpProfiles(const std::string& filter, int limit)
{
	double seconds = GetRecordedNs() / 1e9;

	if (seconds <= 0.0)
	{
		console::Printf("resprof", "Nothing recorded yet. Use ^2resprof_start^7 first.\n");
		return;
	}

	for (auto& profile : GetResourceProfiles())
	{
		if (!filter.empty() && profile->GetName() != filter)
		{
			continue;
		}

		std::vector<SampleEntry> entries;
		profile->Snapshot(entries);

		if (entries.empty())
		{
			continue;
		}

		uint64_t totals[size_t(SampleKind::Count)];
		profile->GetKindTotals(totals);

		uint64_t total = 0;

		for (auto t : totals)
		{
			total += t;
		}

		std::string breakdown;

		for (size_t i = 0; i < size_t(SampleKind::Count); i++)
		{
			if (totals[i] > 0)
			{
				breakdown += fmt::sprintf(" %s %.1f%%", GetKindName(SampleKind(i)), totals[i] * 100.0 / total);
			}
		}

		console::Printf("resprof", "^3%s^7: %.3f ms/s -%s\n", profile->GetName(), total / 1e6 / seconds, breakdown);

		std::sort(entries.begin(), entries.end(), [](const SampleEntry& a, const SampleEntry& b)
		{
			return a.stats.selfNs > b.stats.selfNs;
		});

		for (int i = 0; i < std::min<int>(limit, entries.size()); i++)
		{
			const auto& [info, stats] = entries[i];

			console::Printf("resprof", "  %7.3f ms/s  %-7s %s:%d (%s [%d-%d])%s%s\n",
				stats.selfNs / 1e6 / seconds,
				GetKindName(info.kind),
				info.source,
				info.line,
				info.function,
				info.lineDefined,
				info.lastLineDefined,
				info.detail.empty() ? "" : " -> ",
				info.detail);
		}
	}
}

static InitFunction initFunction([]()
{
	static ConVar<int> intervalVar("resprof_interval", ConVar_None, 1000, &g_sampleInterval);

	static ConVar<std::string> mysqlVar("resprof_mysqlResources", ConVar_None, "oxmysql,mysql-async,ghmattimysql");
	static auto mysqlVarRef = &mysqlVar;

	mysqlVar.GetHelper()->SetChangeCallback([](auto)
	{
		const std::string value = mysqlVarRef->GetValue();
		std::vector<std::string> names;

		for (size_t start = 0; start <= value.size();)
		{
			size_t end = value.find(',', start);

			if (end == std::string::npos)
			{
				end = value.size();
			}

			if (end > start)
			{
				names.emplace_back(value.substr(start, end - start));
			}

			start = end + 1;
		}

		std::lock_guard _(g_mysqlMutex);
		g_mysqlResources = std::move(names);
	});

	static ConsoleCommand startCmd("resprof_start", []()
	{
		SetEnabled(true);
		console::Printf("resprof", "Line profiler started.\n");
	});

	static ConsoleCommand stopCmd("resprof_stop", []()
	{
		SetEnabled(false);
		console::Printf("resprof", "Line profiler stopped.\n");
	});

	static ConsoleCommand resetCmd("resprof_reset", []()
	{
		Reset();
	});

	static ConsoleCommand dumpCmd("resprof_dump", []()
	{
		DumpProfiles({}, 10);
	});

	static ConsoleCommand dumpResCmd("resprof_dump", [](const std::string& resource)
	{
		DumpProfiles(resource, 40);
	});
});
}
