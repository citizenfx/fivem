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

#pragma comment(lib, "shlwapi.lib")
#include <shlwapi.h>

static rage::fiCollection** g_collectionRoot;

static void(*g_origCloseCollection)(rage::fiCollection*);
static void(*g_origOpenPackfileInternal)(rage::fiCollection*, const char* archive, bool bTrue, bool bFalse, intptr_t veryFalse);

static uintptr_t g_vTable_fiPackfile;

static hook::thiscall_stub<rage::fiCollection*(rage::fiCollection*)> packfileCtor([] ()
{
	return hook::pattern("44 89 41 28 4C 89 41 38 4C 89 41 50 48 8D").count(1).get(0).get<void>(-0x1E);
});

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

private:
	// to be filled with packfile data
	char m_pad[184];

	char m_childPackfile[192];

	//rage::fiCollection* m_parentCollection;

	concurrency::concurrent_unordered_map<uint16_t, CollectionEntry> m_entries;

	concurrency::concurrent_unordered_map<std::string, uint16_t> m_reverseEntries;

	HandleEntry m_handles[64];

	bool m_hasCleaned;

	std::recursive_mutex m_mutex;
	
public:
	typedef std::function<bool(const char*, std::string&)> TLookupFn;

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
			m_oldVal = *(reinterpret_cast<char*>(this) + 0xA8);

			m_collection->m_mutex.lock();
			memcpy(collection->m_childPackfile, collection, sizeof(collection->m_childPackfile));
			*(uintptr_t*)collection->m_childPackfile = g_vTable_fiPackfile;

			m_child = reinterpret_cast<rage::fiCollection*>(collection->m_childPackfile);
		}

		~PseudoCallContext()
		{
			memcpy(reinterpret_cast<char*>(m_collection) + 8, &m_collection->m_childPackfile[8], sizeof(m_collection->m_childPackfile) - 8);
			m_collection->m_mutex.unlock();

			bool newVal = *(reinterpret_cast<char*>(this) + 0xA8);
			if (newVal != m_oldVal)
			{
				trace("flag changed from %d to %d.\n", m_oldVal, newVal);
			}
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
	CfxCollection()
	{
		uintptr_t ourVt = *(uintptr_t*)this;
		packfileCtor(this);
		*(uintptr_t*)this = ourVt;

		memset(m_handles, 0, sizeof(m_handles));

		m_lookupFunction = [] (const char*, std::string&)
		{
			return false;
		};

		m_hasCleaned = false;
	}

	void Reset()
	{
		memset(m_handles, 0, sizeof(m_handles));

		m_reverseEntries.clear();
		m_entries.clear();

		m_lookupFunction = [] (const char*, std::string&)
		{
			return false;
		};
	}

	void CleanCloseCollection()
	{
		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		m_entries.clear();
		m_reverseEntries.clear();

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

		m_handles[0].parentDevice = this;

		for (int i = 0; i < _countof(m_handles); i++)
		{
			if (m_handles[i].parentDevice == nullptr)
			{
				m_handles[i].parentDevice = parentDevice;
				m_handles[i].parentHandle = parentHandle;
				m_handles[i].name = name;

				return i;
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
			//trace("open: %s\n", GetEntryName(index));

			return AllocateHandle(this, PseudoCallContext(this)->OpenCollectionEntry(index, ptr), "");// GetEntryName(index));
		}

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

		assert(!__FUNCTION__);
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

	virtual bool UnkA()
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual bool CloseBasePackfile()
	{
		return PseudoCallContext(this)->CloseBasePackfile();
	}

	virtual bool UnkC()
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual bool UnkD()
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual bool UnkE()
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual bool UnkF()
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual uint64_t Open(const char* fileName, bool readOnly)
	{
		trace("open %s\n", fileName);

		return PseudoCallContext(this)->Open(fileName, readOnly);
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr)
	{
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

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead)
	{
		return PseudoCallContext(this)->Read(handle, buffer, toRead);
	}

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
	{
		//trace("ReadBulk %s\n", m_handles[handle].name);

		if (m_handles[handle].parentDevice == this)
		{
			return PseudoCallContext(this)->ReadBulk(m_handles[handle].parentHandle, ptr, buffer, toRead);
		}
		else
		{
			return m_handles[handle].parentDevice->ReadBulk(m_handles[handle].parentHandle, ptr, buffer, toRead);
		}

		//return PseudoCallContext(this)->ReadBulk(handle, ptr, buffer, toRead);
	}

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int)
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual uint32_t Write(uint64_t, void*, int)
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method)
	{
		return PseudoCallContext(this)->Seek(m_handles[handle].parentHandle, distance, method);
	}

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method)
	{
		return PseudoCallContext(this)->SeekLong(m_handles[handle].parentHandle, distance, method);
	}

	virtual int32_t Close(uint64_t handle)
	{
		return PseudoCallContext(this)->Close(handle);
	}

	virtual int32_t CloseBulk(uint64_t handle)
	{
		//trace("CloseBulk %s\n", m_handles[handle].name);

		int32_t retval;

		if (m_handles[handle].parentDevice == this)
		{
			retval = PseudoCallContext(this)->CloseBulk(m_handles[handle].parentHandle);
		}
		else
		{
			retval = m_handles[handle].parentDevice->CloseBulk(m_handles[handle].parentHandle);
		}

		m_handles[handle].parentDevice = nullptr;

		return retval;
		//PseudoCallContext(this)->CloseBulk(handle);
	}

	virtual int GetFileLength(uint64_t handle)
	{
		return PseudoCallContext(this)->GetFileLength(handle);
	}

	virtual uint64_t GetFileLengthUInt64(uint64_t handle)
	{
		return PseudoCallContext(this)->GetFileLengthUInt64(handle);
	}

	// dummy!
	virtual int m_40(int)
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual bool RemoveFile(const char* file)
	{
		assert(!__FUNCTION__);

		return 0;
	}
	virtual int RenameFile(const char* from, const char* to)
	{
		assert(!__FUNCTION__);

		return 0;
	}
	virtual int CreateDirectory(const char* dir)
	{
		assert(!__FUNCTION__);

		return 0;
	}

	virtual int RemoveDirectory(const char * dir)
	{
		assert(!__FUNCTION__);

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
		assert(!__FUNCTION__);

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

	virtual void m_xy()
	{
		assert(!__FUNCTION__);
	}

	virtual bool Truncate(uint64_t handle)
	{
		assert(!__FUNCTION__);
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
		return PseudoCallContext(this)->ReadFull(handle, buffer, length);
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

	virtual int32_t m_yz()
	{
		return PseudoCallContext(this)->m_yz();
	}

	virtual int32_t m_zx() // return 0x40000000
	{
		return PseudoCallContext(this)->m_zx();
	}

	virtual bool m_zy()
	{
		return PseudoCallContext(this)->m_zy();
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
		const char* colon = strchr(archive, ':');

		std::stringstream basePath;

		// temporary: make citizen/ path manually
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		basePath << converter.to_bytes(MakeRelativeCitPath(L"citizen/"));

		basePath << std::string(archive, colon);
		basePath << "/";
		basePath << std::string(&colon[1], StrStrIA(colon, ".rpf"));

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
				struct IgnoreCaseLess
				{
					inline bool operator()(const std::string& left, const std::string& right)
					{
						return _stricmp(left.c_str(), right.c_str()) < 0;
					}
				};

				// read the list
				std::set<std::string, IgnoreCaseLess> fileList;

				do 
				{
					fileList.insert(findData.fileName);
				} while (device->FindNext(findHandle, &findData));

				device->FindClose(findHandle);

				// create a lookup function
				m_lookupFunction = [=] (const char* filename, std::string& newFilename)
				{
					auto it = fileList.find(filename);

					if (it != fileList.end())
					{
						newFilename = str + "/" + *it;

						return true;
					}

					return false;
				};
			}
		}
	}

