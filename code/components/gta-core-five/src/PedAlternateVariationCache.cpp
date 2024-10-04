/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <Hooking.h>

#include <MinHook.h>
#include <nutsnbolts.h>
#include <GameInit.h>
#include <atArray.h>

//
// These patches add caching logic for ped alternate variations (ALTERNATE_VARIATIONS_FILE).
// The first two hooked functions impacted performance a lot during ped creation.
// The original logic was iterating through all the loaded variations (multiple times) to find switches.
// During ped creation this was repeated at least 12 times (for each ped component).
// It seems this code wasn't intended for such big amount of clothes and alternate variations.
// We're fixing it by caching results beforehand, when the game loads variation data files.
//

struct AlternateVariationsSwitchAsset // 0x1BC82D17
{
	uint8_t component;
	uint8_t anchor;
	uint8_t index;
	char pad[1];
	uint32_t dlcNameHash;
};

struct AlternateVariationsSwitch // 0x58E12301
{
	struct
	{
		uint8_t component;	// hand_ ...
		uint8_t index;		// ... 000_ ...
		uint8_t alt;		// ... 23
		char pad[1];
		uint32_t dlcNameHash;
	} data;

	atArray<AlternateVariationsSwitchAsset> sourceAssets;
};

struct AlternateVariationsPed // 0xEF79CBDB
{
	uint32_t name; // model name hash, e.g. mp_m_freemode_01 and etc
	char pad[4];
	atArray<AlternateVariationsSwitch> switches; // +8
};

struct CAlternateVariations
{
	atArray<AlternateVariationsPed> peds;
};

using TAlternateVariationsSwitchSet = std::set<AlternateVariationsSwitch*>;
using TAlternateVariationsCache = std::map<std::tuple<uint32_t, uint32_t, char, char>, TAlternateVariationsSwitchSet>;

// See pedalternatevariations.meta
static CAlternateVariations* g_alternateVariations;

static bool g_hasGeneratedCache = false;

static TAlternateVariationsCache g_cachedAlternatesByIndex;
static TAlternateVariationsCache g_cachedAlternatesByAnchor;

static TAlternateVariationsSwitchSet* GetAlternateVariationsCacheEntry(TAlternateVariationsCache* cacheMap, uint32_t modelHash, uint32_t dlcNameHash, uint8_t component, uint8_t itemIndex)
{
	if (auto it = cacheMap->find({ modelHash, dlcNameHash, component, itemIndex }); it != cacheMap->end())
	{
		return &it->second;
	}

	return nullptr;
}

static void AddAlternateVariationsCacheEntry(TAlternateVariationsCache* cacheMap, AlternateVariationsSwitch* variationsSwitch, uint32_t modelHash, uint32_t dlcNameHash, uint8_t component, uint8_t itemIndex)
{
	auto switchesSet = GetAlternateVariationsCacheEntry(cacheMap, modelHash, dlcNameHash, component, itemIndex);

	if (!switchesSet)
	{
		switchesSet = &cacheMap->insert({ { modelHash, dlcNameHash, component, itemIndex }, {} }).first->second;
	}

	switchesSet->insert(variationsSwitch);
}

static void LoadAlternateVariationSwitches(TAlternateVariationsSwitchSet* switchSet, atArray<AlternateVariationsSwitchAsset>* outArray)
{
	uint16_t index = 0;

	for (auto cacheEntry : *switchSet)
	{
		// rough but will work
		auto switchAsset = *(AlternateVariationsSwitchAsset*)&cacheEntry->data;
		outArray->Set(index++, std::move(switchAsset));
	}

	outArray->m_count = index;
}

static void ClearAlternateVariationsCache()
{
	g_cachedAlternatesByIndex.clear();
	g_cachedAlternatesByAnchor.clear();
	g_hasGeneratedCache = false;
}

