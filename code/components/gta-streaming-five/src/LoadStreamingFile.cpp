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

#include <IteratorView.h>
#include <ICoreGameInit.h>

#include <MinHook.h>

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

static hook::cdecl_stub<void(void*)> _initManifestChunk([]()
{
	return hook::get_pattern("48 8D 4F 10 B2 01 48 89 2F", -0x2E);
});

static hook::cdecl_stub<void(void*)> _loadManifestChunk([]()
{
	return hook::get_call(hook::get_pattern("45 38 AE C0 00 00 00 0F 95 C3 E8", -5));
});

static hook::cdecl_stub<void(void*)> _clearManifestChunk([]()
{
	return hook::get_pattern("33 FF 48 8D 4B 10 B2 01", -0x15);
});

static void* manifestChunkPtr;

bool CfxPackfileMounter::MountFile(DataFileEntry* entry)
{
	entry->disabled = true;
	//entry->persistent = true;
	//entry->locked = true;
	//entry->overlay = true;
	_initManifestChunk(manifestChunkPtr);
	_addPackfile(entry);
	_loadManifestChunk(manifestChunkPtr);
	_clearManifestChunk(manifestChunkPtr);
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

static hook::cdecl_stub<bool(void* streaming, int idx)> _isResourceNotCached([]()
{
	return hook::get_pattern("74 07 8A 40 48 24 01 EB 02 B0 01", -0x1B);
});

static bool g_reloadMapStore = false;

static std::set<std::string> loadedCollisions;

int GetDummyCollectionIndexByTag(const std::string& tag);
extern std::unordered_map<int, std::string> g_handlesToTag;

fwEvent<> OnReloadMapStore;

static void ReloadMapStore()
{
	if (!g_reloadMapStore)
	{
		return;
	}

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
					return;
				}

				auto mgr = streaming::Manager::GetInstance();
				auto relId = obj - streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ybn")->baseIdx;

				if (_isResourceNotCached(mgr, obj) || GetDummyCollectionIndexByTag(g_handlesToTag[mgr->Entries[obj].handle]) == -1)
				{
					mgr->RequestObject(obj, 0);

					streaming::LoadObjectsNow(0);

					mgr->ReleaseObject(obj);

					loadedCollisions.insert(file);

					trace("Loaded %s (id %d)\n", file, relId);
				}
				else
				{
					trace("Skipped %s - it's cached! (id %d)\n", file, relId);
				}
			}
		}
	});

	OnReloadMapStore();

	// workaround by unloading/reloading MP map group
	g_disableContentGroup(*g_extraContentManager, 0xBCC89179); // GROUP_MAP

	// again for enablement
	OnReloadMapStore();

	g_enableContentGroup(*g_extraContentManager, 0xBCC89179);

	g_clearContentCache(0);

	loadedCollisions.clear();

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

	g_reloadMapStore = false;
}

