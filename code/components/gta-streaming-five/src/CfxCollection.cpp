/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <fiCollectionWrapper.h>

#include <Hooking.h>

#include <fnv.h>

#include <Error.h>

//#define CFX_COLLECTION_DISABLE 1

// unset _DEBUG so that there will be no range checking
#ifdef _DEBUG
#undef _DEBUG
#define DEBUG_WAS_SET
#endif

#include <concurrent_unordered_map.h>
#include <unordered_set>

#ifdef DEBUG_WAS_SET
#define _DEBUG
#undef DEBUG_WAS_SET
#endif

#include <mutex>

#include <sstream>

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>

#pragma comment(lib, "shlwapi.lib")
#include <shlwapi.h>

#include <Streaming.h>

#include <atArray.h>

// TODO: replace with C++14 transparent comparison
struct StringRef
{
	inline StringRef(const char* string)
		: m_stringPtr(string)
	{
	}

	inline explicit StringRef(const std::string& string)
		: m_string(std::make_unique<std::string>(string)), m_stringPtr(nullptr)
	{
	}

	inline const char* c_str() const
	{
		if (m_stringPtr)
		{
			return m_stringPtr;
		}
		
		return m_string->c_str();
	}

private:
	const char* m_stringPtr;
	std::unique_ptr<std::string> m_string;
};

struct IgnoreCaseHash
{
	inline size_t operator()(const std::string& value) const
	{
		return fnv1a_size_lower_t()(value);
	}

	inline size_t operator()(const StringRef& value) const
	{
		return fnv1a_size_lower_t()(value.c_str());
	}
};

struct IgnoreCaseEqualTo
{
	bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) == 0;
	}

	bool operator()(const StringRef& left, const StringRef& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) == 0;
	}
};


struct CollectionData
{
	uint64_t pad;
	void* nameTable;
	uint16_t* parentDirectoryTable;
	rage::fiCollection::FileEntry* entryTable;
	uint32_t numEntries;
	uint32_t pad2;
	uint64_t parentHandle;
	rage::fiDevice* unkDevice;
	char pad4[16];
	rage::fiDevice* parentDevice;
	char pad5[4];
	char smallName[32];
	uint32_t pad6;
	atArray<char> name;
	__declspec(align(8)) char pad7[32];
	uint32_t entryTableAllocSize;
	uint32_t keyId;
};


static uint32_t(*rage__fiFile__Read)(void* file, void* read, uint32_t size);
static uint32_t(*rage__fiFile__Write)(void* file, const void* write, uint32_t size);

namespace rage
{
	class fiFile
	{
	public:
		uint32_t Read(void* buffer, uint32_t size)
		{
			return rage__fiFile__Read(this, buffer, size);
		}

		uint32_t Write(const void* buffer, uint32_t size)
		{
			return rage__fiFile__Write(this, buffer, size);
		}
	};
}

static rage::fiCollection** g_collectionRoot;

static void(*g_origCloseCollection)(rage::fiCollection*);
static void(*g_origOpenPackfileInternal)(rage::fiCollection*, const char* archive, bool bTrue, intptr_t veryFalse);
static bool(*g_origLoadFromCache)(rage::fiCollection*, rage::fiFile*);
static bool(*g_origSaveToCache)(rage::fiCollection*, rage::fiFile*);

static uintptr_t g_vTable_fiPackfile;

static hook::thiscall_stub<rage::fiCollection*(rage::fiCollection*)> packfileCtor([] ()
{
	return hook::pattern("44 89 41 28 4C 89 41 38 4C 89 41 50 48 8D").count(1).get(0).get<void>(-0x1E);
});

#define GET_HANDLE(x) ((x) & 0x7FFFFFFF)
#define UNDEF_ASSERT() FatalError("Undefined function " __FUNCTION__)

static atArray<StreamingPackfileEntry>* g_streamingPackfiles;

struct IgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

static std::unordered_set<StringRef, IgnoreCaseHash, IgnoreCaseEqualTo> g_customStreamingFileSet;
static std::unordered_set<std::string, IgnoreCaseHash, IgnoreCaseEqualTo> g_ignoredStreamingFileSet;
static std::vector<int> g_streamingCollections;
static std::map<std::string, std::tuple<std::string, rage::ResourceFlags>, std::less<>> g_customStreamingFileRefs;

static std::set<rage::fiCollection*> g_cfxCollections;

class CfxCollection : public rage::fiCollection
{
private:
	struct CollectionEntry
	{
		FileEntry baseEntry;

		const FileEntry* origEntry;
		std::string fileName;

		bool valid;
	};

	struct HandleEntry
	{
		rage::fiDevice* parentDevice;
		int64_t parentHandle;

		const char* name;

		CollectionEntry* ourEntry;
	};

private:
	// to be filled with packfile data
	char m_pad[184];

	char m_childPackfile[192];

	rage::fiCollection* m_parentCollection;

	concurrency::concurrent_unordered_map<uint16_t, CollectionEntry> m_entries;

	concurrency::concurrent_unordered_map<std::string, uint16_t> m_reverseEntries;

	HandleEntry m_handles[512];

	std::set<std::string, IgnoreCaseLess> m_streamingFileList;

	bool m_hasCleaned;

	std::recursive_mutex m_mutex;

	std::unordered_map<std::string, rage::ResourceFlags> m_resourceFlags;

	std::vector<uint32_t> m_nameOffsetTable;
	
public:
	typedef std::function<bool(const char*, std::string&)> TLookupFn;

private:
	bool m_isPseudoPack;

private:
	bool IsPseudoPack();

	static bool IsPseudoPackPath(const char* path);

	void InitializePseudoPack(const char* path);

private:
	TLookupFn m_lookupFunction;

private:
	struct PseudoCallContext
	{
	private:
		rage::fiCollection* m_child;
		CfxCollection* m_collection;

		bool m_oldVal;

	public:
		PseudoCallContext(CfxCollection* collection)
			: m_collection(collection)
		{
			m_collection->m_mutex.lock();

			memcpy(collection->m_childPackfile, collection, sizeof(collection->m_childPackfile));
			*(uintptr_t*)collection->m_childPackfile = g_vTable_fiPackfile;

			m_child = reinterpret_cast<rage::fiCollection*>(collection->m_childPackfile);
		}

		~PseudoCallContext()
		{
			memcpy(reinterpret_cast<char*>(m_collection) + 8, &m_collection->m_childPackfile[8], sizeof(m_collection->m_childPackfile) - 8);

			m_collection->m_mutex.unlock();
		}

		rage::fiCollection* operator->() const
		{
			return m_child;
		}

		rage::fiCollection* GetPointer() const
		{
			return m_child;
		}
	};

public:
	CfxCollection(rage::fiCollection* parent)
		: m_parentCollection(parent)
	{
		memset(m_pad, 0, sizeof(m_pad));

		uintptr_t ourVt = *(uintptr_t*)this;
		packfileCtor(this);
		*(uintptr_t*)this = ourVt;

		memset(m_handles, 0, sizeof(m_handles));

		m_lookupFunction = [] (const char*, std::string&)
		{
			return false;
		};

		m_hasCleaned = false;
		m_isPseudoPack = false;

		g_cfxCollections.insert(this);
	}

	~CfxCollection()
	{
		trace("del cfxcollection %p\n", (void*)this);

		g_cfxCollections.erase(this);

		PseudoCallContext(this)->~fiCollection();

		rage::GetAllocator()->free(m_parentCollection);
	}

	void CleanCloseCollection()
	{
		if (m_isPseudoPack)
		{
			trace("close packfile %s\n", GetName());
		}

		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		m_entries.clear();
		m_reverseEntries.clear();

		m_isPseudoPack = false;

		memset(m_handles, 0, sizeof(m_handles));

		m_hasCleaned = true;
	}

	virtual void CloseCollection() override
	{
		*(reinterpret_cast<char*>(this) + 186) = true;

		PseudoCallContext(this)->CloseCollection();

		CleanCloseCollection();
	}

