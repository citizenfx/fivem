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

static hook::cdecl_stub<void(void*)> _initManifestChunk([]()
{
	return hook::get_pattern("48 8D 4F 10 B2 01 48 89 2F", -0x2E);
});

static hook::cdecl_stub<void(void*)> _loadManifestChunk([]()
{
	return hook::get_pattern("33 FF 4C 8B E9 BB FF FF 00 00", -0x2D);
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

int GetCollectionIndexByTag(const std::string& tag);
extern std::unordered_map<int, std::string> g_handlesToTag;

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

				if (_isResourceNotCached(mgr, obj) || GetCollectionIndexByTag(g_handlesToTag[mgr->Entries[obj].handle]) == -1)
				{
					mgr->RequestObject(obj, 0);

					streaming::LoadObjectsNow(0);

					mgr->ReleaseObject(obj);

					loadedCollisions.insert(file);

					trace("Loaded %s (id %d)\n", file, obj);
				}
				else
				{
					trace("Skipped %s - it's cached! (id %d)\n", file, obj);
				}
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
		std::remove(g_loadedDataFiles.begin(), g_loadedDataFiles.end(), dataFilePair);

		auto singlePair = { dataFilePair };

		HandleDataFileList(singlePair, [] (CDataFileMountInterface* mounter, DataFileEntry& entry)
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

static hook::cdecl_stub<rage::fiCollection*()> getRawStreamer([]()
{
	return hook::get_call(hook::get_pattern("48 8B D3 4C 8B 00 48 8B C8 41 FF 90 ? 01 00 00", -5));
});

static std::set<std::tuple<std::string, std::string>> g_customStreamingFiles;
static std::set<std::string> g_customStreamingFileRefs;
static std::map<std::string, std::vector<std::string>, std::less<>> g_customStreamingFilesByTag;
static std::unordered_map<int, std::list<uint32_t>> g_handleStack;
std::unordered_map<int, std::string> g_handlesToTag;

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

		auto baseName = std::string(slashPos + 1);
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
			if (ext == "ymap" || ext == "ytyp")
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
			strModule->GetOrCreate(&strId, nameWithoutExt.c_str());

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

static std::map<std::string, std::string, std::less<>> g_manifestNames;

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
	std::string name = g_manifestNames[tagName];

	if (!name.empty())
	{
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

void ForAllStreamingFiles(const std::function<void(const std::string&)>& cb)
{
	for (auto& entry : g_customStreamingFileRefs)
	{
		cb(entry);
	}
}

#include <nutsnbolts.h>

static bool g_reloadStreamingFiles;

void origCfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);

void DLL_EXPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags)
{
	auto baseName = std::string(strrchr(fileName.c_str(), '/') + 1);

	if (baseName == "_manifest.ymf")
	{
		g_manifestNames[tag] = fileName;
	}

	g_customStreamingFilesByTag[tag].push_back(fileName);
	g_customStreamingFiles.insert({ fileName, tag });
	g_customStreamingFileRefs.insert(baseName);

	origCfxCollection_AddStreamingFileByTag(tag, fileName, flags);
}

void DLL_EXPORT CfxCollection_RemoveStreamingTag(const std::string& tag)
{
	for (auto& file : g_customStreamingFilesByTag[tag])
	{
		// get basename ('thing.ytd') and asset name ('thing')
		auto baseName = std::string(strrchr(file.c_str(), '/') + 1);
		auto nameWithoutExt = baseName.substr(0, baseName.find_last_of('.'));

		// get CStreaming instance and associated streaming module
		auto cstreaming = streaming::Manager::GetInstance();
		auto strModule = cstreaming->moduleMgr.GetStreamingModule(strrchr(baseName.c_str(), '.') + 1);

		if (strModule)
		{
			uint32_t strId;
			strModule->GetIndexByName(&strId, nameWithoutExt.c_str());

			auto rawStreamer = getRawStreamer();
			uint32_t idx = (rawStreamer->GetCollectionId() << 16) | rawStreamer->GetEntryByName(file.c_str());

			if (strId != -1)
			{
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
					// TODO: fully delete the streaming object from the module/streamer
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

#include <GameInit.h>

static HookFunction hookFunction([] ()
{
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

		/*if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			// toggle map group again?
			g_enableContentGroup(*g_extraContentManager, 0xBCC89179); // GROUP_MAP
			g_disableContentGroup(*g_extraContentManager, 0xBCC89179);

			g_clearContentCache(0);
		}*/
	}, 99900);

	OnMainGameFrame.Connect([=]()
	{
		if (g_reloadStreamingFiles)
		{
			LoadStreamingFiles();

			g_reloadStreamingFiles = false;
		}
	});

	{
		char* location = hook::get_pattern<char>("48 63 82 90 00 00 00 49 8B 8C C0 ? ? ? ? 48", 11);

		g_dataFileMounters = (decltype(g_dataFileMounters))(0x140000000 + *(int32_t*)location); // why is this an RVA?!
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

	// support CfxRequest for pgRawStreamer
	hook::jump(hook::get_pattern("4D 63 C1 41 8B C2 41 81 E2 FF 03 00 00", -0xD), pgRawStreamer__GetEntryNameToBuffer);
});