class CfxPseudoMounter : public CDataFileMountInterface
{
public:
	virtual bool MountFile(DataFileEntry* entry) override
	{
		if (strcmp(entry->name, "RELOAD_MAP_STORE") == 0)
		{
			g_reloadMapStore = true;

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

void LoadCache(const char* tagName);
void LoadManifest(const char* tagName);

class CfxCacheMounter : public CDataFileMountInterface
{
public:
	virtual bool MountFile(DataFileEntry* entry) override
	{
		LoadManifest(entry->name);
		LoadCache(entry->name);

		return true;
	}

	virtual bool UnmountFile(DataFileEntry* entry) override
	{
		return true;
	}
};

static CfxCacheMounter g_staticCacheMounter;

static CDataFileMountInterface** g_dataFileMounters;

static CDataFileMountInterface* LookupDataFileMounter(const std::string& type)
{
	if (type == "CFX_PSEUDO_ENTRY")
	{
		return &g_staticPseudoMounter;
	}

	if (type == "CFX_PSEUDO_CACHE")
	{
		return &g_staticCacheMounter;
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

	// don't allow TEXTFILE_METAFILE entries (these don't work and will fail to unload)
	if (fileType == 160) // TEXTFILE_METAFILE 
	{
		return nullptr;
	}

	return g_dataFileMounters[fileType];
}

static void LoadDataFiles();

static void HandleDataFile(const std::pair<std::string, std::string>& dataFile, const std::function<bool(CDataFileMountInterface*, DataFileEntry& entry)>& fn, const char* op)
{
	std::string typeName;
	std::string fileName;

	std::tie(typeName, fileName) = dataFile;

	trace("%s %s %s.\n", op, typeName, fileName);

	CDataFileMountInterface* mounter = LookupDataFileMounter(typeName);

	if (mounter == nullptr)
	{
		trace("Could not add data_file %s - invalid type %s.\n", fileName, typeName);
		return;
	}

	if (mounter)
	{
		std::string className = typeid(*mounter).name();

		DataFileEntry entry;
		memset(&entry, 0, sizeof(entry));
		strcpy(entry.name, fileName.c_str()); // muahaha
		entry.type = LookupDataFileType(typeName);

		bool result = SafeCall([&]()
		{
			return fn(mounter, entry);
		}, va("%s of %s in data file mounter %s", op, fileName, className));

		if (result)
		{
			trace("done %s %s in data file mounter %s.\n", op, fileName, className);
		}
		else
		{
			trace("failed %s %s in data file mounter %s.\n", op, fileName, className);
		}
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

void LoadStreamingFiles(bool earlyLoad = false);

static LONG FilterUnmountOperation(DataFileEntry& entry)
{
	if (entry.type == 174) // DLC_ITYP_REQUEST
	{
		trace("failed to unload DLC_ITYP_REQUEST %s\n", entry.name);

		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_SEARCH;
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
			LoadStreamingFiles();
			LoadDataFiles();
		}
	}

	void DLL_EXPORT RemoveDataFileFromLoadList(const std::string& type, const std::string& path)
	{
		auto dataFilePair = std::make_pair(type, path);

		if (std::find(g_loadedDataFiles.begin(), g_loadedDataFiles.end(), dataFilePair) == g_loadedDataFiles.end())
		{
			return;
		}
		
		std::remove(g_loadedDataFiles.begin(), g_loadedDataFiles.end(), dataFilePair);

		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			auto singlePair = { dataFilePair };

			HandleDataFileList(singlePair, [](CDataFileMountInterface* mounter, DataFileEntry& entry)
			{
				__try
				{
					return mounter->UnmountFile(&entry);
				}
				__except (FilterUnmountOperation(entry))
				{

				}
			}, "removing");
		}
	}
}

static hook::cdecl_stub<rage::fiCollection*()> getRawStreamer([]()
{
	return hook::get_call(hook::get_pattern("48 8B D3 4C 8B 00 48 8B C8 41 FF 90 ? 01 00 00", -5));
});

#include <unordered_set>

static std::set<std::tuple<std::string, std::string>> g_customStreamingFiles;
std::set<std::string> g_customStreamingFileRefs;
static std::map<std::string, std::vector<std::string>, std::less<>> g_customStreamingFilesByTag;
static std::unordered_map<int, std::list<uint32_t>> g_handleStack;
static std::set<std::pair<streaming::strStreamingModule*, int>> g_pendingRemovals;
std::unordered_map<int, std::string> g_handlesToTag;

static std::unordered_set<int> g_ourIndexes;

static std::string GetBaseName(const std::string& name)
{
	std::string retval = name;

	std::string policyVal;

	if (Instance<ICoreGameInit>::Get()->GetData("policy", &policyVal))
	{
#ifndef _DEBUG
		if (policyVal.find("[subdir_file_mapping]") != std::string::npos)
#endif
		{
			std::replace(retval.begin(), retval.end(), '^', '/');
		}
	}

	return retval;
}

static void LoadStreamingFiles(bool earlyLoad)
{
	// register any custom streaming assets
	for (auto it = g_customStreamingFiles.begin(); it != g_customStreamingFiles.end(); )
	{
		auto[file, tag] = *it;

		// get basename ('thing.ytd') and asset name ('thing')
		const char* slashPos = strrchr(file.c_str(), '/');

		if (slashPos == nullptr)
		{
			it = g_customStreamingFiles.erase(it);
			continue;
		}

		auto baseName = GetBaseName(std::string(slashPos + 1));
		auto nameWithoutExt = baseName.substr(0, baseName.find_last_of('.'));

		const char* extPos = strrchr(baseName.c_str(), '.');

		if (extPos == nullptr)
		{
			trace("can't register %s: it doesn't have an extension, why is this in stream/?\n", file);
			it = g_customStreamingFiles.erase(it);
			continue;
		}

		// get CStreaming instance and associated streaming module
		std::string ext = extPos + 1;

		if (ext == "rpf")
		{
			trace("can't register %s: it's an RPF, these don't belong in stream/ without extracting them first\n", file);
			it = g_customStreamingFiles.erase(it);
			continue;
		}

		if (earlyLoad)
		{
			if (ext == "ymap" || ext == "ytyp" || ext == "ybn")
			{
				++it;
				continue;
			}
		}

		it = g_customStreamingFiles.erase(it);

		auto cstreaming = streaming::Manager::GetInstance();
		auto strModule = cstreaming->moduleMgr.GetStreamingModule(ext.c_str());

		if (strModule)
		{
			// try to create/get an asset in the streaming module
			// RegisterStreamingFile will still work if one exists as long as the handle remains 0
			uint32_t strId;
			strModule->FindSlotFromHashKey(&strId, nameWithoutExt.c_str());

			g_ourIndexes.insert(strId + strModule->baseIdx);
			g_pendingRemovals.erase({ strModule, strId });

			// if the asset is already registered...
			if (cstreaming->Entries[strId + strModule->baseIdx].handle != 0)
			{
				// get the raw streamer and make an entry in there
				auto rawStreamer = getRawStreamer();
				uint32_t idx = rawStreamer->GetEntryByName(file.c_str());

				if (strId != -1)
				{
					auto& entry = cstreaming->Entries[strId + strModule->baseIdx];

					trace("overriding handle for %s (was %x) -> %x\n", baseName, entry.handle, (rawStreamer->GetCollectionId() << 16) | idx);

					// if no old handle was saved, save the old handle
					auto& hs = g_handleStack[strId + strModule->baseIdx];

					if (hs.empty())
					{
						hs.push_front(entry.handle);
					}

					entry.handle = (rawStreamer->GetCollectionId() << 16) | idx;
					g_handlesToTag[entry.handle] = tag;

					// save the new handle
					hs.push_front(entry.handle);
				}
			}
			else
			{
				uint32_t fileId;
				streaming::RegisterRawStreamingFile(&fileId, file.c_str(), true, baseName.c_str(), false);

				if (fileId != -1)
				{
					auto& entry = cstreaming->Entries[fileId];
					g_handleStack[fileId].push_front(entry.handle);

					g_handlesToTag[entry.handle] = tag;
				}
				else
				{
					trace("failed to reg %s? %d\n", baseName, fileId);
				}
			}
		}
		else
		{
			trace("can't register %s: no streaming module (does this file even belong in stream?)\n", file);
		}
	}
}

static std::multimap<std::string, std::string, std::less<>> g_manifestNames;

#include <fiCustomDevice.h>

class ForcedDevice : public rage::fiCustomDevice
{
private:
	rage::fiDevice* m_device;
	std::string m_fileName;

public:
	ForcedDevice(rage::fiDevice* parent, const std::string& fileName)
		: m_device(parent), m_fileName(fileName)
	{
	}

	virtual uint64_t Open(const char* fileName, bool readOnly) override
	{
		return m_device->Open(m_fileName.c_str(), readOnly);
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) override
	{
		return m_device->OpenBulk(m_fileName.c_str(), ptr);
	}

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*) override
	{
		return OpenBulk(fileName, ptr);
	}

	virtual uint64_t Create(const char* fileName) override
	{
		return -1;
	}

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) override
	{
		return m_device->Read(handle, buffer, toRead);
	}

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) override
	{
		return m_device->ReadBulk(handle, ptr, buffer, toRead);
	}

	virtual int m_40(int a) override
	{
		return m_device->m_40(a);
	}

	virtual rage::fiDevice* GetUnkDevice() override
	{
		return m_device->GetUnkDevice();
	}

	virtual void m_xx() override
	{
		return m_device->m_xx();
	}

	virtual int32_t GetCollectionId() override
	{
		return m_device->GetCollectionId();
	}

	virtual bool m_ax() override
	{
		return m_device->m_ax();
	}

	virtual uint32_t Write(uint64_t, void*, int) override
	{
		return -1;
	}

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int) override
	{
		return -1;
	}

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) override
	{
		return m_device->Seek(handle, distance, method);
	}

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) override
	{
		return m_device->SeekLong(handle, distance, method);
	}

	virtual int32_t Close(uint64_t handle) override
	{
		return m_device->Close(handle);
	}

	virtual int32_t CloseBulk(uint64_t handle) override
	{
		return m_device->CloseBulk(handle);
	}

	virtual int GetFileLength(uint64_t handle) override
	{
		return m_device->GetFileLength(handle);
	}

	virtual uint64_t GetFileLengthLong(const char* fileName) override
	{
		return m_device->GetFileLengthLong(m_fileName.c_str());
	}

	virtual uint64_t GetFileLengthUInt64(uint64_t handle) override
	{
		return m_device->GetFileLengthUInt64(handle);
	}

	virtual bool RemoveFile(const char* file) override
	{
		return false;
	}

	virtual int RenameFile(const char* from, const char* to) override
	{
		return false;
	}

	virtual int CreateDirectory(const char* dir) override
	{
		return false;
	}

	virtual int RemoveDirectory(const char* dir) override
	{
		return false;
	}

	virtual uint64_t GetFileTime(const char* file) override
	{
		return m_device->GetFileTime(m_fileName.c_str());
	}

	virtual bool SetFileTime(const char* file, FILETIME fileTime) override
	{
		return false;
	}

	virtual uint32_t GetFileAttributes(const char* path) override
	{
		return m_device->GetFileAttributes(m_fileName.c_str());
	}

	virtual int m_yx() override
	{
		return m_device->m_yx();
	}

	virtual bool IsCollection() override
	{
		return m_device->IsCollection();
	}

	virtual const char* GetName() override
	{
		return "RageVFSDeviceAdapter";
	}

	virtual int GetResourceVersion(const char* fileName, rage::ResourceFlags* version) override
	{
		return m_device->GetResourceVersion(m_fileName.c_str(), version);
	}

	virtual uint64_t CreateLocal(const char* fileName) override
	{
		return m_device->CreateLocal(m_fileName.c_str());
	}

	virtual void* m_xy(void* a, int len, void* c) override
	{
		return m_device->m_xy(a, len, (void*)m_fileName.c_str());
	}
};