	uint64_t AllocateHandle(rage::fiDevice* parentDevice, uint64_t parentHandle, const char* name)
	{
		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		if (parentHandle == -1)
		{
			trace("CfxCollection[%s]::AllocateHandle got passed a failed handle for %s\n", GetName(), name);

			if (strcmp(GetName(), "dlc.rpf") != 0)
			{
				FatalError("CfxCollection[%s]::AllocateHandle got passed a failed handle for %s\n", GetName(), name);
			}

			return -1;
		}

		m_handles[0].parentDevice = this;

		for (int i = 0; i < _countof(m_handles); i++)
		{
			if (m_handles[i].parentDevice == nullptr)
			{
				m_handles[i].parentDevice = parentDevice;
				m_handles[i].parentHandle = parentHandle;
				m_handles[i].name = name;

				return i | 0x80000000;
			}
		}

		assert(!"No free handles! " __FUNCTION__);

		return -1;
	}

	auto AddEntry(uint16_t idx, const std::string& name)
	{
		CollectionEntry newEntry = { 0 };
		newEntry.fileName = name;
		newEntry.valid = true;
		
		rage::ResourceFlags flags;
		rage::fiDevice* baseDevice = rage::fiDevice::GetDevice(name.c_str(), true);

		if (m_resourceFlags.find(name) != m_resourceFlags.end())
		{
			flags = m_resourceFlags[name];
		}
		else
		{
			baseDevice->GetResourceVersion(name.c_str(), &flags);
		}

		memcpy(&newEntry.baseEntry, PseudoCallContext(this)->GetEntry(idx), sizeof(newEntry.baseEntry));

		newEntry.baseEntry.size = baseDevice->GetFileLengthLong(name.c_str());

		newEntry.baseEntry.virtFlags = flags.flag1;
		newEntry.baseEntry.physFlags = flags.flag2;

		m_reverseEntries[name] = idx;

		return m_entries.insert({ idx, newEntry }).first;
	}

	auto AddEntry(uint16_t idx, const FileEntry* entry)
	{
		CollectionEntry newEntry;
		newEntry.origEntry = entry;
		newEntry.valid = true;
		memcpy(&newEntry.baseEntry, entry, sizeof(FileEntry));

		return m_entries.insert({ idx, newEntry }).first;
	}

	CollectionEntry* GetCfxEntry(uint16_t index)
	{
		// map network entries that shouldn't exist
		CollectionData* collectionData = (CollectionData*)m_pad;
		char* nameTable = (char*)collectionData->nameTable;
		
		if (!nameTable || !collectionData->entryTable)
		{
			return nullptr;
		}

		auto it = m_entries.find(index);
		auto& nentry = collectionData->entryTable[index];

		if (!m_nameOffsetTable.empty())
		{
			nentry.nameOffset = m_nameOffsetTable[index];
		}

		char* name = &nameTable[nentry.nameOffset];

		if (!IsPseudoPack())
		{
			if (g_customStreamingFileSet.find(name) != g_customStreamingFileSet.end())
			{
				trace(__FUNCTION__ ": 'killing' %s\n", name);

				strcpy(name, va("%04x", HashRageString(name) & 0xFFFF));
			}
		}

		if (it == m_entries.end() || !it->second.valid)
		{
			m_entries.unsafe_erase(index);

			char entryName[256] = { 0x1E };
			PseudoCallContext(this)->GetEntryNameToBuffer(index, entryName, sizeof(entryName));

			//trace("init entry: %s -> %s\n", m_parentCollection->GetName(), entryName);

			//assert(isalpha(entryName[0]) || isdigit(entryName[0]) || entryName[0] == '_');

			std::string newName;
			if (m_lookupFunction(entryName, newName))
			{
				it = AddEntry(index, newName);
			}
			else
			{
				g_ignoredStreamingFileSet.insert(entryName);

				auto rit = g_customStreamingFileRefs.find(entryName);

				if (rit != g_customStreamingFileRefs.end())
				{
					auto n = std::get<std::string>(rit->second);
					m_resourceFlags[n] = std::get<rage::ResourceFlags>(rit->second);
					m_reverseEntries.unsafe_erase(entryName);

					// ignore streaming entries insert was here

					it = AddEntry(index, n);
				}
				else
				{
					m_reverseEntries[entryName] = index;
					it = AddEntry(index, PseudoCallContext(this)->GetEntry(index));

					if (entryName[0] != 0x1E)
					{
						it->second.fileName = entryName;
					}
				}
			}
		}

		return &it->second;
	}

	virtual int64_t OpenCollectionEntry(uint16_t index, uint64_t* ptr) override
	{
		//return PseudoCallContext(this)->OpenCollectionEntry(index, ptr);

		auto entry = GetCfxEntry(index);

		if (!entry)
		{
			return AllocateHandle(this, PseudoCallContext(this)->OpenCollectionEntry(index, ptr), "<unknown>");
		}

		if (entry->origEntry)
		{
			char entryName[256] = { 0 };

			PseudoCallContext(this)->GetEntryNameToBuffer(index, entryName, 255);

			CollectionData* data = (CollectionData*)this->m_pad;

			return AllocateHandle(this, PseudoCallContext(this)->OpenCollectionEntry(index, ptr), entry->fileName.c_str());// GetEntryName(index));
		}

		trace("coll open %s\n", entry->fileName.c_str());

		rage::fiDevice* device = rage::fiDevice::GetDevice(entry->fileName.c_str(), true);
		uint64_t handle = device->OpenBulk(entry->fileName.c_str(), ptr);

		return AllocateHandle(device, handle, GetEntryName(index));

		//return AllocateHandle(this, PseudoCallContext(this)->OpenCollectionEntry(index, ptr), "");
	}

	virtual const FileEntry* GetEntry(uint16_t index) override
	{
		auto entry = GetCfxEntry(index);

		if (!entry)
		{
			return PseudoCallContext(this)->GetEntry(index);
		}

		return (entry->origEntry) ? entry->origEntry : &entry->baseEntry;

		//return PseudoCallContext(this)->GetEntry(index);
	}

	virtual const char* GetEntryName(uint16_t index) override
	{
		auto entry = GetCfxEntry(index);

		if (!entry)
		{
			return PseudoCallContext(this)->GetEntryName(index);
		}

		const char* entryName = (entry->origEntry) ? PseudoCallContext(this)->GetEntryName(index) : entry->fileName.c_str();

		if (!IsPseudoPack())
		{
			if (g_customStreamingFileSet.find(entryName) != g_customStreamingFileSet.end())
			{
				trace(__FUNCTION__ " mapping %s to not exist\n", entryName);

				return va("%08x", HashRageString(entryName));
			}
		}

		return entryName;

		//return PseudoCallContext(this)->GetEntryName(index);
	}

	virtual int64_t Unk1(uint16_t index, bool flag)
	{
		auto entry = GetCfxEntry(index);

		return 0x00000000;

		//if (entry->origEntry)
		{
			//trace("Unk1 - %s\n", GetName());

			return PseudoCallContext(this)->Unk1(index, flag);
		}

		UNDEF_ASSERT();
	}

	virtual void GetEntryNameToBuffer(uint16_t index, char* buffer, int maxLen)
	{
		auto entry = GetCfxEntry(index);

		if (strcmp(buffer, "CfxRequest") == 0)
		{
			if (entry)
			{
				strcpy_s(buffer, maxLen, entry->fileName.c_str());
				return;
			}
		}

		if (!entry)
		{
			PseudoCallContext(this)->GetEntryNameToBuffer(index, buffer, maxLen);
			return;
		}

		if (entry->origEntry)
		{
			PseudoCallContext(this)->GetEntryNameToBuffer(index, buffer, maxLen);
		}
		else
		{
			PseudoCallContext(this)->GetEntryNameToBuffer(index, buffer, maxLen);
			//strcpy_s(buffer, maxLen, entry->fileName.c_str());
		}

		if (!IsPseudoPack())
		{
			if (g_customStreamingFileSet.find(buffer) != g_customStreamingFileSet.end())
			{
				trace(__FUNCTION__ " mapping %s to not exist\n", buffer);

				strcpy(buffer, va("%08x", HashRageString(buffer)));
			}
		}
	}

	virtual uint16_t GetEntryByName(const char* name)
	{
		auto it = m_reverseEntries.find(name);

		if (it == m_reverseEntries.end())
		{
			uint16_t index = PseudoCallContext(this)->GetEntryByName(name);

			GetCfxEntry(index);

			return index;
		}

		return it->second;
	}

	virtual int32_t GetUnk1()
	{
		return PseudoCallContext(this)->GetUnk1();
	}

