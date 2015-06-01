/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <fiCollectionWrapper.h>

#include <Hooking.h>

#include <concurrent_unordered_map.h>

#include <mutex>

#include <sstream>

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>

#pragma comment(lib, "shlwapi.lib")
#include <shlwapi.h>

static rage::fiCollection** g_collectionRoot;

static void(*g_origCloseCollection)(rage::fiCollection*);
static void(*g_origOpenPackfileInternal)(rage::fiCollection*, const char* archive, bool bTrue, intptr_t veryFalse);

static uintptr_t g_vTable_fiPackfile;

static hook::thiscall_stub<rage::fiCollection*(rage::fiCollection*)> packfileCtor([] ()
{
	return hook::pattern("44 89 41 28 4C 89 41 38 4C 89 41 50 48 8D").count(1).get(0).get<void>(-0x1E);
});

#define GET_HANDLE(x) ((x) & 0x7FFFFFFF)
#define UNDEF_ASSERT() FatalError("Undefined function " __FUNCTION__)

struct StreamingPackfileEntry
{
	FILETIME modificationTime;
	uint8_t pad[80];
	uint32_t parentIdentifier;
	uint8_t pad2[12];
};

#include <atArray.h>

static atArray<StreamingPackfileEntry>* g_streamingPackfiles;

class CfxCollection : public rage::fiCollection
{
private:
	struct CollectionEntry
	{
		FileEntry baseEntry;

		const FileEntry* origEntry;
		std::string fileName;
	};

	struct HandleEntry
	{
		rage::fiDevice* parentDevice;
		int64_t parentHandle;

		const char* name;

		CollectionEntry* ourEntry;
	};

	struct IgnoreCaseLess
	{
		inline bool operator()(const std::string& left, const std::string& right) const
		{
			return _stricmp(left.c_str(), right.c_str()) < 0;
		}
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
	}

	~CfxCollection()
	{
		trace("del cfxcollection %p\n", this);

		PseudoCallContext(this)->~fiCollection();

		rage::GetAllocator()->free(m_parentCollection);
	}