static hook::cdecl_stub<void(void*, void* packfile, const char*)> loadManifest([]()
{
	return hook::get_pattern("49 8B F0 4C 8B F1 48 85 D2 0F 84", -0x23);
});

void LoadManifest(const char* tagName)
{
	auto range = g_manifestNames.equal_range(tagName);

	for (auto& namePair : fx::GetIteratorView(range))
	{
		auto name = namePair.second;

		_initManifestChunk(manifestChunkPtr);

		auto rel = new ForcedDevice(rage::fiDevice::GetDevice(name.c_str(), true), name);
		rage::fiDevice::MountGlobal("localPack:/", rel, true);

		loadManifest(manifestChunkPtr, (void*)1, tagName);

		rage::fiDevice::Unmount("localPack:/");

		_loadManifestChunk(manifestChunkPtr);
		_clearManifestChunk(manifestChunkPtr);
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

	if (g_reloadMapStore)
	{
		trace("Performing deferred RELOAD_MAP_STORE.\n");

		ReloadMapStore();
	}
}

DLL_EXPORT void ForceMountDataFile(const std::pair<std::string, std::string>& dataFile)
{
	std::vector<std::pair<std::string, std::string>> dataFiles = { dataFile };

	HandleDataFileList(dataFiles, [](CDataFileMountInterface* mounter, DataFileEntry& entry)
	{
		return mounter->MountFile(&entry);
	});
}

void ForAllStreamingFiles(const std::function<void(const std::string&)>& cb)
{
	for (auto& entry : g_customStreamingFileRefs)
	{
		cb(entry);
	}
}

#include <nutsnbolts.h>

static bool g_reloadStreamingFiles;
static std::atomic<int> g_lockedStreamingFiles;

void origCfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);