	virtual bool UnkA(uint16_t a1) override
	{
		return PseudoCallContext(this)->UnkA(a1);
	}

	virtual bool CloseBasePackfile()
	{
		return PseudoCallContext(this)->CloseBasePackfile();
	}

	virtual uint8_t UnkC() override
	{
		return PseudoCallContext(this)->UnkC();
	}

	virtual bool UnkD(uint8_t value) override
	{
		return PseudoCallContext(this)->UnkD(value);
	}

	virtual void* UnkE(const char* a1) override
	{
		return PseudoCallContext(this)->UnkE(a1);
	}

	virtual uint64_t UnkF(void* a1, bool a2) override
	{
		return PseudoCallContext(this)->UnkF(a1, a2);
	}

	virtual uint64_t Open(const char* fileName, bool readOnly)
	{
		std::string outFileName;

		if (m_lookupFunction(fileName, outFileName))
		{
			rage::fiDevice* device = rage::fiDevice::GetDevice(outFileName.c_str(), true);
			uint64_t handle = device->Open(outFileName.c_str(), readOnly);

			return AllocateHandle(device, handle, outFileName.c_str());
		}

		return AllocateHandle(this, PseudoCallContext(this)->Open(fileName, readOnly), fileName);
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr)
	{
		std::string outFileName;

		if (m_lookupFunction(fileName, outFileName))
		{
			rage::fiDevice* device = rage::fiDevice::GetDevice(outFileName.c_str(), true);
			uint64_t handle = device->OpenBulk(outFileName.c_str(), ptr);

			return AllocateHandle(device, handle, outFileName.c_str());
		}

		uint64_t handle = PseudoCallContext(this)->OpenBulk(fileName, ptr);

		return AllocateHandle(this, handle, fileName);
	}

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*)
	{
		return OpenBulk(fileName, ptr);
	}

	virtual uint64_t Create(const char* fileName)
	{
		return PseudoCallContext(this)->Create(fileName);
	}

	virtual uint64_t CreateLocal(const char* fileName)
	{
		return PseudoCallContext(this)->CreateLocal(fileName);
	}

private:
	template<typename TFunc>
	auto InvokeOnHandle(uint64_t handle, const TFunc& func)
	{
		// return an appropriate -1 error value if the handle is -1
		if (handle == -1)
		{
			return static_cast<decltype(func(nullptr, -1))>(-1);
		}

		auto ourHandle = GET_HANDLE(handle);
		auto& handleItem = m_handles[ourHandle];

		if (handleItem.parentDevice == this)
		{
			{
				PseudoCallContext ctx(this);
				return func(ctx.GetPointer(), handleItem.parentHandle);
			}
		}
		else
		{
			return func(handleItem.parentDevice, handleItem.parentHandle);
		}
	}

public:
	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead)
	{
		return InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->Read(handle, buffer, toRead);
		});
	}

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
	{
		uint32_t size = InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->ReadBulk(handle, ptr, buffer, toRead);
		});

		if (size != toRead)
		{
			FatalError("CfxCollection::ReadBulk of streaming file %s failed to read %d bytes (got %d).", m_handles[GET_HANDLE(handle)].name, toRead, size);
		}

		return size;

		//return PseudoCallContext(this)->ReadBulk(handle, ptr, buffer, toRead);
	}

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int)
	{
		UNDEF_ASSERT();

		return 0;
	}

	virtual uint32_t Write(uint64_t, void*, int)
	{
		UNDEF_ASSERT();

		return 0;
	}

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method)
	{
		//return PseudoCallContext(this)->Seek(m_handles[handle].parentHandle, distance, method);
		return InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->Seek(handle, distance, method);
		});
	}

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method)
	{
		//return PseudoCallContext(this)->SeekLong(m_handles[handle].parentHandle, distance, method);
		return InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->SeekLong(handle, distance, method);
		});
	}

	virtual int32_t Close(uint64_t handle)
	{
		int32_t retval = InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->Close(handle);
		});

		auto& handleInfo = m_handles[GET_HANDLE(handle)];
		handleInfo.parentDevice = nullptr;

		return retval;
	}

	virtual int32_t CloseBulk(uint64_t handle)
	{
		int32_t retval = InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->CloseBulk(handle);
		});

		auto& handleInfo = m_handles[GET_HANDLE(handle)];
		handleInfo.parentDevice = nullptr;

		return retval;
	}

	virtual int GetFileLength(uint64_t handle)
	{
		return InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->GetFileLength(handle);
		});
	}

	virtual uint64_t GetFileLengthUInt64(uint64_t handle)
	{
		return InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->GetFileLengthUInt64(handle);
		});
	}

	// dummy!
	virtual int m_40(int)
	{
		return 0;
	}

	virtual bool RemoveFile(const char* file)
	{
		UNDEF_ASSERT();

		return 0;
	}
	virtual int RenameFile(const char* from, const char* to)
	{
		UNDEF_ASSERT();

		return 0;
	}
	virtual int CreateDirectory(const char* dir)
	{
		UNDEF_ASSERT();

		return 0;
	}

	virtual int RemoveDirectory(const char * dir)
	{
		UNDEF_ASSERT();

		return 0;
	}

	virtual void m_xx()
	{

	}

	virtual uint64_t GetFileLengthLong(const char* fileName)
	{
		return PseudoCallContext(this)->GetFileLengthLong(fileName);
	}

	virtual uint32_t GetFileTime(const char* file)
	{
		return PseudoCallContext(this)->GetFileTime(file);
	}
	virtual bool SetFileTime(const char* file, FILETIME fileTime)
	{
		UNDEF_ASSERT();

		return 0;
	}

	virtual uint64_t FindFirst(const char* path, rage::fiFindData* findData)
	{
		return PseudoCallContext(this)->FindFirst(path, findData);
	}

	virtual bool FindNext(uint64_t handle, rage::fiFindData* findData)
	{
		return PseudoCallContext(this)->FindNext(handle, findData);
	}

	virtual int FindClose(uint64_t handle)
	{
		return PseudoCallContext(this)->FindClose(handle);
	}

	virtual rage::fiDevice* GetUnkDevice()
	{
		return this;
	}

	virtual void* m_xy(void* a1, int a2, void* a3)
	{
		//UNDEF_ASSERT();
		return PseudoCallContext(this)->m_xy(a1, a2, a3);
	}

	virtual bool Truncate(uint64_t handle)
	{
		UNDEF_ASSERT();
		return true;
	}

	virtual uint32_t GetFileAttributes(const char* path)
	{
		return PseudoCallContext(this)->GetFileAttributes(path);
	}

	virtual bool m_xz()
	{
		return PseudoCallContext(this)->m_xz();
	}

	virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes)
	{
		return false;
	}

	virtual int m_yx()
	{
		return PseudoCallContext(this)->m_yx();
	}

	// read even if read() returns less than length
	virtual bool ReadFull(uint64_t handle, void* buffer, uint32_t length)
	{
		return InvokeOnHandle(handle, [&] (rage::fiDevice* device, uint64_t handle)
		{
			return device->ReadFull(handle, buffer, length);
		});
	}

	virtual bool WriteFull(uint64_t handle, void* buffer, uint32_t length)
	{
		return false;
	}

	virtual int32_t GetResourceVersion(const char* fileName, rage::ResourceFlags* version)
	{
		return PseudoCallContext(this)->GetResourceVersion(fileName, version);
	}

	virtual int32_t m_yy()
	{
		return PseudoCallContext(this)->m_yy();
	}

	virtual int32_t m_yz(void* a1)
	{
		return PseudoCallContext(this)->m_yz(a1);
	}

	virtual int32_t m_zx(void* a1)
	{
		return PseudoCallContext(this)->m_zx(a1);
	}

	virtual bool IsBulkDevice()
	{
		return PseudoCallContext(this)->IsBulkDevice();
	}

	virtual fiDevice* m_zz() // return this
	{
		return this;
	}

	virtual bool m_ax()
	{
		return PseudoCallContext(this)->m_ax();
	}

	virtual int32_t GetCollectionId()
	{
		return PseudoCallContext(this)->GetCollectionId();
	}

	virtual const char* GetName()
	{
		/*if (!m_parentCollection || !*(uintptr_t*)m_parentCollection)
		{
			return "None";
		}*/

		//return va("Cfx:%s", PseudoCallContext(this)->GetName());
		return PseudoCallContext(this)->GetName();
	}

