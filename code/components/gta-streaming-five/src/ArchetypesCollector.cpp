/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <Pool.h>
#include <EntitySystem.h>
#include "Streaming.h"
#include <Error.h>
#include <GameInit.h>

#include <KnownFolders.h>
#include <ShlObj.h>

#include <algorithm>
#include <array>

#include <fstream>

#include <MinHook.h>

#include "ArchetypesCollector.h"

struct MapTypesFile {
	std::string filename;
	std::vector<std::string> archetypeNames;
	std::vector<float> archetypeLodDists;
};

static std::unordered_map<uint32_t, MapTypesFile> g_mapTypesFiles;

static void*(*g_origLoadMapTypes)(void*, uint32_t, CMapTypes** mapTypes);

static void* CMapTypes_Load(void* a1, uint32_t a2, CMapTypes** data)
{
	CMapTypes* mapTypes = *data;

	if (g_mapTypesFiles.find(mapTypes->name) != g_mapTypesFiles.end())
	{
		return g_origLoadMapTypes(a1, a2, data);
	}

	//trace("Loading CMapTypes from %s\n", streaming::GetStreamingBaseNameForHash(mapTypes->name));
	MapTypesFile file;

	file.filename = streaming::GetStreamingBaseNameForHash(mapTypes->name);

	for (fwArchetypeDef* archetype : mapTypes->archetypes)
	{
		if (archetype->assetType == 3) // ignore ASSET_TYPE_DRAWABLEDICTIONARY 
		{
			continue;
		}

		auto archetypeName = streaming::GetStreamingBaseNameForHash(archetype->name);

		if (!archetypeName.empty())
		{
			file.archetypeNames.push_back(archetypeName);
			file.archetypeLodDists.push_back(archetype->lodDist);
		}
	}

	g_mapTypesFiles.insert({ mapTypes->name, file });

	return g_origLoadMapTypes(a1, a2, data);
}

static void WriteArchetypesFile()
{
	PWSTR appDataPath;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appDataPath);

	std::string archetypesFilePath = ToNarrow(appDataPath) + "\\vmp\\archetypes.json";

	CoTaskMemFree(appDataPath);

	std::ofstream archetypesFileStream(archetypesFilePath);

	// header
	archetypesFileStream << "{\"____v\":1";

	// first write file names
	{
		archetypesFileStream << ",\"files\":[";

		bool first = true;

		for (auto& [_filenameHash, mapfile] : g_mapTypesFiles)
		{
			if (mapfile.archetypeNames.size() == 0)
			{
				continue;
			}

			if (first) first = false;
			else archetypesFileStream << ',';

			archetypesFileStream << '"' << mapfile.filename << '"';
		}

		archetypesFileStream << ']';
	}

	// now archetypes
	{
		archetypesFileStream << ",\"archetypes\":{";

		bool first = true;
		auto fileIdx = 0;

		for (auto& [_filenameHash, mapfile] : g_mapTypesFiles)
		{
			if (mapfile.archetypeNames.size() == 0)
			{
				continue;
			}

			auto i = 0;

			for (auto& archetypeName : mapfile.archetypeNames)
			{
				if (first) first = false;
				else archetypesFileStream << ',';

				archetypesFileStream << '"' << archetypeName << "\":" << '[' << fileIdx << ',' << mapfile.archetypeLodDists[i] << ']';

				i++;
			}

			fileIdx++;
		}

		archetypesFileStream << '}';
	}

	archetypesFileStream << '}';
}

static void LoadAllMapTypesFiles()
{
	//trace("Start loading all mapdatas, there're currently %d archetypes loaded\n", g_archetypesNames.size());

	auto mgr = streaming::Manager::GetInstance();
	auto mapTypesStore = mgr->moduleMgr.GetStreamingModule("ytyp");
	auto pool = (atPoolBase*)((char*)mapTypesStore + 56);

	static constexpr uint8_t batchSize = 4;

	for (int count = 0; count < pool->GetCount(); count += batchSize)
	{
		int end = std::min((count + batchSize), (int)pool->GetCount());

		std::vector<uint32_t> requestedObjects;

		for (int i = count; i < end; i++)
		{
			if (pool->GetAt<void*>(i) == NULL)
			{
				continue;
			}

			auto gidx = mapTypesStore->baseIdx + i;

			if ((mgr->Entries[gidx].flags & 3) == 0)
			{
				mgr->RequestObject(gidx, 0);
				requestedObjects.push_back(gidx);
			}
		}

		if (requestedObjects.size() == 0)
		{
			continue;
		}

		streaming::LoadObjectsNow(0);

		for (auto gidx : requestedObjects)
		{
			mgr->ReleaseObject(gidx);
		}
	}

	//trace("Done loading all mapdatas, there're currently %d archetypes loaded\n", g_archetypesNames.size());

	WriteArchetypesFile();
}

static HookFunction hookFunction([]()
{
#ifdef GTA_FIVE
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("F6 46 10 04 75 10 B8 00 02 00 00", -0x59), CMapTypes_Load, (void**)&g_origLoadMapTypes);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	OnKillNetworkDone.Connect([]()
	{
		g_mapTypesFiles.clear();
	});

	OnRefreshArchetypesCollection.Connect([]()
	{
		LoadAllMapTypesFiles();
		OnRefreshArchetypesCollectionDone();
	});
#endif
});