void DLL_EXPORT CfxCollection_SetStreamingLoadLocked(bool locked)
{
	if (locked)
	{
		g_lockedStreamingFiles++;
	}
	else
	{
		g_lockedStreamingFiles--;
	}
}

void DLL_EXPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags)
{
	auto baseName = std::string(strrchr(fileName.c_str(), '/') + 1);

	if (baseName.find(".ymf") == baseName.length() - 4)
	{
		g_manifestNames.emplace(tag, fileName);
	}

	g_customStreamingFilesByTag[tag].push_back(fileName);
	g_customStreamingFiles.insert({ fileName, tag });
	g_customStreamingFileRefs.insert(baseName);

	g_reloadStreamingFiles = true;

	origCfxCollection_AddStreamingFileByTag(tag, fileName, flags);
}

void DLL_EXPORT CfxCollection_RemoveStreamingTag(const std::string& tag)
{
	// ensure that we can call into game code here
	// #FIXME: should we not always be on the main thread?!
	rage::sysMemAllocator::UpdateAllocatorValue();

	for (auto& file : g_customStreamingFilesByTag[tag])
	{
		// get basename ('thing.ytd') and asset name ('thing')
		auto baseName = GetBaseName(std::string(strrchr(file.c_str(), '/') + 1));
		auto nameWithoutExt = baseName.substr(0, baseName.find_last_of('.'));

		// get dot position and skip if no dot
		auto dotPos = strrchr(baseName.c_str(), '.');

		if (!dotPos)
		{
			continue;
		}

		// get CStreaming instance and associated streaming module
		auto cstreaming = streaming::Manager::GetInstance();
		auto strModule = cstreaming->moduleMgr.GetStreamingModule(dotPos + 1);

		if (strModule)
		{
			uint32_t strId;
			strModule->FindSlotFromHashKey(&strId, nameWithoutExt.c_str());

			auto rawStreamer = getRawStreamer();
			uint32_t idx = (rawStreamer->GetCollectionId() << 16) | rawStreamer->GetEntryByName(file.c_str());

			if (strId != -1)
			{
				// remove from our index set
				g_ourIndexes.erase(strId + strModule->baseIdx);

				// erase existing stack entry
				auto& handleData = g_handleStack[strId + strModule->baseIdx];

				for (auto it = handleData.begin(); it != handleData.end(); ++it)
				{
					if (*it == idx)
					{
						it = handleData.erase(it);
					}
				}

				// if not empty, set the handle to the current stack entry
				auto& entry = cstreaming->Entries[strId + strModule->baseIdx];

				if (!handleData.empty())
				{
					entry.handle = handleData.front();
				}
				else
				{
					// release the object if it was likely to have been faked
					if (streaming::IsStreamerShuttingDown())
					{
						streaming::Manager::GetInstance()->ReleaseObject(strId + strModule->baseIdx);
					}

					g_pendingRemovals.insert({ strModule, strId });

					g_customStreamingFileRefs.erase(baseName);
					entry.handle = 0;
				}
			}
		}
	}

	for (auto& file : g_customStreamingFilesByTag[tag])
	{
		g_customStreamingFiles.erase(std::tuple<std::string, std::string>{ file, tag });
	}

	g_customStreamingFilesByTag.erase(tag);
	g_manifestNames.erase(tag);
}