private:
	void BeforeNativeOpen(const char* archive);

	void AfterNativeOpen(const char* archive);

	void PrepareStreamingListFromNetwork();

	void PrepareStreamingListForTag(const char* tag);

	void PrepareStreamingListFromList(const std::vector<std::pair<std::string, rage::ResourceFlags>>& entries);

	void DetermineLookupFunction(const char* archive)
	{
		if (strstr(archive, "resource_surrogate:/") != nullptr)
		{
			PrepareStreamingListForTag(&archive[20]);

			return;
		}

		if (strstr(archive, "dunno") != nullptr)
		{
			PrepareStreamingListForTag(archive);

			return;
		}

		// if this is the streaming surrogate, treat it like such
		if (strstr(archive, "streaming_surrogate.rpf") != nullptr)
		{
			PrepareStreamingListFromNetwork();

			return;
		}

		// build a local path string to verify
		std::stringstream basePath;

		if (_strnicmp(archive, "usermaps:/", 10) == 0)
		{
			const char* colon = strchr(archive, ':');

			// temporary: make citizen/ path manually
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			basePath << converter.to_bytes(MakeRelativeCitPath(L"/"));

			basePath << std::string(archive, colon);
			basePath << "/";
			basePath << std::string(&colon[1], const_cast<const char*>(StrStrIA(colon, ".rpf")));
		}
		else if (!IsPseudoPackPath(archive) || _strnicmp(archive, "pseudoPack:/", 12) == 0)
		{
			const char* colon = strchr(archive, ':');

			// temporary: make citizen/ path manually
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			basePath << converter.to_bytes(MakeRelativeCitPath(L"citizen/"));

			basePath << std::string(archive, colon);
			basePath << "/";
			basePath << std::string(&colon[1], const_cast<const char*>(strrchr(colon, '.')));
		}
		else
		{
			basePath << std::string(archive, const_cast<const char*>(strrchr(archive, '.')));
		}

		// get the folder device
		std::string str = basePath.str();
		rage::fiDevice* device = rage::fiDevice::GetDevice(str.c_str(), true);

		// if it's a valid device
		if (device)
		{
			// look for files in the path
			rage::fiFindData findData;

			uint64_t findHandle = device->FindFirst(str.c_str(), &findData);

			if (findHandle != (uint64_t)-1)
			{
				// read the list
				std::set<std::string, IgnoreCaseLess> fileList;

				do
				{
					if (findData.fileName[0] != '.')
					{
						fileList.insert(findData.fileName);
					}
				} while (device->FindNext(findHandle, &findData));

				device->FindClose(findHandle);

				// create a lookup function
				m_lookupFunction = [=] (const char* filename, std::string& newFilename)
				{
					// if this is an absolute filename...
					if (strchr(filename, ':'))
					{
						// try looking it up on disk, with the mount removed
						std::string tryFileName = str + "/" + &filename[*(uint32_t*)(&m_pad[80])]; // + 88 is mountpoint length

						if (device->GetFileAttributes(tryFileName.c_str()) != INVALID_FILE_ATTRIBUTES)
						{
							newFilename = tryFileName;

							return true;
						}
					}
					else
					{
						// else try looking it up in the streaming list
						auto it = fileList.find(filename);

						if (it != fileList.end())
						{
							newFilename = str + "/" + *it;

							return true;
						}
					}

					return false;
				};

				// if there are any streaming files, somewhat invalidate the parent packfile timestamp
				SYSTEMTIME systemTime;
				GetSystemTime(&systemTime);

				FILETIME fileTime;
				SystemTimeToFileTime(&systemTime, &fileTime);

				g_streamingPackfiles->Get(GetCollectionId()).modificationTime = fileTime;

				m_streamingFileList = fileList;

				g_streamingCollections.push_back(GetCollectionId());
			}
		}
	}

public:

	// functions
	bool OpenPackfile(const char* archive, bool bTrue, int type, intptr_t veryFalse);

	bool LoadFromCache(rage::fiFile* cacheFile);

	bool SaveToCache(rage::fiFile* cacheFile);

	bool OpenPackfileInternal(const char* archive, bool bTrue, intptr_t veryFalse)
	{
		DetermineLookupFunction(archive);

		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		rage::fiDevice* unk = *(rage::fiDevice**)((reinterpret_cast<char*>(this) + 80));

		if (unk && dynamic_cast<CfxCollection*>(unk))
		{
			//trace("Tried to open a CfxCollection inside a CfxCollection. Is this bad?\n");
		}

		if (!IsPseudoPackPath(archive))
		{
			assert(!IsPseudoPack());

			{
				PseudoCallContext ctx(this);
				g_origOpenPackfileInternal(ctx.GetPointer(), archive, bTrue, veryFalse);
			}

			if (*(int64_t*)(&m_pad[40]) == -1)
			{
				OpenPackfile(archive, bTrue, 3, veryFalse);

				if (*(int64_t*)(&m_pad[40]) == -1)
				{
					FatalError("Reading packfile %s header failed, even after retrying.", archive);
				}
			}
		}
		else
		{
			/*{
				PseudoCallContext ctx(this);
				g_origOpenPackfileInternal(ctx.GetPointer(), "platform:/levels/gta5/_cityw/beverly_01/bh1_07.rpf", bTrue, veryFalse);
			}*/

			InitializePseudoPack(archive);
		}

		return true;
	}

	void Invalidate()
	{
		for (auto& entry : m_entries)
		{
			entry.second.valid = false;
		}

		m_resourceFlags.clear();
	}

	void Mount(const char* mountPoint)
	{
		{
			PseudoCallContext ctx(this);

			reinterpret_cast<rage::fiPackfile*>(ctx.GetPointer())->Mount(mountPoint);
		}
	}
};

static rage::fiCollection* ConstructPackfile(rage::fiCollection* packfile)
{
	rage::fiCollection* cfxCollection = new CfxCollection(packfile);

	g_collectionRoot[cfxCollection->GetCollectionId()] = cfxCollection;

	return cfxCollection;
}

void DoCloseCollection(rage::fiCollection* collection)
{
	auto cfxCollection = dynamic_cast<CfxCollection*>(collection);

	if (cfxCollection != nullptr)
	{
		// don't delete the name table
		*(reinterpret_cast<char*>(collection) + 186) = true;

		g_origCloseCollection(collection);
		cfxCollection->CleanCloseCollection();
	}
	else
	{
		g_origCloseCollection(collection);
	}
}

static void PtrError()
{
	if (CoreIsDebuggerPresent())
	{
		__debugbreak();
	}

	FatalError("Invalid fixup, address is neither virtual nor physical (rage::pg*)");
}

// this should be moved to another component eventually...
#ifdef DRAWABLE_DBG
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

static void(*g_origDrawDrawable)(void* thing, void* a2, void* a3, void* a4);

void CustomDrawDrawable(char* thing, void* a2, void* a3, void* a4)
{
	/*std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::wstring b = converter.from_bytes(*(char**)(thing + 0xA8));

	D3DPERF_BeginEvent(D3DCOLOR_ARGB(0, 0, 0, 0), b.c_str());*/
	if (strstr(*(char**)(thing + 0xA8), "#dr"))
	{
		printf("");
	}

	g_origDrawDrawable(thing, a2, a3, a4);
	//D3DPERF_EndEvent();
}

static void(*g_origDrawDrawable2)(void* thing, void* a2, void* a3, void* a4, void*, void*);

void CustomDrawDrawable2(char* thing, void* a2, void* a3, void* a4, void* a5, void* a6)
{
	/*std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::wstring b = converter.from_bytes(*(char**)(thing + 0xA8));

	D3DPERF_BeginEvent(D3DCOLOR_ARGB(0, 0, 0, 0), b.c_str());*/
	if (strstr(*(char**)(thing + 0xA8), "#dr"))
	{
		printf("");
	}

	g_origDrawDrawable2(thing, a2, a3, a4, a5, a6);
	//D3DPERF_EndEvent();
}

static void*(*g_origDrawDrawableR)(void* thing, void* a2, void* a3, void* a4, void*);

