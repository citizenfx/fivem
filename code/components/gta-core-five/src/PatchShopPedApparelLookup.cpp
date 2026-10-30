#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.FlexStruct.h>
#include <Hooking.Stubs.h>

#include <CoreConsole.h>
#include <CrossBuildRuntime.h>
#include <ICoreGameInit.h>

#include <cassert>
#include <cstdint>
#include <iterator>
#include <EASTL/hash_map.h>

// Shop ped apparel items are resolved by name through one engine map whose backing
// array counts entries in a u16; once the total <Item> count crosses ~65,520 it
// overflows and corrupts the heap on join. This diverts the rebuild and both lookup
// helpers to an unbounded side table (eastl::hash_map), leaving the engine map empty.

static bool g_extendedApparelLimitEnabled = false;
static bool g_builtApparelIndex = false;

static int32_t g_mgrFilesPtrOffset;
static int32_t g_mgrFilesCountOffset;
static int32_t g_mgrMapSortedOffset;
static int32_t g_mgrMapCountOffset;

static constexpr int32_t kItemNameHashOffset = 0x20;

// The three item sub-arrays in a ShopPedApparel file (component/prop/outfit).
struct ItemArrayLayout
{
	int32_t ptrOffset;
	int32_t countOffset;
	size_t  stride;
	uint8_t kind;
};

static constexpr ItemArrayLayout kItemArrays[3] = {
	{ 0x28, 0x30, 144, 0 },
	{ 0x38, 0x40, 136, 1 },
	{ 0x48, 0x50, 160, 2 },
};

// Packed to match what the engine getters decode: kind[0:8) file[8:24) item[32:48).
static inline uint64_t EncodeLocation(uint8_t kind, uint16_t fileIndex, uint16_t itemIndex)
{
	return static_cast<uint64_t>(kind)
	     | (static_cast<uint64_t>(fileIndex) << 8)
	     | (static_cast<uint64_t>(itemIndex) << 32);
}

static eastl::hash_map<uint32_t, uint64_t> g_apparelIndex;

static void (*g_origRebuild)(hook::FlexStruct* manager);

static void ApparelIndex_Rebuild(hook::FlexStruct* manager)
{
	g_builtApparelIndex = true;
	g_apparelIndex.clear();

	if (!g_extendedApparelLimitEnabled)
	{
		g_origRebuild(manager);
		return;
	}

	uint16_t fileCount = manager->Get<uint16_t>(g_mgrFilesCountOffset);
	auto files = manager->Get<hook::FlexStruct**>(g_mgrFilesPtrOffset);

	for (uint16_t fileIndex = 0; fileIndex < fileCount; fileIndex++)
	{
		hook::FlexStruct* file = files[fileIndex];
		if (!file)
		{
			continue;
		}

		for (const auto& arr : kItemArrays)
		{
			uint16_t count = file->Get<uint16_t>(arr.countOffset);
			auto items = file->Get<uint8_t*>(arr.ptrOffset);
			if (!items)
			{
				continue;
			}

			for (uint16_t itemIndex = 0; itemIndex < count; itemIndex++)
			{
				auto item = reinterpret_cast<hook::FlexStruct*>(items + static_cast<size_t>(itemIndex) * arr.stride);
				uint32_t hash = item->Get<uint32_t>(kItemNameHashOffset);
				// keep the first occurrence, matching the engine's lower-bound on ties
				g_apparelIndex.emplace(hash, EncodeLocation(arr.kind, fileIndex, itemIndex));
			}
		}
	}

	// leave the engine map empty + sorted so nothing grows or reads it
	manager->Set<uint16_t>(g_mgrMapCountOffset, 0);
	manager->Set<uint8_t>(g_mgrMapSortedOffset, 1);
}

// The engine has two compiled copies of the lookup helper; divert both.
static char* (*g_origLookupA)(hook::FlexStruct* map, uint32_t* nameHash);
static char* (*g_origLookupB)(hook::FlexStruct* map, uint32_t* nameHash);