static void UnloadDataFiles()
{
	if (!g_loadedDataFiles.empty())
	{
		trace("Unloading data files (%d entries)\n", g_loadedDataFiles.size());

		HandleDataFileList(g_loadedDataFiles, [] (CDataFileMountInterface* mounter, DataFileEntry& entry)
		{
			return mounter->UnmountFile(&entry);
		}, "unloading");

		g_loadedDataFiles.clear();
	}
}

static hook::cdecl_stub<void()> _unloadMultiplayerContent([]()
{
	return hook::get_pattern("01 E8 ? ? ? ? 48 8B 0D ? ? ? ? BA 79", -0x11);
});

static const char* NormalizePath(char* out, const char* in, size_t length)
{
	strncpy(out, in, length);

	int l = strlen(out);

	for (int i = 0; i < l; i++)
	{
		if (out[i] == '\\')
		{
			out[i] = '/';
		}
	}

	return out;
}

struct pgRawStreamer
{
	struct Entry
	{
		char m_pad[24];
		const char* fileName;
	};

	char m_pad[1456];
	Entry* m_entries[64];
};

static const char* pgRawStreamer__GetEntryNameToBuffer(pgRawStreamer* streamer, uint16_t index, char* buffer, int len)
{
	const char* fileName = streamer->m_entries[index >> 10][index & 0x3FF].fileName;

	if (fileName == nullptr)
	{
		buffer[0] = '\0';
		return buffer;
	}

	strncpy(buffer, fileName, len - 1);
	buffer[len - 1] = '\0';

	return buffer;
}

static void DisplayRawStreamerError [[noreturn]] (pgRawStreamer* streamer, uint16_t index)
{
	auto streamingMgr = streaming::Manager::GetInstance();

	uint32_t attemptIndex = (((rage::fiCollection*)streamer)->GetCollectionId() << 16) | index;
	std::string extraData;

	for (size_t i = 0; i < streamingMgr->numEntries; i++)
	{
		const auto& entry = streamingMgr->Entries[i];

		if (entry.handle == attemptIndex)
		{
			std::string tag = g_handlesToTag[entry.handle];

			extraData += fmt::sprintf("Streaming tag: %s\n", tag);
			extraData += fmt::sprintf("File name: %s\n", streaming::GetStreamingNameForIndex(i));
			extraData += fmt::sprintf("Handle stack size: %d\n", g_handleStack[i].size());
			extraData += fmt::sprintf("Tag exists: %s\n", g_customStreamingFilesByTag.find(tag) != g_customStreamingFilesByTag.end() ? "yes" : "no");
		}
	}

	FatalError("Invalid pgRawStreamer call - fileName == NULL.\nStreaming index: %d\n%s", index, extraData);
}

static int64_t(*g_origOpenCollectionEntry)(pgRawStreamer* streamer, uint16_t index, uint64_t* ptr);

static int64_t pgRawStreamer__OpenCollectionEntry(pgRawStreamer* streamer, uint16_t index, uint64_t* ptr)
{
	const char* fileName = streamer->m_entries[index >> 10][index & 0x3FF].fileName;

	if (fileName == nullptr)
	{
		DisplayRawStreamerError(streamer, index);
	}

	return g_origOpenCollectionEntry(streamer, index, ptr);
}

static int64_t(*g_origGetEntry)(pgRawStreamer* streamer, uint16_t index);

