/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <algorithm>
#include <array>

static void(*dataFileMgr__loadDat)(void*, const char*, bool);

static std::vector<std::string> g_beforeLevelMetas;
static std::vector<std::string> g_afterLevelMetas;

struct DataFileEntry
{
	char name[128];
	char pad[20];
	int32_t length;
	bool flag1;
	bool flag2;
	bool flag3;
	bool disabled;
};

struct DataFileLess
{
	bool operator()(const std::string& left, const std::string& right)
	{
		return (_stricmp(left.c_str(), right.c_str()) < 0);
	}
};

using DataFileSet = std::set<std::string, DataFileLess>;

extern hook::cdecl_stub<DataFileEntry*(void*, DataFileEntry*)> dataFileMgr__getNextEntry;
extern DataFileEntry*(*dataFileMgr__getEntries)(void*, int);

static hook::cdecl_stub<void*(void*)> dataFileMgr__ctor([] ()
{
	return hook::pattern("83 C8 FF 48 8B F9 48 89 31 89 71 08").count(1).get(0).get<void>(-0x11);
});

static hook::cdecl_stub<void(DataFileEntry*)> dataFileEntry__enablePackfile([] ()
{
	return hook::pattern("80 B9 9B 00 00 00 00 48 8B F9 74 59").count(1).get(0).get<void>(-0xA);
});

static hook::cdecl_stub<void(DataFileEntry*)> dataFileEntry__disablePackfile([] ()
{
	return hook::pattern("80 B9 9B 00 00 00 00 48 8B F9 0F 85 A6").count(1).get(0).get<void>(-0xD);
});

static hook::cdecl_stub<void(char* buffer, int size, const char* inString, const char* inExt)> formatFilename([] ()
{
	return hook::pattern("83 64 24 28 00 4C 89 4C 24 20 4D 8B C8 44 8B C2").count(1).get(0).get<void>(-4);
});

static hook::cdecl_stub<uint32_t(const char* packfileName)> lookupStreamingPackfileByName([] ()
{
	return hook::get_call(hook::pattern("8D 4C 24 30 E8 ? ? ? ? 8B D8 83 F8 FF 74").count(1).get(0).get<void>(4));
});

void SetStreamingPackfileEnabled(uint32_t index, bool enabled);

static void* CreateDataFileMgr()
{
	void* data = malloc(264);

	return dataFileMgr__ctor(data);
}

template<typename Set>
void InsertInto(DataFileEntry* entry, Set& entries)
{
	entries.insert(entry);
}

template<typename TLess, typename TAllocator>
void InsertInto(DataFileEntry* entry, std::set<std::string, TLess, TAllocator>& entries)
{
	entries.insert(entry->name);
}

template<typename TField, typename TAllocator>
void InsertInto(DataFileEntry* entry, std::vector<TField, TAllocator>& entries)
{
	entries.push_back(entry);
}

template<typename TAllocator>
void InsertInto(DataFileEntry* entry, std::vector<std::string, TAllocator>& entries)
{
	entries.push_back(entry->name);
}

template<typename TargetSet>
static void GetEnabledEntryList(void* dataFileMgr, int type, TargetSet& entries)
{
	DataFileEntry* entry = dataFileMgr__getEntries(dataFileMgr, type);

	while (entry->length >= 0)
	{
		if (!entry->disabled)
		{
			InsertInto(entry, entries);
		}

		entry = dataFileMgr__getNextEntry(dataFileMgr, entry);
	}
}

template<typename TargetSet>
static TargetSet GetCurrentPackList(void* dataFileMgr)
{
	TargetSet entries;

	GetEnabledEntryList(dataFileMgr, 0, entries); // RPF_FILE
	GetEnabledEntryList(dataFileMgr, 0xB9, entries); // RPF_FILE_PRE_INSTALL
	GetEnabledEntryList(dataFileMgr, 0x9, entries); // another one

	return entries;
}

static void LoadDats(void* dataFileMgr, const char* name, bool enabled)
{
	dataFileMgr__loadDat(dataFileMgr, "citizen:/citizen.meta", enabled);

	// load before-level metas
	for (const auto& meta : g_beforeLevelMetas)
	{
		dataFileMgr__loadDat(dataFileMgr, meta.c_str(), enabled);
	}

	// load the level
	dataFileMgr__loadDat(dataFileMgr, name, enabled);

	// load after-level metas
	for (const auto& meta : g_afterLevelMetas)
	{
		dataFileMgr__loadDat(dataFileMgr, meta.c_str(), enabled);
	}
}