void* CustomDrawDrawableR(char* thing, void* a2, void* a3, void* a4, void* a5)
{
	/*std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::wstring b = converter.from_bytes(*(char**)(thing + 0xA8));

	D3DPERF_BeginEvent(D3DCOLOR_ARGB(0, 0, 0, 0), b.c_str());*/
	bool l = false;

	if (strstr(*(char**)(thing + 0xA8), "lovely.#dr"))
	{
		l = true;
		D3DPERF_BeginEvent(D3DCOLOR_ARGB(0, 0, 0, 0), L"lovely.ydr");
	}

	void* rv = g_origDrawDrawableR(thing, a2, a3, a4, a5);

	if (l)
	{
		D3DPERF_EndEvent();
	}

	return rv;
	//D3DPERF_EndEvent();
}

static void*(*g_origDrawDrawableR2)(void* thing, void* a2, void* a3, void* a4, void* a5, void*);

void* CustomDrawDrawableR2(char* thing, void* a2, void* a3, void* a4, void* a5, void* a6)
{
	/*std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::wstring b = converter.from_bytes(*(char**)(thing + 0xA8));

	D3DPERF_BeginEvent(D3DCOLOR_ARGB(0, 0, 0, 0), b.c_str());*/
	bool l = false;

	if (strstr(*(char**)(thing + 0xA8), "lovely.#dr"))
	{
		l = true;
		D3DPERF_BeginEvent(D3DCOLOR_ARGB(0, 0, 0, 0), L"lovely.ydr");
	}

	void* rv = g_origDrawDrawableR2(thing, a2, a3, a4, a5, a6);

	if (l)
	{
		D3DPERF_EndEvent();
	}

	return rv;
	//D3DPERF_EndEvent();
}

static void*(*g_origResolveQB)(void* qb, void* bm);

static void(*g_resolvePtr)(uintptr_t* ptr);

void* ResolveQB(char* qb, void* bm)
{
	bool isFlagged = false;
	void* rv;

	if (*(uintptr_t*)qb >= 0x7FF600000000)
	{
		isFlagged = true;

		static bool nagged = false;

		if (!nagged)
		{
			//assert(!"HEY ATTACH A DEBUGGER");
			*(uintptr_t*) qb = 0x50064000;

			g_resolvePtr((uintptr_t*)qb);

			uintptr_t ptr = *(uintptr_t*)qb;

			nagged = true;
		}
	}

	rv = g_origResolveQB(qb, bm);

	if (isFlagged)
	{
		char* vb = *(char**)(qb + 24);
		char* vf = *(char**)(vb + 48);

		trace("Cfx grmGeometryQB resolve\n");
	}

	return rv;
}
#endif

namespace rage
{
	class strStreamingModule
	{
	private:
		uint32_t m_baseIndex;

	public:
		virtual ~strStreamingModule();

		inline uint32_t GetBaseIndex()
		{
			return m_baseIndex;
		}
	};
}

struct StreamingEntry
{
	uint16_t a;
	uint16_t b;
	uint16_t c;
	uint16_t d;
};

static rage::strStreamingModule* g_streamingModule;
static StreamingEntry** g_streamingEntries;

static_assert(sizeof(StreamingPackfileEntry) == 104, "muh");

void CfxCollection::BeforeNativeOpen(const char* archive)
{
	DetermineLookupFunction(archive);
}

bool CfxCollection::LoadFromCache(rage::fiFile* cacheFile)
{
	uint8_t shouldRead;
	cacheFile->Read(&shouldRead, 1);

	if (!shouldRead)
	{
		char buf[10];
		cacheFile->Read(buf, 10);

		uint32_t numEntries;
		cacheFile->Read(&numEntries, 4);

		// however, pretend to read what was written.
		uint64_t r1;
		uint32_t r2;
		cacheFile->Read(&r1, 8);
		cacheFile->Read(&r2, 4);

		char dummyBuffer[1024];

		for (size_t i = 0; i < r2; i++)
		{
			cacheFile->Read(dummyBuffer, 1);
			cacheFile->Read(dummyBuffer, dummyBuffer[0]);
			cacheFile->Read(dummyBuffer, 4);
			cacheFile->Read(dummyBuffer, 16);
		}

		return false;
	}

	bool result;

	{
		PseudoCallContext ctx(this);
		result = g_origLoadFromCache(ctx.GetPointer(), cacheFile);
	}

	assert(result);

	if (result)
	{
		CollectionData* collectionData = (CollectionData*)m_pad;

		{
			std::vector<char> nameTableBuffer(32768);
			std::vector<uint16_t> parentDirectoryTableBuffer;
			size_t offset = 0;

			uint32_t tableEntries;
			cacheFile->Read(&tableEntries, sizeof(tableEntries));

			m_nameOffsetTable.reserve(tableEntries);
			parentDirectoryTableBuffer.reserve(tableEntries);

			for (size_t i = 0; i < tableEntries; i++)
			{
				uint32_t nameLength;
				cacheFile->Read(&nameLength, sizeof(nameLength));

				if ((offset + nameLength) >= nameTableBuffer.size())
				{
					nameTableBuffer.resize(nameTableBuffer.size() * 2);
				}

				cacheFile->Read(&nameTableBuffer[offset], nameLength);

				//collectionData->entryTable[i].nameOffset = offset;
				m_nameOffsetTable.push_back(offset);

				uint16_t parentDirectory;
				cacheFile->Read(&parentDirectory, 2);

				parentDirectoryTableBuffer.push_back(parentDirectory);

				offset += (nameLength + 1);
			}

			collectionData->nameTable = rage::GetAllocator()->allocate(nameTableBuffer.size(), 16, 0);
			memcpy(collectionData->nameTable, nameTableBuffer.data(), nameTableBuffer.size());

			collectionData->parentDirectoryTable = (uint16_t*)rage::GetAllocator()->allocate(parentDirectoryTableBuffer.size() * 2, 16, 0);
			memcpy(collectionData->parentDirectoryTable, parentDirectoryTableBuffer.data(), parentDirectoryTableBuffer.size() * 2);
		}

		const char* archive = &collectionData->name[0];

		BeforeNativeOpen(archive);

		rage::fiDevice* baseDevice = nullptr;

		if (!IsPseudoPackPath(archive))
		{
			baseDevice = rage::fiDevice::GetDevice(archive, true);
		}
		else
		{
			InitializePseudoPack(archive);
		}
	}

	return result;
}

bool CfxCollection::SaveToCache(rage::fiFile* cacheFile)
{
	uint8_t shouldRead = !IsPseudoPack();
	cacheFile->Write(&shouldRead, 1);

	CollectionData* collectionData = (CollectionData*)m_pad;

	if (!shouldRead)
	{
		char surrogateTag[] = { 's', 'u', 'r', 'r', 'o', 'g', 'a', 't', 'e', '\0' };

		cacheFile->Write(surrogateTag, sizeof(surrogateTag));

		// so we can reconcile reading this file later on
		cacheFile->Write(&collectionData->numEntries, 4);
		return false;
	}

	{
		PseudoCallContext ctx(this);
		g_origSaveToCache(ctx.GetPointer(), cacheFile);
	}

	char* nameTable = (char*)collectionData->nameTable;

	cacheFile->Write(&collectionData->numEntries, sizeof(collectionData->numEntries));

	for (size_t i = 0; i < collectionData->numEntries; i++)
	{
		char* name = &nameTable[collectionData->entryTable[i].nameOffset];
		uint32_t length = strlen(name);

		cacheFile->Write(&length, sizeof(length));
		cacheFile->Write(name, length);

		cacheFile->Write(&collectionData->parentDirectoryTable[i], 2);
	}

	return true;
}