static int64_t pgRawStreamer__GetEntry(pgRawStreamer* streamer, uint16_t index)
{
	const char* fileName = streamer->m_entries[index >> 10][index & 0x3FF].fileName;

	if (fileName == nullptr)
	{
		DisplayRawStreamerError(streamer, index);
	}

	return g_origGetEntry(streamer, index);
}

static bool g_unloadingCfx;

namespace streaming
{
	bool IsStreamerShuttingDown()
	{
		return g_unloadingCfx;
	}
}

static void* g_streamingInternals;

static hook::cdecl_stub<void()> _waitUntilStreamerClear([]()
{
	return hook::get_call(hook::get_pattern("80 A1 7A 01 00 00 FE 8B EA", 12));
});

static hook::cdecl_stub<void(void*)> _resyncStreamers([]()
{
	return hook::get_call(hook::get_pattern("80 A1 7A 01 00 00 FE 8B EA", 24));
});

static hook::cdecl_stub<void()> _unloadTextureLODs([]()
{
	// there's two of these, both seem to do approximately the same thing, but the first one we want
	return hook::get_pattern("48 85 DB 75 1B 8D 47 01 49 8D", -0x84);
});

static void SafelyDrainStreamer()
{
	g_unloadingCfx = true;

	trace("Shutdown: waiting for streaming to finish\n");

	_waitUntilStreamerClear();

	trace("Shutdown: updating GTA streamer state\n");

	_resyncStreamers(g_streamingInternals);

	trace("Shutdown: unloading texture LODs\n");

	_unloadTextureLODs();

	trace("Shutdown: streamer tasks done\n");
}

static void(*g_origAddMapBoolEntry)(void* map, int* index, bool* value);

void WrapAddMapBoolEntry(void* map, int* index, bool* value)
{
	// don't allow this for any files of our own
	if (g_ourIndexes.find(*index) == g_ourIndexes.end())
	{
		g_origAddMapBoolEntry(map, index, value);
	}
}

static void(*g_origExecuteGroup)(void* mgr, uint32_t hashValue, bool value);

static void ExecuteGroupForWeaponInfo(void* mgr, uint32_t hashValue, bool value)
{
	g_origExecuteGroup(mgr, hashValue, value);

	for (auto it = g_loadedDataFiles.begin(); it != g_loadedDataFiles.end();)
	{
		auto[fileType, fileName] = *it;

		if (fileType == "WEAPONINFO_FILE_PATCH" || fileType == "WEAPONINFO_FILE")
		{
			HandleDataFile(*it, [](CDataFileMountInterface* mounter, DataFileEntry& entry)
			{
				return mounter->UnmountFile(&entry);
			}, "early-unloading for CWeaponMgr");

			it = g_loadedDataFiles.erase(it);
		}
		else
		{
			it++;
		}
	}
}

static void(*g_origUnloadWeaponInfos)();

static hook::cdecl_stub<void(void*)> wib_ctor([]()
{
	// 1604
	return (void*)0x140E78710;
});

struct CWeaponInfoBlob
{
	char m_pad[248];

	CWeaponInfoBlob()
	{
		wib_ctor(this);
	}
};

static atArray<CWeaponInfoBlob>* g_weaponInfoArray;

static void UnloadWeaponInfosStub()
{
	g_origUnloadWeaponInfos();

	g_weaponInfoArray->Clear();
	g_weaponInfoArray->Expand(0x80);
}

static hook::cdecl_stub<void(int32_t)> rage__fwArchetypeManager__FreeArchetypes([]()
{
	return hook::get_pattern("8B F9 8B DE 66 41 3B F0 73 33", -0x19);
});

static void(*g_origUnloadMapTypes)(void*, uint32_t);

void fwMapTypesStore__Unload(char* assetStore, uint32_t index)
{
	auto pool = (atPoolBase*)(assetStore + 56);
	auto entry = pool->GetAt<char>(index);

	if (entry != nullptr)
	{
		if (*(uintptr_t*)entry != 0)
		{
			if (g_unloadingCfx)
			{
				*(uint16_t*)(entry + 16) &= ~0x14;
			}

			g_origUnloadMapTypes(assetStore, index);
		}
		else
		{
			AddCrashometry("maptypesstore_workaround_2", "true");
		}
	}
	else
	{
		AddCrashometry("maptypesstore_workaround", "true");
	}
}

#include <GameInit.h>

std::set<std::string> g_streamingSuffixSet;

static void ModifyHierarchyStatusHook(streaming::strStreamingModule* module, int idx, int* status)
{
	if (*status == 1 && g_ourIndexes.find(module->baseIdx + idx) != g_ourIndexes.end())
	{
		auto thisName = streaming::GetStreamingNameForIndex(module->baseIdx + idx);

		// if this is, say, vb_02.ymap, and we also loaded hei_vb_02.ymap, skip this file
		if (g_streamingSuffixSet.find(thisName) == g_streamingSuffixSet.end())
		{
			*status = 2;
		}
	}
}