static std::vector<std::string> g_oldEntryList;

#include <sysAllocator.h>
#include <atArray.h>

struct fwBoxStreamerEntries
{
	atArray<uint32_t> indices[2];
	uint8_t indicesUsed;
};

static hook::cdecl_stub<void(void*)> freeObj1([] ()
{
	return hook::pattern("48 8B D9 48 8B 49 08 48 85 C9 74 2A 83 64").count(1).get(0).get<void>(-6);
});

struct fwBoxStreamer_obj1
{
	~fwBoxStreamer_obj1()
	{
		freeObj1(this);
	}

	void operator delete(void* ptr)
	{
		rage::GetAllocator()->free(ptr);
	}
};

struct fwBoxStreamer
{
public:
	virtual ~fwBoxStreamer() {}

	virtual void ResetQuadTree(void* streamingModule, float a3) = 0;

	atArray<int> _f8;
	void* streamingModule;
	atArray<int> _f20;
	atArray<int> _f30;
	alignas(uint64_t) uint32_t entryCount;
	uint32_t _f44;
	atArray<int> _f48;
	void* _f58;

	void* _f60;
	float _f68;

	alignas(uint64_t) char pad[80];

	fwBoxStreamerEntries entries;

	alignas(uint64_t) char pad2[16];

	fwBoxStreamer_obj1* _fF8;

	// fwBoxStreamerVariable members
	atArray<int> _f100;
	atArray<int> _f110;

	fwBoxStreamer_obj1* _f120;
};

fwBoxStreamer* g_mapDataBoxStreamer;

static hook::cdecl_stub<void(fwBoxStreamerEntries*)> clearEntries([] ()
{
	return hook::pattern("57 48 83 EC 20 48 8B F9 33 F6 B9 01 00").count(1).get(0).get<void>(-0xA);
});

static void SwitchPackfile(const std::string& packfileName, bool enabled)
{
	char buffer[256];
	formatFilename(buffer, sizeof(buffer), packfileName.c_str(), "");

	uint32_t idx = lookupStreamingPackfileByName(buffer);

	if (idx == 0xFFFFFFFF)
	{
		return;
	}

	SetStreamingPackfileEnabled(idx, enabled);
}

atArray<std::array<char, 40>>* g_boundDependencyArray;