static char* ResolveFromIndex(hook::FlexStruct* map, uint32_t nameHash)
{
	auto it = g_apparelIndex.find(nameHash);
	if (it == g_apparelIndex.end())
	{
		return nullptr;
	}

	uint64_t packed = it->second;
	uint8_t kind = static_cast<uint8_t>(packed & 0xFF);
	uint16_t fileIdx = static_cast<uint16_t>((packed >> 8) & 0xFFFF);
	uint16_t itemIdx = static_cast<uint16_t>((packed >> 32) & 0xFFFF);

	// re-validate against the live manager so a stale entry can't point at an unloaded file
	auto manager = &map->At<hook::FlexStruct>(-g_mgrMapSortedOffset);
	if (fileIdx >= manager->Get<uint16_t>(g_mgrFilesCountOffset))
	{
		return nullptr;
	}

	hook::FlexStruct* file = manager->Get<hook::FlexStruct**>(g_mgrFilesPtrOffset)[fileIdx];
	if (!file)
	{
		return nullptr;
	}

	// kind is always 0-2; the modulo keeps the index in range (silences C6385)
	const ItemArrayLayout& arr = kItemArrays[kind % std::size(kItemArrays)];
	if (itemIdx >= file->Get<uint16_t>(arr.countOffset))
	{
		return nullptr;
	}

	return reinterpret_cast<char*>(&it->second);
}

static char* ApparelIndex_LookupA(hook::FlexStruct* map, uint32_t* nameHash)
{
	return g_extendedApparelLimitEnabled ? ResolveFromIndex(map, *nameHash) : g_origLookupA(map, nameHash);
}

static char* ApparelIndex_LookupB(hook::FlexStruct* map, uint32_t* nameHash)
{
	return g_extendedApparelLimitEnabled ? ResolveFromIndex(map, *nameHash) : g_origLookupB(map, nameHash);
}

static void OnExtendedApparelLimitChange(internal::ConsoleVariableEntry<bool>* var)
{
	// Lock the value in once the index is built; changing it mid-session - or when
	// the convar resets on disconnect - would desync the lookups from the index.
	if (g_builtApparelIndex)
	{
		return;
	}

	g_extendedApparelLimitEnabled = var->GetRawValue();
}

static HookFunction hookFunction([]()
{
	static ConVar<bool> extendedApparelLimitVar("game_enableExtendedApparelShopLimit", ConVar_Replicated, false, &OnExtendedApparelLimitChange);

	if (!xbr::IsGameBuildOrGreater<3258>())
	{
		return;
	}

	// Session over (and its apparel content gone) - unpin so the next one's value applies.
	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_builtApparelIndex = false;
		g_extendedApparelLimitEnabled = extendedApparelLimitVar.GetValue();
	});

	// Read the manager offsets from the displacements RebuildApparelLookup uses.
	g_mgrMapSortedOffset = *hook::get_pattern<uint32_t>("33 D2 C6 81 ? ? ? ? 01 48 8B F9", 4);
	g_mgrMapCountOffset = *hook::get_pattern<uint32_t>("48 8B F9 66 89 91 ? ? ? ? 8B F2", 6);
	g_mgrFilesCountOffset = *hook::get_pattern<uint8_t>("0F B7 47 ? FF C6 89 75 ? 3B F0", 3);
	g_mgrFilesPtrOffset = *hook::get_pattern<uint8_t>("48 8B 47 ? 8B CE 44 8B E2 4C 8B 34 C8", 3);

#ifdef _DEBUG
	// Catch a struct-layout change in a future game build.
	assert(*hook::get_pattern<uint8_t>("48 8D 0C C0 49 8B 46 ? 45 0B EF", 7) == kItemArrays[0].ptrOffset);
	assert(*hook::get_pattern<uint8_t>("4C 8B 34 C8 66 41 3B 56 ? 73", 8) == kItemArrays[0].countOffset);
	assert(*hook::get_pattern<uint8_t>("44 89 6D D8 8B 5C C8 ? 8B 4D DC", 7) == kItemNameHashOffset);
#endif

	g_origRebuild = hook::trampoline(hook::get_pattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC ? 33 D2"), ApparelIndex_Rebuild);
	g_origLookupA = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 57 48 83 EC ? 45 33 C9 48 8B FA 48 8B D9 66 44 39 49 ? 76 ? 8B 02 48 8B 49 ? 4C 89 4C 24 ? 89 44 24 ? 0F B7 43 ? 4C 8D 0D ? ? ? ? 4C 8D 04 40 4A 8D 14 81"), ApparelIndex_LookupA);
	g_origLookupB = hook::trampoline(hook::get_pattern("4C 8B DC 49 89 5B ? 57 48 83 EC ? 45 33 C0 48 8B FA 48 8B D9 66 44 39 41 ? 76 ? ? ? 48 8B 49 ? 4D 89 43 ? 89 44 24"), ApparelIndex_LookupB);
});
