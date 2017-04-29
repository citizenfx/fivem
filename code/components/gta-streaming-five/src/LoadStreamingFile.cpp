/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <Pool.h>

#include <algorithm>
#include <array>

#include <gameSkeleton.h>

#include <boost/algorithm/string.hpp>

#include <Error.h>

#include <ICoreGameInit.h>

static void(*dataFileMgr__loadDat)(void*, const char*, bool);
static void(*dataFileMgr__loadDefDat)(void*, const char*, bool);

static std::vector<std::string> g_beforeLevelMetas;
static std::vector<std::string> g_afterLevelMetas;

static std::vector<std::string> g_defaultMetas;

static std::vector<std::string> g_gtxdFiles;
static std::vector<std::pair<std::string, std::string>> g_dataFiles;
static std::vector<std::pair<std::string, std::string>> g_loadedDataFiles;

struct DataFileEntry
{
	char name[128];
	char pad[16]; // 128
	int32_t type; // 140
	int32_t index; // 148
	bool locked; // 152
	bool flag2; // 153
	bool flag3; // 154
	bool disabled; // 155
	bool persistent; // 156
	bool overlay;
	char pad2[10];
};

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

static void* g_dataFileMgr;

static void LoadDefDats(void* dataFileMgr, const char* name, bool enabled)
{
	//dataFileMgr__loadDefDat(dataFileMgr, "citizen:/citizen.meta", enabled);

	g_dataFileMgr = dataFileMgr;

	// load before-level metas
	trace("LoadDefDats: %s\n", name);

	// load the level
	dataFileMgr__loadDefDat(dataFileMgr, name, enabled);
}

static std::vector<std::string> g_oldEntryList;

