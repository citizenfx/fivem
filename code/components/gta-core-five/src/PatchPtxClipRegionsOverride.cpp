#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.Patterns.h>
#include <CrossBuildRuntime.h>
#include <GameInit.h>
#include <VFSManager.h>

#include <atomic>
#include <charconv>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>
#include <vector>

//
// Implements the `replace_ptxclipregions_file` manifest statement: a resource can replace
// ptxclipregions.dat, the particle texture UV clip-region table. The game caches the pointers
// rage::ptxClipRegions::GetData returns, so we hook GetData and serve overrides from our own store
// rather than rebuilding the game's table.
//

// 16-byte aligned to match rage::Vec4V (uMin, uMax, vMin, vMax).
struct alignas(16) ClipVec4
{
	float v[4];
};

struct ClipRegionEntry
{
	const ClipVec4* data;
	int numColumns;
	int numRows;
};

struct ParsedClipRegion
{
	uint32_t hash;
	std::vector<ClipVec4> regions;
	int numColumns;
	int numRows;
};

static std::unordered_map<uint32_t, ClipRegionEntry> g_store;

static std::deque<std::vector<ClipVec4>> g_regionPools;

static std::shared_mutex g_storeMutex;
static std::atomic<bool> g_overrideActive{ false };
static std::atomic<bool> g_hookInstalled{ false };

// rage::ptxClipRegions::GetData.
static bool (*g_origGetClipRegionData)(uint32_t, const ClipVec4**, int*, int*);

class ClipRegionTokenizer
{
public:
	ClipRegionTokenizer(const uint8_t* data, size_t size)
		: m_p(reinterpret_cast<const char*>(data)), m_end(m_p + size)
	{
	}

	bool Next(std::string_view& out)
	{
		while (m_p < m_end && IsSpace(*m_p))
		{
			m_p++;
		}
		if (m_p >= m_end)
		{
			return false;
		}
		const char* start = m_p;
		while (m_p < m_end && !IsSpace(*m_p))
		{
			m_p++;
		}
		out = std::string_view(start, m_p - start);
		return true;
	}

	template<typename T>
	bool NextNumber(T& out)
	{
		std::string_view token;
		if (!Next(token))
		{
			return false;
		}
		auto [end, ec] = std::from_chars(token.data(), token.data() + token.size(), out);
		return ec == std::errc() && end == token.data() + token.size();
	}

private:
	static bool IsSpace(char c)
	{
		return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
	}

	const char* m_p;
	const char* m_end;
};

constexpr int kMaxClipRegionTextures = 100000;
constexpr int kMaxClipRegionDimension = 4096;

// Format: <numTextures> <numClipRegionsTotal>, then per texture <name> <numColumns> <numRows>
// followed by numColumns*numRows "uMin uMax vMin vMax" quads.
static bool ParseClipRegions(const std::vector<uint8_t>& bytes, std::vector<ParsedClipRegion>& out)
{
	ClipRegionTokenizer token(bytes.data(), bytes.size());

	int numTextures = 0;
	int numClipRegionsTotal = 0;
	if (!token.NextNumber(numTextures) || !token.NextNumber(numClipRegionsTotal))
	{
		return false;
	}
	if (numTextures < 0 || numTextures > kMaxClipRegionTextures)
	{
		return false;
	}

	out.reserve(numTextures);
	int regionsLoaded = 0;
	for (int i = 0; i < numTextures; i++)
	{
		std::string_view name;
		int numColumns = 0;
		int numRows = 0;
		if (!token.Next(name) || !token.NextNumber(numColumns) || !token.NextNumber(numRows))
		{
			return false;
		}
		if (numColumns <= 0 || numRows <= 0 || numColumns > kMaxClipRegionDimension || numRows > kMaxClipRegionDimension)
		{
			return false;
		}

		int numFrames = numColumns * numRows;
		ParsedClipRegion entry;
		entry.hash = HashString(name); // must match rage's atHashValue for the texture name
		entry.numColumns = numColumns;
		entry.numRows = numRows;
		entry.regions.resize(numFrames);
		for (int f = 0; f < numFrames; f++)
		{
			if (!token.NextNumber(entry.regions[f].v[0]) || !token.NextNumber(entry.regions[f].v[1])
				|| !token.NextNumber(entry.regions[f].v[2]) || !token.NextNumber(entry.regions[f].v[3]))
			{
				return false;
			}
		}

		regionsLoaded += numFrames;
		out.push_back(std::move(entry));
	}

	return regionsLoaded == numClipRegionsTotal;
}

static bool GetClipRegionData(uint32_t hash, const ClipVec4** outData, int* outColumns, int* outRows)
{
	if (g_overrideActive.load(std::memory_order_acquire))
	{
		std::shared_lock<std::shared_mutex> lock(g_storeMutex);
		auto it = g_store.find(hash);
		if (it != g_store.end())
		{
			*outData = it->second.data;
			*outColumns = it->second.numColumns;
			*outRows = it->second.numRows;
			return true;
		}
	}

	return g_origGetClipRegionData(hash, outData, outColumns, outRows);
}

static void ApplyOverride(const std::string& path)
{
	fwRefContainer<vfs::Stream> stream = vfs::OpenRead(path);
	if (!stream.GetRef())
	{
		trace("replace_ptxclipregions_file: could not open %s\n", path.c_str());
		return;
	}

	std::vector<uint8_t> bytes = stream->ReadToEnd();

	std::vector<ParsedClipRegion> parsed;
	if (!ParseClipRegions(bytes, parsed))
	{
		trace("replace_ptxclipregions_file: could not parse %s; keeping current clip regions\n", path.c_str());
		return;
	}

	{
		std::unique_lock<std::shared_mutex> lock(g_storeMutex);

		// Replace the lookups only; earlier pools stay alive (see g_regionPools).
		g_store.clear();
		for (ParsedClipRegion& entry : parsed)
		{
			g_regionPools.push_back(std::move(entry.regions));
			g_store[entry.hash] = { g_regionPools.back().data(), entry.numColumns, entry.numRows };
		}
	}

	g_overrideActive.store(true, std::memory_order_release);

	trace("Replacing common:/data/effects/ptxclipregions.dat with %s\n", path.c_str());
}

static void ResetOverride()
{
	// Stop serving before clearing so any in-flight GetData() falls back to the game.
	g_overrideActive.store(false, std::memory_order_release);

	std::unique_lock<std::shared_mutex> lock(g_storeMutex);
	g_store.clear();
}

namespace streaming
{
	// GTA_FIVE only: Set the path of a ptxclipregions.dat file to load instead of the default one.
	// Last caller wins; the override lasts until disconnect.
	void DLL_EXPORT SetPtxClipRegionsOverridePath(const std::string& path)
	{
		if (path.empty())
		{
			return;
		}

		if (!g_hookInstalled.load(std::memory_order_acquire))
		{
			trace("replace_ptxclipregions_file ignored: GetData hook unavailable on this game build\n");
			return;
		}

		ApplyOverride(path);
	}
}

static HookFunction hookFunction([]()
{

	auto pat = hook::pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B F2 8B D1 48 8D 0D ? ? ? ? 49 8B D9 49 8B F8");
	if (pat.size() != 1)
	{
		trace("replace_ptxclipregions_file: ptxClipRegions::GetData not found; cannot replace ptxclipregions on this game build\n");
		return;
	}

	g_origGetClipRegionData = hook::trampoline(pat.get(0).get<void>(), GetClipRegionData);
	g_hookInstalled.store(true, std::memory_order_release);

	OnKillNetworkDone.Connect([]()
	{
		ResetOverride();
	});
});