static void LoadLevelDatHook(void* dataFileMgr, const char* name, bool enabled)
{
	// perform a dummy load first to see what this load will add to the list
	void* dummyMgr = CreateDataFileMgr();

	// load entries into the dummy manager
	LoadDats(dummyMgr, name, enabled);

	// function to fill an entry map
	std::unordered_map<std::string, DataFileEntry*> entryMap;

	auto fillEntryMap = [&] ()
	{
		// get the entries currently existing in the global manager
		std::set<DataFileEntry*> curEntries = GetCurrentPackList<std::set<DataFileEntry*>>(dataFileMgr);

		// map all entries for faster lookup
		entryMap.clear();

		for (auto& entry : curEntries)
		{
			entryMap.insert({ entry->name, entry });
		}
	};

	// get an entry set from the dummy manager
	std::vector<std::string> entries = GetCurrentPackList<std::vector<std::string>>(dummyMgr);
	std::sort(entries.begin(), entries.end());

	// if there's an old list, as well, do some differencing
	if (!g_oldEntryList.empty())
	{
		// the entries seem to need to be preallocated
		std::vector<std::string> removableEntries(max(g_oldEntryList.size(), entries.size()));

		// calculate difference
		auto it = std::set_difference(g_oldEntryList.begin(), g_oldEntryList.end(), entries.begin(), entries.end(), removableEntries.begin());

		// remove trailing entries
		removableEntries.resize(it - removableEntries.begin());

		// get the entries currently existing in the global manager
		fillEntryMap();

		// disable all the entries we found in here in the global manager
		for (auto& entry : removableEntries)
		{
			auto it = entryMap.find(entry);

			if (it != entryMap.end())
			{
				trace("disabling %s (previous state: %d)\n", entry.c_str(), it->second->disabled);

				dataFileEntry__disablePackfile(it->second);
				SwitchPackfile(it->first, false);
			}
			else
			{
				trace("force-disabling %s\n", entry.c_str());

				DataFileEntry tempEntry = { 0 };
				strcpy(tempEntry.name, entry.c_str());
				tempEntry.disabled = false;

				dataFileEntry__disablePackfile(&tempEntry);
				SwitchPackfile(entry, false);
			}
		}

		// and disable DLC entries as well because *why not*
		for (auto& entry : entryMap)
		{
			if (entry.first.find("dlc") == 0 && entry.first.find("levels/gta5") != std::string::npos)
			{
				trace("disabling %s (previous state: %d)\n", entry.first.c_str(), entry.second->disabled);

				dataFileEntry__disablePackfile(entry.second);
				SwitchPackfile(entry.first, false);
			}
		}
	}

	g_oldEntryList = entries;

	// load entries into the *global* data file manager
	LoadDats(dataFileMgr, name, enabled);

	// refill the entry map
	fillEntryMap();

	// and enable anything that might've been disabled
	for (auto& entry : entries)
	{
		auto it = entryMap.find(entry);

		if (it != entryMap.end())
		{
			trace("enabling %s (previous state: %d)\n", entry.c_str(), it->second->disabled);

			dataFileEntry__enablePackfile(it->second);
			SwitchPackfile(it->first, true);
		}
	}

	// free the dummy manager (NOTE: this won't actually free the entries contained - and there doesn't seem to be a dtor in the game?)
	free(dummyMgr);

	// clear the fwMapDataStore box streamer entry list
	clearEntries(&g_mapDataBoxStreamer->entries);

	delete g_mapDataBoxStreamer->_fF8;
	g_mapDataBoxStreamer->_fF8 = nullptr;

	g_mapDataBoxStreamer->entries.indices[0].Clear();
	g_mapDataBoxStreamer->entries.indices[1].Clear();

	if (g_mapDataBoxStreamer->_f8.GetSize())
	{
		g_mapDataBoxStreamer->_f8.Clear();
		// following two are the quadtree-esques
		g_mapDataBoxStreamer->_f20.Clear();
		g_mapDataBoxStreamer->_f30.Clear();
		g_mapDataBoxStreamer->_f48.Clear();
		// another quadtree
		g_mapDataBoxStreamer->_f100.Clear();
		g_mapDataBoxStreamer->_f110.Clear();

		delete g_mapDataBoxStreamer->_f120;
		g_mapDataBoxStreamer->_f120 = nullptr;

		// reset the quadtree list
		if (g_mapDataBoxStreamer->entryCount)
		{
			g_mapDataBoxStreamer->ResetQuadTree(g_mapDataBoxStreamer->streamingModule, g_mapDataBoxStreamer->_f68);
		}
	}

	// an array of static bound dependencies on map data sectors
	g_boundDependencyArray->Clear();
}

static void(*g_origAddStreamingPackfile)(const char*, bool);

void AddStreamingPackfileWrap(const char* fileName, bool scanNow)
{
	// if it's already registered but stupid us disabled it, re-enable
	SwitchPackfile(fileName, true);

	g_origAddStreamingPackfile(fileName, scanNow);
}

namespace streaming
{
	void DLL_EXPORT AddMetaToLoadList(bool before, const std::string& meta)
	{
		((before) ? g_beforeLevelMetas : g_afterLevelMetas).push_back(meta);
	}
}

static InitFunction initFunction([] ()
{
	assert(offsetof(fwBoxStreamer, entries) == 192);
	assert(offsetof(fwBoxStreamer, _f100) == 256);

	char test[40];
	static_assert(sizeof(std::array<char, 40>) == sizeof(test), "std::array size is not the same as the underlying array");
});

static HookFunction hookFunction([] ()
{
	// level load
	void* hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B0 01 48 8B D3").count(1).get(0).get<void>(18);

	hook::set_call(&dataFileMgr__loadDat, hookPoint);
	hook::call(hookPoint, LoadLevelDatHook);

	// box streamer
	{
		char* location = hook::pattern("48 8D 53 10 48 8D 0D ? ? ? ? 45 33 C0 E8").count(1).get(0).get<char>(7);

		g_mapDataBoxStreamer = (fwBoxStreamer*)(location + *(int32_t*)location + 4);
	}

	// bound dependency array
	{
		char* location = hook::pattern("48 8B 05 ? ? ? ? 48 8D 14 9B 48 8D 0C D0 48").count(1).get(0).get<char>(3);

		g_boundDependencyArray = (decltype(g_boundDependencyArray))(location + *(int32_t*)location + 4);
	}

	// re-enabling of DLC packfiles once loaded legitimately
	{
		char* location = hook::pattern("80 B9 9B 00 00 00 00 48 8B F9 74 59").count(1).get(0).get<char>(21);

		hook::set_call(&g_origAddStreamingPackfile, location);
		hook::call(location, AddStreamingPackfileWrap);
	}
});