static bool(*g_orig_fwStaticBoundsStore__ModifyHierarchyStatus)(streaming::strStreamingModule* module, int idx, int status);

static bool fwStaticBoundsStore__ModifyHierarchyStatus(streaming::strStreamingModule* module, int idx, int status)
{
	// don't disable hierarchy overlay for any custom overrides
	ModifyHierarchyStatusHook(module, idx, &status);

	return g_orig_fwStaticBoundsStore__ModifyHierarchyStatus(module, idx, status);
}

static bool(*g_orig_fwMapDataStore__ModifyHierarchyStatusRecursive)(streaming::strStreamingModule* module, int idx, int status);

static bool fwMapDataStore__ModifyHierarchyStatusRecursive(streaming::strStreamingModule* module, int idx, int status)
{
	// don't disable hierarchy overlay for any custom overrides
	ModifyHierarchyStatusHook(module, idx, &status);

	return g_orig_fwMapDataStore__ModifyHierarchyStatusRecursive(module, idx, status);
}

static bool g_lockReload;

static HookFunction hookFunction([] ()
{
	// process streamer-loaded resource: check 'free instantly' flag even if no dependencies exist (change jump target)
	*hook::get_pattern<int8_t>("4C 63 C0 85 C0 7E 54 48 8B", 6) = 0x25;

	// same function: stub to change free-instantly flag if needed by bypass streaming
	static struct : jitasm::Frontend
	{
		static bool ShouldRequestBeAllowed()
		{
			if (streaming::IsStreamerShuttingDown())
			{
				return false;
			}

			return true;
		}

		void InternalMain() override
		{
			sub(rsp, 0x28);

			// restore rcx as the call stub used this
			mov(rcx, r14);

			// call the original function that's meant to be called
			mov(rax, qword_ptr[rax + 0xA8]);
			call(rax);

			// save the result in a register (r12 is used as output by this function)
			mov(r12, rax);

			// store the streaming request in a1
			mov(rcx, rsi);
			mov(rax, (uintptr_t)&ShouldRequestBeAllowed);
			call(rax);

			// perform a switcharoo of rax and r12
			// (r12 is the result the game wants, rax is what we want in r12)
			xchg(r12, rax);

			add(rsp, 0x28);
			ret();
		}
	} streamingBypassStub;

	{
		auto location = hook::get_pattern("45 8A E7 FF 90 A8 00 00 00");
		hook::nop(location, 9);
		hook::call_rcx(location, streamingBypassStub.GetCode());
	}

	g_streamingInternals = hook::get_address<void*>(hook::get_pattern("80 A1 7A 01 00 00 FE 8B EA", 20));

	manifestChunkPtr = hook::get_address<void*>(hook::get_pattern("83 F9 08 75 43 48 8D 0D", 8));

	// level load
	void* hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B0 01 48 8B D3").count(1).get(0).get<void>(18);
	hook::set_call(&dataFileMgr__loadDat, hookPoint);
	hook::call(hookPoint, LoadDats);

	//hookPoint = hook::pattern("E8 ? ? ? ? 33 C9 E8 ? ? ? ? 41 8B CE E8 ? ? ? ?").count(1).get(0).get<void>(0); //Jayceon - If I understood right, is this what we were supposed to do? It seems wrong to me
	hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 1D ? ? ? ? 41 8B F7").count(1).get(0).get<void>(0);
	hook::set_call(&dataFileMgr__loadDefDat, hookPoint);
	hook::call(hookPoint, LoadDefDats); //Call the new function to load the handling files

	// don't normalize paths in pgRawStreamer
	hook::call(hook::get_pattern("48 8B D6 E8 ? ? ? ? B2 01 48", 3), NormalizePath);

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
		// safely drain the RAGE streamer before we unload everything
		SafelyDrainStreamer();

		g_unloadingCfx = false;
	}, 99900);

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		// safely drain the RAGE streamer before we unload everything
		SafelyDrainStreamer();

		g_lockReload = true;
		g_unloadingCfx = true;

		UnloadDataFiles();

		std::set<std::string> tags;

		for (auto& tag : g_customStreamingFilesByTag)
		{
			tags.insert(tag.first);
		}

		for (auto& tag : tags)
		{
			CfxCollection_RemoveStreamingTag(tag);
		}

		auto typesStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytyp");

		for (auto [ module, idx ] : g_pendingRemovals)
		{
			if (module == typesStore)
			{
				atPoolBase* entryPool = (atPoolBase*)((char*)module + 56);
				auto entry = entryPool->GetAt<char>(idx);

				*(uint16_t*)(entry + 16) &= ~0x14;
			}

			streaming::Manager::GetInstance()->ReleaseObject(idx + module->baseIdx);

			if (module == typesStore)
			{
				// if unloaded at *runtime* but flags were set, archetypes likely weren't freed - we should
				// free them now.
				rage__fwArchetypeManager__FreeArchetypes(idx);
			}

			module->RemoveSlot(idx);
		}

		g_pendingRemovals.clear();

		g_unloadingCfx = false;
	}, -9999);

	OnMainGameFrame.Connect([=]()
	{
		if (g_reloadStreamingFiles && g_lockedStreamingFiles == 0 && !g_lockReload)
		{
			LoadStreamingFiles();

			g_reloadStreamingFiles = false;
		}
	});

	{
		char* location = hook::get_pattern<char>("48 63 82 90 00 00 00 49 8B 8C C0 ? ? ? ? 48", 11);

		g_dataFileMounters = (decltype(g_dataFileMounters))(hook::get_adjusted(0x140000000) + *(int32_t*)location); // why is this an RVA?!
	}

	{
		char* location = hook::get_pattern<char>("79 91 C8 BC E8 ? ? ? ? 48 8D", -0x30);

		location += 0x1A;
		g_extraContentManager = (void**)(*(int32_t*)location + location + 4);
		location -= 0x1A;

		hook::set_call(&g_disableContentGroup, location + 0x23);
		hook::set_call(&g_enableContentGroup, location + 0x34);
		hook::set_call(&g_clearContentCache, location + 0x50);
	}

	rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			g_lockReload = false;

			LoadStreamingFiles(true);
		}
	});

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			LoadStreamingFiles();
			LoadDataFiles();
		}
	});

	// special point for CWeaponMgr streaming unload
	// (game calls CExtraContentManager::ExecuteTitleUpdateDataPatchGroup with a specific group intended for weapon info here)
	{
		auto location = hook::get_pattern<char>("45 33 C0 BA E9 C8 73 AA E8", 8);
		hook::set_call(&g_origExecuteGroup, location);
		hook::call(location, ExecuteGroupForWeaponInfo);

		g_weaponInfoArray = hook::get_address<decltype(g_weaponInfoArray)>(location + 0x74);
	}

	// don't create an unarmed weapon when *unloading* a WEAPONINFO_FILE in the mounter (this will get badly freed later
	// which will lead to InitSession failing)
	{
		hook::return_function(hook::get_pattern("7C 94 48 85 F6 74 0D 48 8B 06 BA 01 00 00 00", 0x3C));
	}

	// fully clean weaponinfoblob array when resetting weapon manager
	// not doing so will lead to parser crashes when a half-reset value is reused
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("45 33 C0 BA E9 C8 73 AA E8", -0x11), UnloadWeaponInfosStub, (void**)&g_origUnloadWeaponInfos);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// fwMapTypesStore double unloading workaround
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("4C 63 C2 33 ED 46 0F B6 0C 00 8B 41 4C", -18), fwMapTypesStore__Unload, (void**)&g_origUnloadMapTypes);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// support CfxRequest for pgRawStreamer
	hook::jump(hook::get_pattern("4D 63 C1 41 8B C2 41 81 E2 FF 03 00 00", -0xD), pgRawStreamer__GetEntryNameToBuffer);

	// do not ever register our streaming files as part of DLC packfile dependencies
	{
		auto location = hook::get_pattern("48 8B CE C6 85 ? 00 00 00 01 89 44 24 20 E8", 14);
		hook::set_call(&g_origAddMapBoolEntry, location);
		hook::call(location, WrapAddMapBoolEntry);
	}

	// debug hook for pgRawStreamer::OpenCollectionEntry
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("8B D5 81 E2", -0x24), pgRawStreamer__OpenCollectionEntry, (void**)&g_origOpenCollectionEntry);
	MH_CreateHook(hook::get_pattern("0F B7 C3 48 8B 5C 24 30 8B D0 25 FF", -0x14), pgRawStreamer__GetEntry, (void**)&g_origGetEntry);
	MH_CreateHook(hook::get_pattern("45 8B E8 4C 8B F1 83 FA FF 0F 84", -0x18), fwStaticBoundsStore__ModifyHierarchyStatus, (void**)&g_orig_fwStaticBoundsStore__ModifyHierarchyStatus);
	MH_CreateHook(hook::get_pattern("45 33 D2 84 C0 0F 84 ? 01 00 00 4C", -0x28), fwMapDataStore__ModifyHierarchyStatusRecursive, (void**)&g_orig_fwMapDataStore__ModifyHierarchyStatusRecursive);
	MH_EnableHook(MH_ALL_HOOKS);
});