static void GenerateAlternateVariationsCache()
{
	ClearAlternateVariationsCache();

	for (auto& pedEntry : g_alternateVariations->peds)
	{
		for (auto& switchEntry : pedEntry.switches)
		{
			for (auto& assetEntry : switchEntry.sourceAssets)
			{
				AddAlternateVariationsCacheEntry(&g_cachedAlternatesByIndex, &switchEntry, pedEntry.name, assetEntry.dlcNameHash, assetEntry.component, assetEntry.index);
				AddAlternateVariationsCacheEntry(&g_cachedAlternatesByAnchor, &switchEntry, pedEntry.name, assetEntry.dlcNameHash, assetEntry.component, assetEntry.anchor);
			}
		}
	}

	g_hasGeneratedCache = true;
}

static bool (*g_origGetAlternateVariationSwitchesByIndex)(void*, char, char, uint32_t, void*);

static bool GetAlternateVariationSwitchesByIndex(AlternateVariationsPed* pedEntry, char component, char index, uint32_t dlcNameHash, atArray<AlternateVariationsSwitchAsset>* outArray)
{
	if (!g_hasGeneratedCache)
	{
		return g_origGetAlternateVariationSwitchesByIndex(pedEntry, component, index, dlcNameHash, outArray);
	}

	outArray->m_count = 0;

	if (auto cachedSwitches = GetAlternateVariationsCacheEntry(&g_cachedAlternatesByIndex, pedEntry->name, dlcNameHash, component, index))
	{
		LoadAlternateVariationSwitches(cachedSwitches, outArray);
	}

	return outArray->m_count != 0;
}

static bool (*g_origGetAlternateVariationSwitchesByAnchor)(void*, char, char, uint32_t, void*);

static bool GetAlternateVariationSwitchesByAnchor(AlternateVariationsPed* pedEntry, char component, char anchor, uint32_t dlcNameHash, atArray<AlternateVariationsSwitchAsset>* outArray)
{
	if (!g_hasGeneratedCache)
	{
		return g_origGetAlternateVariationSwitchesByAnchor(pedEntry, component, anchor, dlcNameHash, outArray);
	}

	outArray->m_count = 0;

	if (auto cachedSwitches = GetAlternateVariationsCacheEntry(&g_cachedAlternatesByAnchor, pedEntry->name, dlcNameHash, component, anchor))
	{
		LoadAlternateVariationSwitches(cachedSwitches, outArray);
	}

	return outArray->m_count != 0;
}

static void (*g_origLoadAlternateVariationsFile)(void*);

static void LoadAlternateVariationsFile(void* data)
{
	g_origLoadAlternateVariationsFile(data);

	if (g_hasGeneratedCache)
	{
		GenerateAlternateVariationsCache();
	}
}

static void (*g_origUnloadAlternateVariationsFile)(void*);

static void UnloadAlternateVariationsFile(void* data)
{
	g_origUnloadAlternateVariationsFile(data);

	if (g_hasGeneratedCache)
	{
		GenerateAlternateVariationsCache();
	}
}

static HookFunction initFunction([]()
{
	g_alternateVariations = hook::get_address<decltype(g_alternateVariations)>(hook::get_pattern("85 C0 7E 28 4C 8B 0D", 7));

	MH_Initialize();
	MH_CreateHook(hook::get_call(hook::get_pattern("48 8D 45 38 40 8A D6 48 89 44 24", 12)), GetAlternateVariationSwitchesByIndex, (void**)&g_origGetAlternateVariationSwitchesByIndex);
	MH_CreateHook(hook::get_call(hook::get_pattern("45 8A C5 40 8A D6 48 8B CB 48 89 44", 14)), GetAlternateVariationSwitchesByAnchor, (void**)&g_origGetAlternateVariationSwitchesByAnchor);
	MH_CreateHook(hook::get_call(hook::get_pattern("83 E9 09 74 34 81 E9 A1 00 00 00", 50)), LoadAlternateVariationsFile, (void**)&g_origLoadAlternateVariationsFile);
	MH_CreateHook(hook::get_call(hook::get_pattern("81 BA ? ? ? ? AA 00 00 00 75 08", 15)), UnloadAlternateVariationsFile, (void**)&g_origUnloadAlternateVariationsFile);
	MH_EnableHook(MH_ALL_HOOKS);

	Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([]()
	{
		GenerateAlternateVariationsCache();
	});

	OnKillNetworkDone.Connect([]()
	{
		ClearAlternateVariationsCache();
	});
});