public:

	// functions
	bool OpenPackfile(const char* archive, bool bTrue, bool bFalse, int type, intptr_t veryFalse)
	{
		DetermineLookupFunction(archive);

		rage::fiDevice* baseDevice = rage::fiDevice::GetDevice(archive, true);

		// weird workaround for fiDeviceRelative not passing this through
		if (!baseDevice->m_zy())
		{
			archive = va("update:/x64/%s", strstr(archive, ":/") + 2);
		}

		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		{
			PseudoCallContext ctx(this);
			reinterpret_cast<rage::fiPackfile*>(ctx.GetPointer())->OpenPackfile(archive, bTrue, bFalse, type, veryFalse);
		}

		return true;
	}

	bool OpenPackfileInternal(const char* archive, bool bTrue, bool bFalse, intptr_t veryFalse)
	{
		DetermineLookupFunction(archive);

		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		rage::fiDevice* unk = *(rage::fiDevice**)((reinterpret_cast<char*>(this) + 80));

		if (unk && dynamic_cast<CfxCollection*>(unk))
		{
			FatalError("Tried to open a CfxCollection inside a CfxCollection.");
		}

		{
			PseudoCallContext ctx(this);
			g_origOpenPackfileInternal(ctx.GetPointer(), archive, bTrue, bFalse, veryFalse);
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
	//rage::fiCollection* collection = packfileCtor(packfile);
	rage::fiCollection* cfxCollection = new CfxCollection();

	g_collectionRoot[cfxCollection->GetCollectionId()] = cfxCollection;

	return cfxCollection;
}

void DoCloseCollection(rage::fiCollection* collection)
{
	auto cfxCollection = dynamic_cast<CfxCollection*>(collection);

	trace("clean close collection\n");

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

const char* ptrError = "Invalid pointer value found while resolving a pg* resource (attach a debugger for more info)";

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

static HookFunction hookFunction([] ()
{
	return;

	// packfile create
	hook::call(hook::pattern("4C 8B F0 49 8B 06 49 8B CE FF 90 60 01 00 00 48").count(1).get(0).get<void>(-5), ConstructPackfile);

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

	// collection cleaning
	void* closeCollection = hook::pattern("EB 05 E8 ? ? ? ? 48 8B 53 30 C6").count(1).get(0).get<void>(2);

	hook::set_call(&g_origCloseCollection, closeCollection);
	hook::call(closeCollection, DoCloseCollection);

	// collection root
	char* location = hook::pattern("4C 8D 0D ? ? ? ? 48 89 01 4C 89 81 80 00 00").count(1).get(0).get<char>(3);

	g_collectionRoot = (rage::fiCollection**)(location + *(int32_t*)location + 4);

	//
	uint32_t* result = hook::pattern("44 89 41 28 4C 89 41 38 4C 89 41 50 48 8D 05").count(1).get(0).get<uint32_t>(15);

	uintptr_t endOffset = ((uintptr_t)result) + 4;

	g_vTable_fiPackfile = endOffset + *result;

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
});