bool CfxCollection::OpenPackfile(const char* archive, bool bTrue, int type, intptr_t veryFalse)
{
	BeforeNativeOpen(archive);

	rage::fiDevice* baseDevice = nullptr;

	if (!IsPseudoPackPath(archive))
	{
		baseDevice = rage::fiDevice::GetDevice(archive, true);
	}
	else
	{
		trace("open packfile %s\n", archive);
	}

	// weird workaround for fiDeviceRelative not passing this through
	bool isProxiedRelative = false;

	/*if (baseDevice && !baseDevice->IsBulkDevice() && !strstr(archive, "dlcpacks:"))
	{
		const char* mount = strstr(archive, ":/");

		if (!mount)
		{
			mount = strstr(archive, ":\\");
		}

		archive = va("update:/x64/%s", mount + 2);

		isProxiedRelative = true;
	}*/

	std::unique_lock<std::recursive_mutex> lock(m_mutex);

	if (baseDevice)
	{
		PseudoCallContext ctx(this);
		reinterpret_cast<rage::fiPackfile*>(ctx.GetPointer())->OpenPackfile(archive, bTrue, type, veryFalse);
	}
	else
	{
		{
			PseudoCallContext ctx(this);
			reinterpret_cast<rage::fiPackfile*>(ctx.GetPointer())->OpenPackfile("platform:/levels/gta5/_cityw/beverly_01/bh1_07.rpf", bTrue, type, veryFalse);
		}

		InitializePseudoPack(archive);
	}

	// dependency for game reloading (gta:core GameInit) - verify the parent collection address in the packfile registry
	if (baseDevice)
	{
		int baseCollectionId = baseDevice->GetCollectionId();

		auto& packfile = g_streamingPackfiles->Get(GetCollectionId());

		// if this is a packfile with an address already...
		if (!isProxiedRelative && packfile.parentIdentifier != 0)
		{
			uint16_t oldCollectionId = packfile.parentIdentifier >> 16;

			// and the collection ID is not the one mentioned in the address...
			if (oldCollectionId != baseCollectionId)
			{
				// and it isn't collection 0, either (pgRawStreamer, which we don't handle)
				if (baseCollectionId != 0)
				{
					// get the entry identifier and make a new address
					rage::fiCollection* collection = static_cast<rage::fiCollection*>(baseDevice);
					uint16_t entryIndex = collection->GetEntryByName(archive);

					packfile.parentIdentifier = (baseCollectionId << 16) | entryIndex;
				}
			}
		}
	}
	else
	{
		// set the parent identifier to our guinea pig's
		rage::fiCollection* collection = static_cast<rage::fiCollection*>(rage::fiDevice::GetDevice("platform:/levels/gta5/_cityw/beverly_01/bh1_07.rpf", true));
		uint16_t entryIndex = collection->GetEntryByName("platform:/levels/gta5/_cityw/beverly_01/bh1_07.rpf");

		int baseCollectionId = collection->GetCollectionId();

		auto& packfile = g_streamingPackfiles->Get(GetCollectionId());

		packfile.parentIdentifier = (baseCollectionId << 16) | entryIndex;
	}

	return true;
}

bool CfxCollection::IsPseudoPack()
{
	return m_isPseudoPack;
}

bool CfxCollection::IsPseudoPackPath(const char* path)
{
	// we better hope no legitimate pack is called 'pseudo' :D
	return strstr(path, "pseudo") != nullptr ||
		   strstr(path, "usermaps") != nullptr ||
		   strstr(path, "streaming_surrogate") != nullptr ||
		   strstr(path, "dunno") != nullptr ||
		   strstr(path, "resource_surrogate") != nullptr;
}

#include <numeric>
#include <sysAllocator.h>

void CfxCollection::InitializePseudoPack(const char* path)
{
	m_isPseudoPack = true;

	if (!m_streamingFileList.empty())
	{
		trace("Initializing pseudo-pack %s\n", path);
	}

	// clear existing items
	m_entries.clear();
	m_reverseEntries.clear();

	// process
	//assert(m_streamingFileList.size() > 0);

	auto nameTableSize = std::accumulate(m_streamingFileList.begin(), m_streamingFileList.end(), 2, [] (int left, const std::string& right)
	{
		return left + right.length() + 1;
	});

	auto nameTable = (char*)rage::GetAllocator()->allocate(nameTableSize, 16, 0);
	auto parentDirectoryTable = (uint16_t*)rage::GetAllocator()->allocate(sizeof(uint16_t) * (m_streamingFileList.size() + 1), 16, 0);
	auto entryTable = (rage::fiCollection::FileEntry*)rage::GetAllocator()->allocate(sizeof(rage::fiCollection::FileEntry) * (m_streamingFileList.size() + 1), 16, 0);
	auto entryCount = 0;

	{
		char* nameTablePtr = nameTable;
		
		auto addString = [&] (const std::string& string)
		{
			size_t strLength = string.length();
			memcpy(nameTablePtr, string.c_str(), strLength);
			nameTablePtr[strLength] = '\0';

			auto startIndex = nameTablePtr - nameTable;

			nameTablePtr += strLength + 1;

			return startIndex;
		};

		entryTable[0].nameOffset = addString("");
		entryTable[0].size = 0;
		entryTable[0].offset = 0x7FFFFF;
		entryTable[0].virtFlags = 1; // first entry
		entryTable[0].physFlags = m_streamingFileList.size(); // entry count

		parentDirectoryTable[0] = 0;

		int i = 1;

		for (auto& entry : m_streamingFileList)
		{
			if (g_ignoredStreamingFileSet.find(entry) != g_ignoredStreamingFileSet.end())
			{
				continue;
			}

			std::string entryFullName;
			m_lookupFunction(entry.c_str(), entryFullName);

			rage::ResourceFlags flags;
			auto entryDevice = rage::fiDevice::GetDevice(entryFullName.c_str(), true);

			if (m_resourceFlags.find(entryFullName) != m_resourceFlags.end())
			{
				flags = m_resourceFlags[entryFullName];
			}
			else
			{
				entryDevice->GetResourceVersion(entryFullName.c_str(), &flags);
			}

			entryTable[i].nameOffset = addString(entry);
			entryTable[i].offset = 0x8FFFFF;
			entryTable[i].size = entryDevice->GetFileLengthLong(entryFullName.c_str());
			entryTable[i].virtFlags = flags.flag1;
			entryTable[i].physFlags = flags.flag2;

			parentDirectoryTable[i] = 0;

			i++;
		}

		entryCount = i;

		if (entryCount > 1)
		{
			trace("Scanned packfile: %d entries.\n", entryCount);
		}
	}

	CollectionData* collectionData = (CollectionData*)m_pad;

	// set the new values
	collectionData->entryTable = entryTable;
	collectionData->nameTable = nameTable;
	collectionData->numEntries = entryCount;
	collectionData->parentDirectoryTable = parentDirectoryTable;

	collectionData->parentDevice = nullptr;
	collectionData->unkDevice = nullptr;

	collectionData->parentHandle = -1;

	collectionData->name.Expand(strlen(path) + 2);
	collectionData->name.Set(strlen(path), 0);
	strcpy(&collectionData->name.Get(0), path);

	std::string pathBits = path;
	pathBits = pathBits.substr(pathBits.find_last_of("/\\") + 1);

	if (pathBits.length() >= 32)
	{
		pathBits = pathBits.substr(0, 31);
	}

	strcpy_s(collectionData->smallName, pathBits.c_str());

	collectionData->entryTableAllocSize = sizeof(rage::fiCollection::FileEntry) * (m_streamingFileList.size() + 1);
	collectionData->keyId = 0;

	static_assert(sizeof(CollectionData) == 176, "m");
}

static std::vector<std::pair<std::string, rage::ResourceFlags>> g_customStreamingFiles;
static std::map<std::string, std::vector<std::pair<std::string, rage::ResourceFlags>>, std::less<>> g_customStreamingFilesByTag;

void DLL_EXPORT CfxCollection_AddStreamingFile(const std::string& fileName, rage::ResourceFlags flags)
{
#ifndef CFX_COLLECTION_DISABLE
	g_customStreamingFileSet.insert(StringRef(std::string(strrchr(fileName.c_str(), '/') + 1)));
	g_customStreamingFiles.push_back({ fileName, flags });
#endif
}

void DLL_EXPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags)
{
#ifndef CFX_COLLECTION_DISABLE
	auto baseName = std::string(strrchr(fileName.c_str(), '/') + 1);

	g_customStreamingFilesByTag[tag].push_back({ fileName, flags });

	if (baseName != "_manifest.ymf")
	{
		g_customStreamingFileRefs.insert({ baseName, { fileName, flags } });
	}
#endif
}

void ForAllStreamingFiles(const std::function<void(const std::string&)>& cb)
{
	for (auto& entry : g_customStreamingFileRefs)
	{
		cb(entry.first);
	}
}