	void CleanCloseCollection()
	{
		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		m_entries.clear();
		m_reverseEntries.clear();

		m_isPseudoPack = false;

		memset(m_handles, 0, sizeof(m_handles));

		m_hasCleaned = true;

		trace("cleanup %s\n", GetName());
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
			FatalError("CfxCollection[%s]::AllocateHandle got passed a failed handle for %s", GetName(), name);
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

	void AddEntry(uint16_t idx, const std::string& name)
	{
		CollectionEntry newEntry = { 0 };
		newEntry.fileName = name;
		
		rage::ResourceFlags flags;
		rage::fiDevice* baseDevice = rage::fiDevice::GetDevice(name.c_str(), true);
		baseDevice->GetResourceVersion(name.c_str(), &flags);

		memcpy(&newEntry.baseEntry, PseudoCallContext(this)->GetEntry(idx), sizeof(newEntry.baseEntry));

		newEntry.baseEntry.size = baseDevice->GetFileLengthLong(name.c_str());

		newEntry.baseEntry.virtFlags = flags.flag1;
		newEntry.baseEntry.physFlags = flags.flag2;

		m_reverseEntries[name] = idx;
		m_entries[idx] = newEntry;
	}

	void AddEntry(uint16_t idx, const FileEntry* entry)
	{
		CollectionEntry newEntry;
		newEntry.origEntry = entry;
		memcpy(&newEntry.baseEntry, entry, sizeof(FileEntry));

		m_entries[idx] = newEntry;
	}

	CollectionEntry* GetCfxEntry(uint16_t index)
	{
		auto it = m_entries.find(index);

		if (it == m_entries.end())
		{
			char entryName[256] = { 0x1E };
			PseudoCallContext(this)->GetEntryNameToBuffer(index, entryName, sizeof(entryName));

			//trace("init entry: %s -> %s\n", m_parentCollection->GetName(), entryName);

			//assert(isalpha(entryName[0]) || isdigit(entryName[0]) || entryName[0] == '_');

			std::string newName;
			if (m_lookupFunction(entryName, newName))
			{
				AddEntry(index, newName);
			}
			else
			{
				m_reverseEntries[entryName] = index;
				AddEntry(index, PseudoCallContext(this)->GetEntry(index));

				if (entryName[0] != 0x1E)
				{
					m_entries[index].fileName = entryName;
				}
			}
		}

		return &m_entries[index];
	}

	virtual int64_t OpenCollectionEntry(uint16_t index, uint64_t* ptr) override
	{
		//return PseudoCallContext(this)->OpenCollectionEntry(index, ptr);

		auto entry = GetCfxEntry(index);

		if (entry->origEntry)
		{
			char entryName[256] = { 0 };

			PseudoCallContext(this)->GetEntryNameToBuffer(index, entryName, 255);

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

		return (entry->origEntry) ? entry->origEntry : &entry->baseEntry;

		//return PseudoCallContext(this)->GetEntry(index);
	}

	virtual const char* GetEntryName(uint16_t index) override
	{
		auto entry = GetCfxEntry(index);

		return (entry->origEntry) ? PseudoCallContext(this)->GetEntryName(index) : entry->fileName.c_str();

		//return PseudoCallContext(this)->GetEntryName(index);
	}

	virtual int64_t Unk1(uint16_t index, bool flag)
	{
		auto entry = GetCfxEntry(index);

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

		if (entry->origEntry)
		{
			PseudoCallContext(this)->GetEntryNameToBuffer(index, buffer, maxLen);
		}
		else
		{
			PseudoCallContext(this)->GetEntryNameToBuffer(index, buffer, maxLen);
			//strcpy_s(buffer, maxLen, entry->fileName.c_str());
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

			return AllocateHandle(device, handle, "");
		}

		return AllocateHandle(this, PseudoCallContext(this)->Open(fileName, readOnly), "");
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr)
	{
		std::string outFileName;

		if (m_lookupFunction(fileName, outFileName))
		{
			rage::fiDevice* device = rage::fiDevice::GetDevice(outFileName.c_str(), true);
			uint64_t handle = device->OpenBulk(outFileName.c_str(), ptr);

			return AllocateHandle(device, handle, "");
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
		UNDEF_ASSERT();

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
	void DetermineLookupFunction(const char* archive)
	{
		// build a local path string to verify
		std::stringstream basePath;

		if (!IsPseudoPackPath(archive) || _strnicmp(archive, "pseudoPack:/", 12) == 0)
		{
			const char* colon = strchr(archive, ':');

			// temporary: make citizen/ path manually
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			basePath << converter.to_bytes(MakeRelativeCitPath(L"citizen/"));

			basePath << std::string(archive, colon);
			basePath << "/";
			basePath << std::string(&colon[1], const_cast<const char*>(StrStrIA(colon, ".rpf")));
		}
		else
		{
			basePath << std::string(archive, const_cast<const char*>(StrStrIA(archive, ".rpf")));
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
				if (!fileList.empty())
				{
					SYSTEMTIME systemTime;
					GetSystemTime(&systemTime);

					FILETIME fileTime;
					SystemTimeToFileTime(&systemTime, &fileTime);

					g_streamingPackfiles->Get(GetCollectionId()).modificationTime = fileTime;

					m_streamingFileList = fileList;
				}
			}
		}
	}

public:

	// functions
	bool OpenPackfile(const char* archive, bool bTrue, int type, intptr_t veryFalse);

	bool OpenPackfileInternal(const char* archive, bool bTrue, intptr_t veryFalse)
	{
		DetermineLookupFunction(archive);

		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		rage::fiDevice* unk = *(rage::fiDevice**)((reinterpret_cast<char*>(this) + 80));

		if (unk && dynamic_cast<CfxCollection*>(unk))
		{
			trace("Tried to open a CfxCollection inside a CfxCollection. Is this bad?\n");
		}

		if (!IsPseudoPackPath(archive))
		{
			if (IsPseudoPack())
			{
				__debugbreak();
			}

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

template<const char** errorString>
static void DbgError()
{
	if (CoreIsDebuggerPresent())
	{
		__debugbreak();
	}

	FatalError(*errorString);
}

const char* ptrError = "Invalid fixup, address is neither virtual nor physical (paged resources - attach a debugger to find where)";

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

bool CfxCollection::OpenPackfile(const char* archive, bool bTrue, int type, intptr_t veryFalse)
{
	DetermineLookupFunction(archive);

	rage::fiDevice* baseDevice = nullptr;

	if (!IsPseudoPackPath(archive))
	{
		baseDevice = rage::fiDevice::GetDevice(archive, true);
	}

	// weird workaround for fiDeviceRelative not passing this through
	bool isProxiedRelative = false;

	if (baseDevice && !baseDevice->IsBulkDevice() && !strstr(archive, "dlcpacks:"))
	{
		const char* mount = strstr(archive, ":/");

		if (!mount)
		{
			mount = strstr(archive, ":\\");
		}

		archive = va("update:/x64/%s", mount + 2);

		isProxiedRelative = true;
	}

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
	return strstr(path, "pseudo") != nullptr;
}

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

#include <numeric>
#include <sysAllocator.h>

void CfxCollection::InitializePseudoPack(const char* path)
{
	m_isPseudoPack = true;

	trace("init pseudopack %s\n", path);

	assert(m_streamingFileList.size() > 0);

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
			std::string entryFullName;
			m_lookupFunction(entry.c_str(), entryFullName);

			auto entryDevice = rage::fiDevice::GetDevice(entryFullName.c_str(), true);

			rage::ResourceFlags flags;
			entryDevice->GetResourceVersion(entryFullName.c_str(), &flags);

			entryTable[i].nameOffset = addString(entry);
			entryTable[i].offset = 0x8FFFFF;
			entryTable[i].size = entryDevice->GetFileLengthLong(entryFullName.c_str());
			entryTable[i].virtFlags = flags.flag1;
			entryTable[i].physFlags = flags.flag2;

			parentDirectoryTable[i] = 0;

			i++;
		}

		entryCount = i;
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

	strcpy_s(collectionData->smallName, "gta_ny.rpf");

	collectionData->entryTableAllocSize = sizeof(rage::fiCollection::FileEntry) * (m_streamingFileList.size() + 1);
	collectionData->keyId = 0;

	static_assert(sizeof(CollectionData) == 176, "m");
}

static HookFunction hookFunction([] ()
{
	assert(offsetof(CollectionData, name) == 120);

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
	hook::call(hook::pattern("B9 3D 27 92 83 E8").count(1).get(0).get<void>(5), DbgError<&ptrError>);

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

	// break the game! phBoundPolyhedron + 184
	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			mov(byte_ptr[rbx + 130], 0);
			mov(qword_ptr[rbx + 0xB8], 0);
			xor(rax, rax);

			ret();
		}
	} blahHook;

	hook::call(hook::pattern("48 01 83 B0 00 00 00 48 8B 93 B8 00 00 00 48 85").count(1).get(0).get<void>(0x8A - 0x74), blahHook.GetCode());
});