static int SehRoutine(const char* whatPtr, PEXCEPTION_POINTERS exception)
{
	if (exception->ExceptionRecord->ExceptionCode & 0x80000000)
	{
		if (!whatPtr)
		{
			whatPtr = "a safe-call operation";
		}

		FatalErrorNoExcept("An exception occurred (%08x at %p) during %s. The game will be terminated.",
			exception->ExceptionRecord->ExceptionCode, exception->ExceptionRecord->ExceptionAddress,
			whatPtr);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

#include <sysAllocator.h>
#include <atArray.h>

template<typename T>
static auto SafeCall(const T& fn, const char* whatPtr = nullptr)
{
#ifndef _DEBUG
	__try
	{
#endif
		return fn();
#ifndef _DEBUG
	}
	__except (SehRoutine(whatPtr, GetExceptionInformation()))
	{
		return std::result_of_t<T()>();
	}
#endif
}

struct EnumEntry
{
	uint32_t hash;
	uint32_t index;
};

static EnumEntry* g_dataFileTypes;

static int LookupDataFileType(const std::string& type)
{
	uint32_t thisHash = HashRageString(boost::to_upper_copy(type).c_str());

	for (size_t i = 0; i < 0xC9; i++)
	{
		auto entry = &g_dataFileTypes[i];

		if (entry->hash == thisHash)
		{
			return entry->index;
		}
	}

	return -1;
}

class CDataFileMountInterface
{
public:
	virtual ~CDataFileMountInterface() = default;

	virtual bool MountFile(DataFileEntry* entry) = 0;

	virtual bool UnmountFile(DataFileEntry* entry) = 0;
};

class CfxPackfileMounter : public CDataFileMountInterface
{
public:
	virtual bool MountFile(DataFileEntry* entry) override;

	virtual bool UnmountFile(DataFileEntry* entry) override;
};

static hook::cdecl_stub<void(DataFileEntry* entry)> _addPackfile([]()
{
	return hook::get_call(hook::get_pattern("EB 15 48 8B 0B 40 38 7B 0C 74 07 E8", 11));
});

static hook::cdecl_stub<void(DataFileEntry* entry)> _removePackfile([]()
{
	return hook::get_call(hook::get_pattern("EB 15 48 8B 0B 40 38 7B 0C 74 07 E8", 18));
});

bool CfxPackfileMounter::MountFile(DataFileEntry* entry)
{
	entry->disabled = true;
	//entry->persistent = true;
	//entry->locked = true;
	//entry->overlay = true;
	_addPackfile(entry);
	return true;
}

bool CfxPackfileMounter::UnmountFile(DataFileEntry* entry)
{
	_removePackfile(entry);
	return true;
}

static void** g_extraContentManager;
static void(*g_disableContentGroup)(void*, uint32_t);
static void(*g_enableContentGroup)(void*, uint32_t);
static void(*g_clearContentCache)(int);

static CfxPackfileMounter g_staticRpfMounter;

#include <Streaming.h>

void ForAllStreamingFiles(const std::function<void(const std::string&)>& cb);

static CDataFileMountInterface* LookupDataFileMounter(const std::string& type);

class CfxPseudoMounter : public CDataFileMountInterface
{
private:
	std::set<std::string> loadedCollisions;

public:
	virtual bool MountFile(DataFileEntry* entry) override
	{
		if (strcmp(entry->name, "RELOAD_MAP_STORE") == 0)
		{
			// preload collisions for the world
			ForAllStreamingFiles([&](const std::string& file)
			{
				if (file.find(".ybn") != std::string::npos)
				{
					if (loadedCollisions.find(file) == loadedCollisions.end())
					{
						auto obj = streaming::GetStreamingIndexForName(file);

						if (obj == 0)
						{
							trace("waaaa?\n");
							return;
						}

						auto mgr = streaming::Manager::GetInstance();
						mgr->RequestObject(obj, 0);

						streaming::LoadObjectsNow(0);

						mgr->ReleaseObject(obj);

						loadedCollisions.insert(file);

						trace("Loaded %s (id %d)\n", file, obj);
					}
				}
			});

			// workaround by unloading/reloading MP map group
			g_disableContentGroup(*g_extraContentManager, 0xBCC89179); // GROUP_MAP
			g_enableContentGroup(*g_extraContentManager, 0xBCC89179);

			g_clearContentCache(0);

			// load gtxd files
			for (auto& file : g_gtxdFiles)
			{
				auto mounter = LookupDataFileMounter("GTXD_PARENTING_DATA");

				DataFileEntry ventry;
				memset(&ventry, 0, sizeof(ventry));
				strcpy(ventry.name, file.c_str()); // muahaha
				ventry.type = LookupDataFileType("GTXD_PARENTING_DATA");

				mounter->MountFile(&ventry);

				trace("Mounted gtxd parenting data %s\n", file);
			}

			return true;
		}

		return false;
	}

	virtual bool UnmountFile(DataFileEntry* entry) override
	{
		if (strcmp(entry->name, "RELOAD_MAP_STORE") == 0)
		{
			// empty?
			loadedCollisions.clear();
		}


		return true;
	}
};

static CfxPseudoMounter g_staticPseudoMounter;

static CDataFileMountInterface** g_dataFileMounters;

static CDataFileMountInterface* LookupDataFileMounter(const std::string& type)
{
	if (type == "CFX_PSEUDO_ENTRY")
	{
		return &g_staticPseudoMounter;
	}

	int fileType = LookupDataFileType(type);

	if (fileType < 0)
	{
		return nullptr;
	}

	if (fileType == 0)
	{
		return &g_staticRpfMounter;
	}

	return g_dataFileMounters[fileType];
}

static void LoadDataFiles();

static void HandleDataFile(const std::pair<std::string, std::string>& dataFile, const std::function<bool(CDataFileMountInterface*, DataFileEntry& entry)>& fn, const char* op)
{
	std::string typeName;
	std::string fileName;

	std::tie(typeName, fileName) = dataFile;

	CDataFileMountInterface* mounter = LookupDataFileMounter(typeName);

	if (mounter == nullptr)
	{
		trace("Could not add data_file %s - invalid type %s.\n", fileName, typeName);
		return;
	}

	if (mounter)
	{
		std::string typeName = typeid(*mounter).name();

		DataFileEntry entry;
		memset(&entry, 0, sizeof(entry));
		strcpy(entry.name, fileName.c_str()); // muahaha
		entry.type = LookupDataFileType(typeName);

		bool result = SafeCall([&]()
		{
			return fn(mounter, entry);
		}, va("%s of %s in data file mounter %s", op, fileName, typeName));

		if (result)
		{
			trace("done %s %s in data file mounter %s.\n", op, fileName, typeName);
		}
		else
		{
			trace("failed %s %s in data file mounter %s.\n", op, fileName, typeName);
		}
	}
	else
	{
		trace("%s %s failed (no mounter for type %s)\n", op, fileName, typeName);
	}
}

template<typename TFn, typename TList>
inline void HandleDataFileList(const TList& list, const TFn& fn, const char* op = "loading")
{
	for (const auto& dataFile : list)
	{
		HandleDataFile(dataFile, fn, op);
	}
}

namespace streaming
{
	void DLL_EXPORT AddMetaToLoadList(bool before, const std::string& meta)
	{
		((before) ? g_beforeLevelMetas : g_afterLevelMetas).push_back(meta);
	}

	void DLL_EXPORT AddDefMetaToLoadList(const std::string& meta)
	{
		g_defaultMetas.push_back(meta);
	}

	void DLL_EXPORT AddDataFileToLoadList(const std::string& type, const std::string& path)
	{
		if (type == "GTXD_PARENTING_DATA")
		{
			g_gtxdFiles.push_back(path);
			return;
		}

		g_dataFiles.push_back({ type, path });

		if (Instance<ICoreGameInit>::Get()->GetGameLoaded() && !Instance<ICoreGameInit>::Get()->HasVariable("gameKilled"))
		{
			LoadDataFiles();
		}
	}

	void DLL_EXPORT RemoveDataFileFromLoadList(const std::string& type, const std::string& path)
	{
		auto dataFilePair = std::make_pair(type, path);
		std::remove(g_loadedDataFiles.begin(), g_loadedDataFiles.end(), dataFilePair);

		auto singlePair = { dataFilePair };

		HandleDataFileList(singlePair, [] (CDataFileMountInterface* mounter, DataFileEntry& entry)
		{
			return mounter->UnmountFile(&entry);
		}, "removing");
	}
}

static void LoadDataFiles()
{
	trace("Loading mounted data files (total: %d)\n", g_dataFiles.size());

	HandleDataFileList(g_dataFiles, [] (CDataFileMountInterface* mounter, DataFileEntry& entry)
	{
		return mounter->MountFile(&entry);
	});
	
	g_loadedDataFiles.insert(g_loadedDataFiles.end(), g_dataFiles.begin(), g_dataFiles.end());
	g_dataFiles.clear();
}

static void UnloadDataFiles()
{
	if (!g_loadedDataFiles.empty())
	{
		trace("Unloading data files (%d entries)\n", g_loadedDataFiles.size());

		HandleDataFileList(g_dataFiles, [] (CDataFileMountInterface* mounter, DataFileEntry& entry)
		{
			return mounter->UnmountFile(&entry);
		}, "unloading");

		g_loadedDataFiles.clear();
	}
}

static hook::cdecl_stub<void()> _unloadMultiplayerContent([]()
{
	return hook::get_pattern("BA 79 91 C8 BC C6 05 ? ? ? ? 01 E8", -0xB);
});

#include <GameInit.h>

static HookFunction hookFunction([] ()
{
	// level load
	void* hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B0 01 48 8B D3").count(1).get(0).get<void>(18);
	hook::set_call(&dataFileMgr__loadDat, hookPoint);
	//hook::call(hookPoint, LoadLevelDatHook);

	//hookPoint = hook::pattern("E8 ? ? ? ? 33 C9 E8 ? ? ? ? 41 8B CE E8 ? ? ? ?").count(1).get(0).get<void>(0); //Jayceon - If I understood right, is this what we were supposed to do? It seems wrong to me
	hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 1D ? ? ? ? 41 8B F7").count(1).get(0).get<void>(0);
	hook::set_call(&dataFileMgr__loadDefDat, hookPoint);
	hook::call(hookPoint, LoadDefDats); //Call the new function to load the handling files

	g_dataFileTypes = hook::get_pattern<EnumEntry>("61 44 DF 04 00 00 00 00");

	rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_BEFORE_MAP_LOADED)
		{
			if (!g_dataFileMgr)
			{
				return;
			}

			trace("Loading default meta overrides (total: %d)\n", g_defaultMetas.size());

			for (const auto& dat : g_defaultMetas)
			{
				trace("Loading default meta %s\n", dat);

				dataFileMgr__loadDat(g_dataFileMgr, dat.c_str(), true);
			}

			trace("Done loading default meta overrides!\n");
		}
	});

	// unload GROUP_MAP before reloading, for it'll break fwMapTypesStore if there's a map CCS loaded
	// by the time DLC reinitializes
	OnKillNetworkDone.Connect([]()
	{
		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			// unload GROUP_MAP and load GROUP_MAP_SP
			_unloadMultiplayerContent();
		}
	}, 99925);

	OnKillNetworkDone.Connect([]()
	{
		UnloadDataFiles();

		/*if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			// toggle map group again?
			g_enableContentGroup(*g_extraContentManager, 0xBCC89179); // GROUP_MAP
			g_disableContentGroup(*g_extraContentManager, 0xBCC89179);

			g_clearContentCache(0);
		}*/
	}, 99900);

	{
		char* location = hook::get_pattern<char>("48 63 82 90 00 00 00 49 8B 8C C0 ? ? ? ? 48", 11);

		g_dataFileMounters = (decltype(g_dataFileMounters))(0x140000000 + *(int32_t*)location); // why is this an RVA?!
	}

	{
		char* location = hook::get_pattern<char>("E2 99 8F 57 C6 05 ? ? ? ? 00 E8", -0xC);

		location += 7;
		g_extraContentManager = (void**)(*(int32_t*)location + location + 4);
		location -= 7;

		hook::set_call(&g_disableContentGroup, location + 0x17);
		hook::set_call(&g_enableContentGroup, location + 0x28);
		hook::set_call(&g_clearContentCache, location + 0x33);
	}

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			LoadDataFiles();
		}
	});
});