void CfxCollection::PrepareStreamingListFromList(const std::vector<std::pair<std::string, rage::ResourceFlags>>& entries)
{
	std::set<std::string, IgnoreCaseLess> fileList;
	std::map<std::string, std::pair<std::string, rage::ResourceFlags>, IgnoreCaseLess> fileMapping;

	m_resourceFlags.clear();

	for (auto& file : entries)
	{
		std::string baseName = strrchr(file.first.c_str(), '/') + 1;

		if (g_ignoredStreamingFileSet.find(baseName) != g_ignoredStreamingFileSet.end())
		{
			continue;
		}

		fileMapping.insert({ baseName, file });
		fileList.insert(baseName);

		m_resourceFlags.insert(file);
	}

	m_lookupFunction = [=](const char* lookupName, std::string& fileName)
	{
		auto it = fileMapping.find(lookupName);

		if (it == fileMapping.end())
		{
			return false;
		}

		fileName = it->second.first;
		return true;
	};

	// as streaming may be slow, we should do it from a separate thread
	g_streamingPackfiles->Get(GetCollectionId()).isHdd = false;

	// if there are any streaming files, somewhat invalidate the parent packfile timestamp
	auto& packfileInfo = g_streamingPackfiles->Get(GetCollectionId());
	packfileInfo.modificationTime.dwHighDateTime = 0;
	packfileInfo.modificationTime.dwLowDateTime = 0;

	m_streamingFileList = fileList;

	g_streamingCollections.push_back(GetCollectionId());
}


void CfxCollection::PrepareStreamingListFromNetwork()
{
	PrepareStreamingListFromList(g_customStreamingFiles);
}

void CfxCollection::PrepareStreamingListForTag(const char* tag)
{
	// parse \ as apparently it expects a real fs path
	std::string tagName = tag;
	std::replace(tagName.begin(), tagName.end(), '\\', '/');

	tagName = tagName.substr(tagName.find_last_of('/') + 1);
	tagName = tagName.substr(0, tagName.find_last_of('.'));

	PrepareStreamingListFromList(g_customStreamingFilesByTag[tagName]);
}

static void(*g_origGeomThing)(void*, void*);

static hook::cdecl_stub<bool(void*)> calculateBVH([] ()
{
	return hook::pattern("B9 80 00 00 00 44 0F 29 48 88 44 0F").count(1).get(0).get<void>(-0x36);
});

static void DoGeomThing(char* a1, void* a2)
{
	g_origGeomThing(a1, a2);

	if (*(uintptr_t*)(a1 + 304) == 0)
	{
		calculateBVH(a1);
	}
}

static void(*g_origBvhSet)(int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6, int64_t a7);

static void BvhSet(int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6, int64_t a7)
{
	return g_origBvhSet(a1, a2, a3, a4, 0, 1, a7);
}

static char* g_registerStreamingFile;

static std::unordered_set<std::string, IgnoreCaseHash, IgnoreCaseEqualTo> g_registeredFileSet;
static std::map<uint32_t, std::string> g_hashes;

std::string g_lastStreamingName;

static const char* RegisterStreamingFileStrchrWrap(const char* str, const int ch)
{
	// set the name for the later hook to use
	g_lastStreamingName = str;

	// scan for the entry in the streaming list and patch as appropriate
	bool isNetworkFile = (g_customStreamingFileSet.find(str) != g_customStreamingFileSet.end());

	// DLC/update overriding - seems irrelevant
#if 0
	static char* startBit = hook::pattern("45 33 DB 85 C0 74 35 48 8B 05").count(1).get(0).get<char>(5);

	if (isNetworkFile)
	{
		hook::nop(startBit, 2);
		hook::put<uint16_t>(startBit + 49, 0xE990);
	}
	else
	{
		hook::put<uint16_t>(startBit, 0x3574);
		hook::put<uint16_t>(startBit + 49, 0x850F);
	}
#endif

	// if this file has not been registered yet, allow it to be for this one time
	// FIXME: clear this state if server reconnecting is ever to work again
	if (isNetworkFile)
	{
		if (g_registeredFileSet.find(str) == g_registeredFileSet.end())
		{
			g_registeredFileSet.insert(str);

			isNetworkFile = false;
		}
	}

	static char* startBit = hook::pattern("44 88 6C 1C 4F 44 3B F6 0F 84").count(1).get(0).get<char>(8);

	if (isNetworkFile)
	{
		hook::put<uint16_t>(startBit, 0xE990);
	}
	else
	{
		hook::put<uint16_t>(startBit, 0x840F);
	}

#if 0
	// temp: store hashes
	char nameWithoutExt[256];
	strcpy(nameWithoutExt, str);

	char* dot = strrchr(nameWithoutExt, '.');

	if (dot)
	{
		dot[0] = '\0';

		g_hashes[HashString(nameWithoutExt)] = nameWithoutExt;
	}
#endif

	// return strchr
	return strrchr(str, ch);
}

static void(*g_origOpenPackfiles)();

void OpenPackfilesWrap()
{
	// clear parent handles of streaming collections so we'll reload them
	for (auto& collection : g_streamingCollections)
	{
		auto& spf = g_streamingPackfiles->Get(collection);
		spf.modificationTime.dwHighDateTime = 0;
		spf.modificationTime.dwLowDateTime = 0;
		spf.loadedFlag = false;
		spf.packfileParentHandle = -1;

		trace("cleared collection metadata for %s\n", spf.packfile->GetName());
	}

	g_streamingCollections.clear();

	// reload the packfiles
	g_origOpenPackfiles();
}

static void(*g_origAddCollision)(void* module, uint32_t* outIdx, uint32_t* inHash);

void AddCollisionWrap(void* module, uint32_t* outIdx, uint32_t* inHash)
{
	uint32_t hash = *inHash;
	g_origAddCollision(module, outIdx, inHash);

#if 0
	trace("collision %d -> %s\n", *outIdx, g_hashes[hash].c_str());
#endif
}

void SetStreamingPackfileEnabled(uint32_t index, bool enabled)
{
	if (index != -1)
	{
		g_streamingPackfiles->Get(index).enabled = enabled;
	}
}

static void(*g_origRemoveStreamingPackfile)(uint32_t);

static void RemoveStreamingPackfileWrap(uint32_t index)
{
	g_origRemoveStreamingPackfile(index);

	(*g_streamingPackfiles)[index].loadedFlag = true;
}

static rage::fiFile*(*rage__fiFile__Open)(const char* fileName, rage::fiDevice* device, bool readOnly);

rage::fiFile* rage__fiFile__OpenWrap(const char* fileName, rage::fiDevice* device, bool readOnly)
{
	// force this to be a *real* fiDeviceLocal
	auto localDevice = rage::fiDevice::GetDevice(fileName, true);

	// by replacing the vtable.
	*(uintptr_t*)device = *(uintptr_t*)localDevice;
	
	return rage__fiFile__Open(fileName, device, true);
}

namespace streaming
{
	StreamingPackfileEntry* GetStreamingPackfileForEntry(StreamingDataEntry* entry)
	{
		auto handle = entry->handle;
		if ((handle & 0xC0000000) == 0x80000000)
		{
			return nullptr;
		}
		else
		{
			return &g_streamingPackfiles->Get(handle >> 16);
		}

		return nullptr;
	}
}

#include <ICoreGameInit.h>
#include <GameInit.h>

static HookFunction hookFunction([] ()
{
	assert(offsetof(CollectionData, name) == 120);
	assert(offsetof(StreamingPackfileEntry, enabled) == 68);

	ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();
	
	OnKillNetworkDone.Connect([] ()
	{
		// clear blacklisted files
		g_registeredFileSet.clear();

		// clear custom files
		g_customStreamingFiles.clear();
		g_customStreamingFileSet.clear();

		// clear new custom files
		g_customStreamingFilesByTag.clear();
		g_customStreamingFileRefs.clear();

		// clear state
		for (auto& collection : g_cfxCollections)
		{
			((CfxCollection*)collection)->Invalidate();
		}
	}, -500);

	{
		void* location = hook::pattern("41 B0 01 BA 1B E6 DA 93 E8").count(1).get(0).get<void>(-12);

		hook::set_call(&g_origOpenPackfiles, location);
		hook::call(location, OpenPackfilesWrap);
	}

	{
		void* location = hook::pattern("48 8B FA 89 44 24 30 48 8B D9 E8 ? ? ? ? 0F").count(1).get(0).get<void>(10);

		hook::set_call(&g_origAddCollision, location);
		hook::call(location, AddCollisionWrap);
	}

	// packfile create
	hook::call(hook::pattern("4C 8B F0 49 8B 06 49 8B CE FF 90 60 01 00 00 48").count(1).get(0).get<void>(-5), ConstructPackfile);

	// dlc packfile create
	hook::call(hook::pattern("48 8B C8 E8 ? ? ? ? 48 8B E8 EB 03 49 8B EF 48 89").count(1).get(0).get<void>(3), ConstructPackfile);

	// dlc packfile open
	hook::call(hook::pattern("48 8B C8 E8 ? ? ? ? 48 8B E8 EB 03 49 8B EF 48 89").count(1).get(0).get<void>(3 + 52), hook::get_member(&CfxCollection::OpenPackfile));

	// local packfile mount
	hook::call(hook::pattern("48 8B CF E8 ? ? ? ? 8B 53 10 B9 01 00 00 00").count(1).get(0).get<void>(3), hook::get_member(&CfxCollection::Mount));

	// streaming packfile open
	hook::call(hook::pattern("44 8A 44 3B 49 41 B9 02 00 00 00 41 D0 E8").count(1).get(0).get<void>(0x1D), hook::get_member(&CfxCollection::OpenPackfile));

	// same
	hook::call(hook::pattern("49 8B CC 48 89 7C 24 20 45 1B C9 41 83 E1 03 E8").count(1).get(0).get<void>(15), hook::get_member(&CfxCollection::OpenPackfile));

	// streaming packfile load-from-meta
	void* cacheLoadAddr = hook::get_pattern("45 84 ED 0F 85 ? 01 00 00 49 8B CC E8", 12);
	hook::set_call(&g_origLoadFromCache, cacheLoadAddr);
	hook::call(cacheLoadAddr, &CfxCollection::LoadFromCache);

	hook::set_call(&rage__fiFile__Read, (char*)g_origLoadFromCache + 0x34);

	// save-to-meta
	void* cacheSaveAddr = hook::get_pattern("75 7E 49 8B CC E8 ? ? ? ? 8B CF 89", 5);
	hook::set_call(&g_origSaveToCache, cacheSaveAddr);
	hook::call(cacheSaveAddr, &CfxCollection::SaveToCache);

	hook::set_call(&rage__fiFile__Write, (char*)g_origSaveToCache + 0x2F);

	// similar for a weird case
	void* internal = hook::pattern("4C 8B CE 41 D0 E8 48 8B D0 48 8B CD 41 80 E0 01").count(1).get(0).get<void>(16);

	hook::set_call(&g_origOpenPackfileInternal, internal);
	hook::call(internal, hook::get_member(&CfxCollection::OpenPackfileInternal));

	// streaming module
	char* location = hook::pattern("48 63 D9 A8 01 75 23 83 C8 01").count(1).get(0).get<char>(0xD);

	g_streamingModule = (rage::strStreamingModule*)(location + *(int32_t*)location + 4);

	// streaming entries
	location = hook::pattern("84 C0 74 3C 8B 53 08 48 8D 0D").count(1).get(0).get<char>(10);

	g_streamingEntries = (StreamingEntry**)(location + *(int32_t*)location + 4);

	// streaming packfile entries
	location = hook::pattern("48 8B 05 ? ? ? ? 48 8B CB 48 6B C9 68 80 64").count(1).get(0).get<char>(3);

	g_streamingPackfiles = (decltype(g_streamingPackfiles))(location + *(int32_t*)location + 4);

	// collection cleaning
	void* closeCollection = hook::pattern("EB 05 E8 ? ? ? ? 48 8B 53 30 C6").count(1).get(0).get<void>(2);

	hook::set_call(&g_origCloseCollection, closeCollection);
	hook::call(closeCollection, DoCloseCollection);

	// collection root
	location = hook::pattern("4C 8D 0D ? ? ? ? 48 89 01 4C 89 81 80 00 00").count(1).get(0).get<char>(3);

	g_collectionRoot = (rage::fiCollection**)(location + *(int32_t*)location + 4);

	//
	uint32_t* result = hook::pattern("44 89 41 28 4C 89 41 38 4C 89 41 50 48 8D 05").count(1).get(0).get<uint32_t>(15);

	uintptr_t endOffset = ((uintptr_t)result) + 4;

	g_vTable_fiPackfile = endOffset + *result;

	// initial mount
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string pseudoPackPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\pseudopack"));

		// TODO: replace with an empty packfile once suitable
		rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
		//device->SetPath(pseudoPackPath.c_str(), true);
		device->SetPath("citizen:/pseudopack/", true);
		device->Mount("pseudoPack:/");
	}, 50);

	// resource ptr resolve error
	hook::call(hook::pattern("B9 3D 27 92 83 E8").count(1).get(0).get<void>(5), PtrError);

#ifdef DRAWABLE_DBG
	// 
	location = hook::pattern("33 FF 48 89 03 48 8B 93 B0 00 00 00 48 85 D2").count(1).get(0).get<char>(-4);

	void** vt = (void**)(location + *(int32_t*)location + 4);

	g_origDrawDrawableR = ((decltype(g_origDrawDrawableR))vt[4]);
	vt[4] = CustomDrawDrawableR;

	g_origDrawDrawableR2 = ((decltype(g_origDrawDrawableR2))vt[5]);
	vt[5] = CustomDrawDrawableR2;

	g_origDrawDrawable = ((decltype(g_origDrawDrawable))vt[6]);
	vt[6] = CustomDrawDrawable;

	g_origDrawDrawable2 = ((decltype(g_origDrawDrawable2))vt[7]);
	vt[7] = CustomDrawDrawable2;

	char* call = (char*)hook::get_call(hook::pattern("FF 50 10 48 89 06 85 FF 7E 18 33 DB 48 8B 0E 48").count(3).get(0).get<void>(0x5F - 0x4B));
	call += 0x85;

	char* intn = (char*)hook::get_call(call);
	assert(intn[32] == 3);
	
	hook::set_call(&g_origResolveQB, call);
	hook::call(call, ResolveQB);

	// pointer resolution
	hook::set_call(&g_resolvePtr, hook::pattern("48 8B D9 48 89 01 48 83 C1 20 E8 ? ? ? ? 48 8D 4B 30").count(1).get(0).get<void>(10));
#endif

	// boundbvh -> boundgeometry :d
	hook::set_call(&g_origGeomThing, hook::pattern("EB 4E 48 8B D1 48 8B CB E8").count(1).get(0).get<void>(8));

	hook::call(hook::pattern("EB 4E 48 8B D1 48 8B CB E8").count(1).get(0).get<void>(8), DoGeomThing);

	// somehow this doesn't set a6 for the func; wrap it
	void* bvhFunc = hook::pattern("48 89 4C 24 20 48 8B 8F 30 01 00 00 49 8B D5").count(1).get(0).get<void>(15);

	hook::set_call(&g_origBvhSet, bvhFunc);
	hook::call(bvhFunc, BvhSet);

	// don't even think about thinking (packfile is hdd flag)
	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			xor(r14d, r14d);
			mov(word_ptr[rbx + 0x60], r14w);

			ret();
		}
	} doHddThing;

	void* hddAddr = hook::pattern("66 89 7B 60 45 33 F6").count(1).get(0).get<void>(0);
	//hook::nop(hddAddr, 7);
	//hook::call(hddAddr, doHddThing.GetCode());

	// streaming file registration - disable DLC override capability if the file is also existent in global streaming
	g_registerStreamingFile = hook::pattern("48 8B D8 41 8B C6 25 00 00 00 C0 3D").count(1).get(0).get<char>(-0x42);

	hook::call(g_registerStreamingFile + 0x3A, RegisterStreamingFileStrchrWrap);

	// (not temp dbg: )InvalidFile overwrite fuckery
	hook::nop(hook::get_pattern("33 D2 E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8D 15", -7), 58);

	// reloading dependency: flag streaming packfiles from DLC changesets to not automatically load
	// RELOADING ATTEMPT NOP
#if 0
	{
		void* loc = hook::get_pattern("45 33 C0 8B D3 E8 ? ? ? ? 8B CB E8", 12);
		hook::set_call(&g_origRemoveStreamingPackfile, loc);
		hook::call(loc, RemoveStreamingPackfileWrap);
	}
#endif

	// make the pfm.dat read-only
	{
		auto loc = hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? 00 00 44 39 35", 70);
		hook::set_call(&rage__fiFile__Open, loc);
		hook::call(loc, rage__fiFile__OpenWrap);
	}
});