#include <StdInc.h>
#include <Hooking.h>

#include <sysAllocator.h>

#include <LaunchMode.h>

static int ReturnTrue()
{
	return 1;
}

struct IndexStore
{
	struct Entry
	{
		uint32_t hash;
		uint32_t index;
		uint32_t next;
	};

	Entry* entries;
	int* indices;
	uint32_t count;
	uint32_t nextFree;

	IndexStore()
	{
		entries = nullptr;
		indices = nullptr;
		count = 0;
	}

	void Initialize(int targetCount)
	{
		this->count = targetCount * 2;
		this->indices = (int*)rage::GetAllocator()->allocate(sizeof(*indices) * this->count, 16, 0);
		this->entries = (Entry*)rage::GetAllocator()->allocate(sizeof(*entries) * this->count, 16, 0);

		memset(this->indices, 0xFF, sizeof(*indices) * this->count);
		memset(this->entries, 0, sizeof(*entries) * this->count);

		this->nextFree = 0;

		// initialize the next list
		for (int i = 0; i < targetCount - 1; i++)
		{
			this->entries[i].next = i + 1;
		}

		this->entries[targetCount - 1].next = -1;
	}

	void Insert(uint32_t hash, int index)
	{
		int freeIdx = this->nextFree;
		int start = (hash % this->count);

		auto& entry = this->entries[freeIdx];
		
		auto oldNext = entry.next;
		entry.hash = hash;
		entry.index = index;

		entry.next = indices[start];
		indices[start] = freeIdx;

		this->nextFree = oldNext;
	}

	void Remove(uint32_t hash)
	{
		int start = (hash % this->count);
		auto idx = this->indices[start];

		if (idx != -1)
		{
			while (this->entries[idx].hash != hash)
			{
				idx = this->entries[idx].next;

				if (idx == -1)
				{
					return;
				}
			}

			auto& entry = this->entries[idx];
			this->indices[start] = entry.next;

			entry.hash = 0;
			entry.index = 0;
			entry.next = this->nextFree;

			this->nextFree = idx;
		}
	}

	int GetIndex(uint32_t hash)
	{
		int start = (hash % this->count);
		auto idx = this->indices[start];

		if (idx == -1)
		{
			return -1;
		}

		while (this->entries[idx].hash != hash)
		{
			idx = this->entries[idx].next;

			if (idx == -1)
			{
				return -1;
			}
		}

		return this->entries[idx].index;
	}

	void Clear()
	{
		this->nextFree = -1;

		rage::GetAllocator()->free(this->entries);
		rage::GetAllocator()->free(this->indices);

		this->entries = nullptr;
		this->indices = nullptr;
	}
};

#include <atPool.h>

struct AssetStore
{
	char m_pad[56];
	atPoolBase pool;
	char m_pad2[32];
	IndexStore store;

	int* GetIndexByKey(int* index, uint32_t key)
	{
		int idx = store.GetIndex(key);

		if (idx != -1)
		{
			auto ptr = pool.GetAt<void>(idx);

			if (!ptr)
			{
				idx = -1;
			}
		}

		*index = idx;

		return index;
	}
};

#include <MinHook.h>

static char* g_drawableStore;
static char* g_dwdStore;

inline bool IsInt32Store(IndexStore* store)
{
	return (store == (IndexStore*)(g_drawableStore + 112) ||
		store == (IndexStore*)(g_dwdStore + 112));
}

inline bool IsInt32Store(AssetStore* store)
{
	return (store == (AssetStore*)g_drawableStore ||
		store == (AssetStore*)g_dwdStore);
}

static int*(*g_origGetIndexByKey)(AssetStore* store, int* index, uint32_t* key);

static int* GetIndexByKeyStub(AssetStore* store, int* index, uint32_t* key)
{
	if (IsInt32Store(store))
	{
		return store->GetIndexByKey(index, *key);
	}

	return g_origGetIndexByKey(store, index, key);
}

static void(*g_origStoreRemove)(IndexStore* store, uint32_t hash);

static void StoreRemove(IndexStore* store, uint32_t hash)
{
	if (IsInt32Store(store))
	{
		return store->Remove(hash);
	}

	return g_origStoreRemove(store, hash);
}

static void(*g_origStoreInsert)(IndexStore* store, uint32_t hash, int index);

static void StoreInsert(IndexStore* store, uint32_t hash, int index)
{
	if (IsInt32Store(store))
	{
		return store->Insert(hash, index);
	}

	return g_origStoreInsert(store, hash, index);
}

static void(*g_origStoreInit)(IndexStore* store, int count);

static void StoreInit(IndexStore* store, int count)
{
	if (IsInt32Store(store))
	{
		return store->Initialize(count);
	}

	return g_origStoreInit(store, count);
}

static bool(*g_origSetDrawable)(char* archetype, uint32_t* hash, int txd, char type);

static int g_drawableIndices[65536];

static bool ArchetypeSetDrawable(char* archetype, uint32_t* hash, int txd, char type)
{
	if (type != 0)
	{
		return g_origSetDrawable(archetype, hash, txd, type);
	}

	*(uint8_t*)(archetype + 96) = 0;
	*(uint16_t*)(archetype + 98) = -1;

	uint16_t archetypeIdx = *(uint16_t*)(archetype + 102);
	g_drawableIndices[archetypeIdx] = -1;

	if (*hash != HashRageString("null"))
	{
		auto drbStore = (AssetStore*)g_drawableStore;
		int idx = drbStore->store.GetIndex(*hash);

		if (idx != -1)
		{
			auto ptr = drbStore->pool.GetAt<char>(idx);

			if (ptr)
			{
				*(uint8_t*)(archetype + 96) = 2;
				*(uint16_t*)(archetype + 98) = idx;
				g_drawableIndices[archetypeIdx] = idx;

				if (txd != -1)
				{
					*(uint16_t*)(ptr + 24) = txd;
				}

				return true;
			}
		}
	}

	return g_origSetDrawable(archetype, hash, txd, true);
}

static bool ArchetypeSetDrawableDict(char* archetype, uint32_t* hash, int txd)
{
	*(uint8_t*)(archetype + 96) = 0;
	*(uint16_t*)(archetype + 98) = -1;

	uint16_t archetypeIdx = *(uint16_t*)(archetype + 102);
	g_drawableIndices[archetypeIdx] = -1;

	if (*hash != HashRageString("null"))
	{
		auto dwdStore = (AssetStore*)g_dwdStore;
		int idx = dwdStore->store.GetIndex(*hash);

		if (idx != -1)
		{
			auto ptr = dwdStore->pool.GetAt<char>(idx);

			if (ptr)
			{
				*(uint8_t*)(archetype + 96) = 3;
				*(uint16_t*)(archetype + 98) = idx;
				g_drawableIndices[archetypeIdx] = idx;

				if (txd != -1)
				{
					*(uint16_t*)(ptr + 16) = txd;
				}

				return true;
			}
		}
	}

	return true;
}

static void*(*g_origGetDrawable)(void* archetype);

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#define RAGE_FORMATS_IN_GAME

#include <gtaDrawable.h>

static void* ArchetypeGetDrawable(char* archetype)
{
	if (!(*(uint8_t*)(archetype + 80) & 1))
	{
		return nullptr;
	}

	if (*(uint8_t*)(archetype + 96) == 2)
	{
		auto drbStore = (AssetStore*)g_drawableStore;

		uint16_t archetypeIdx = *(uint16_t*)(archetype + 102);
		auto entry = drbStore->pool.GetAt<char>(g_drawableIndices[archetypeIdx]);

		if (entry)
		{
			return *(void**)entry;
		}

		return nullptr;
	}
	else if (*(uint8_t*)(archetype + 96) == 3)
	{
		auto dwdStore = (AssetStore*)g_dwdStore;

		uint16_t archetypeIdx = *(uint16_t*)(archetype + 102);
		auto entry = dwdStore->pool.GetAt<char>(g_drawableIndices[archetypeIdx]);

		if (entry)
		{
			auto dict = *(rage::five::pgDictionary<rage::five::gtaDrawable>**)entry;
			return dict->GetAt(*(uint8_t*)(archetype + 97));
		}

		return nullptr;
	}

	return g_origGetDrawable(archetype);
}

static int(*g_origGetDrawableF18)(void* archetype);

// get txd index?
static int ArchetypeGetDrawableF18(char* archetype)
{
	if (*(uint8_t*)(archetype + 96) == 2)
	{
		auto drbStore = (AssetStore*)g_drawableStore;

		uint16_t archetypeIdx = *(uint16_t*)(archetype + 102);
		auto entry = drbStore->pool.GetAt<char>(g_drawableIndices[archetypeIdx]);

		if (entry)
		{
			uint16_t idx = *(uint16_t*)(entry + 24);

			if (idx != 0xFFFF)
			{
				return idx;
			}
		}

		return -1;
	}
	else if (*(uint8_t*)(archetype + 96) == 2) // dwd
	{
		auto dwdStore = (AssetStore*)g_dwdStore;

		uint16_t archetypeIdx = *(uint16_t*)(archetype + 102);
		auto entry = dwdStore->pool.GetAt<char>(g_drawableIndices[archetypeIdx]);

		if (entry)
		{
			uint16_t idx = *(uint16_t*)(entry + 16);

			if (idx != 0xFFFF)
			{
				return idx;
			}
		}

		return -1;
	}

	return g_origGetDrawableF18(archetype);
}

static void AdjustLimits()
{
	g_drawableStore = hook::get_address<char*>(hook::get_pattern("74 16 8B 17 48 8D 0D ? ? ? ? 41 B8 02 00 00 00", 7));
	g_dwdStore = hook::get_address<char*>(hook::get_pattern("EB 7B 48 8D 54 24 40 48 8D 0D", 10));

	// not needed on 1365 anymore
	return;

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("41 8B 18 44 0F B7 81 80 00 00 00", -5), GetIndexByKeyStub, (void**)&g_origGetIndexByKey);
	MH_CreateHook(hook::get_pattern("48 89 7C 24 10 44 0F B7 49 10", -5)/*(void*)0x1414F9E68*/, StoreInsert, (void**)&g_origStoreInsert);
	MH_CreateHook((void*)hook::get_pattern("44 0F B7 41 10 44 8B DA 33 D2")/*0x1414F4A34*/, StoreRemove, (void**)&g_origStoreRemove);
	MH_CreateHook((void*)hook::get_pattern("48 63 DA 48 8B F9 8B C3", -20)/*0x1414F9790*/, StoreInit, (void**)&g_origStoreInit);
	MH_CreateHook((void*)hook::get_pattern("BE FF FF 00 00 C6 41 60 00", -16)/*0x141588FE4*/, ArchetypeSetDrawable, (void**)&g_origSetDrawable);
	MH_CreateHook((void*)hook::get_pattern("41 8B F8 48 8B D9 75 12", -16)/*0x141588FE4*/, ArchetypeSetDrawableDict, nullptr);
	MH_CreateHook((void*)hook::get_pattern("F6 41 50 01 4C 8B D1 75 03")/*0x141586390*/, ArchetypeGetDrawable, (void**)&g_origGetDrawable);
	MH_CreateHook((void*)hook::get_pattern("0F B6 51 60 41 83 C9 FF")/*0x14158615C*/, ArchetypeGetDrawableF18, (void**)&g_origGetDrawableF18);
	MH_EnableHook(MH_ALL_HOOKS);

	static struct : jitasm::Frontend
	{
		static int GetArchetype(char* archetype)
		{
			if (*(uint8_t*)(archetype + 96) != 2 && *(uint8_t*)(archetype + 96) != 3)
			{
				return *(uint16_t*)(archetype + 98);
			}

			uint16_t archetypeIdx = *(uint16_t*)(archetype + 102);			
			return g_drawableIndices[archetypeIdx];
		}

		virtual void InternalMain() override
		{
			push(rcx);
			push(r8);
			push(r9);
			sub(rsp, 0x20);

			mov(rcx, rdx);
			mov(rax, (uintptr_t)GetArchetype);
			call(rax);

			add(rsp, 0x20);
			pop(r9);
			pop(r8);
			pop(rcx);

			add(ecx, eax);

			ret();
		}
	} getArchetypeStub;

	{
		auto location = hook::get_pattern("0F B7 42 62 03 C8");
		hook::nop(location/*0x141586371*/, 6);
		hook::call(location, getArchetypeStub.GetCode());
	}

	// in setting of drawable dict index
	static struct : jitasm::Frontend
	{
		static int GetArchetype(char* archetype)
		{
			uint16_t archetypeIdx = *(uint16_t*)(archetype + 102);
			return g_drawableIndices[archetypeIdx];
		}

		virtual void InternalMain() override
		{
			push(rcx);
			push(r8);
			push(r9);
			sub(rsp, 0x20);

			mov(rcx, r10);
			mov(rax, (uintptr_t)GetArchetype);
			call(rax);

			add(rsp, 0x20);
			pop(r9);
			pop(r8);
			pop(rcx);

			mov(r8d, eax);

			ret();
		}
	} getArchetypeStub2;

	{
		auto location = hook::get_pattern("E9 84 00 00 00 45 0F B7 42 62", 5);
		hook::nop(location, 5);
		hook::call(location, getArchetypeStub2.GetCode());
	}
}

#include <Streaming.h>
#include <VFSManager.h>

#include <Error.h>

extern hook::cdecl_stub<rage::fiCollection*()> getRawStreamer;

#define VFS_GET_RCD_DEBUG_INFO 0x30001

struct GetRcdDebugInfoExtension
{
	const char* fileName; // in
	std::string outData; // out
};

static void ErrorInflateFailure(char* ioData, char* requestData, int zlibError, char* zlibStream)
{
	if (streaming::IsStreamerShuttingDown())
	{
		trace("Streamer shutdown: ignoring inflate() failure!\n");
		return;
	}

	uint32_t handle = *(uint32_t*)(requestData + 4);
	uint8_t* nextIn = *(uint8_t**)(ioData + 8);
	uint32_t availIn = *(uint32_t*)(ioData);
	uint32_t totalIn = *(uint32_t*)(ioData + 16);
	const char* msg = *(const char**)(zlibStream + 32);

	if (zlibError == -5 /* Z_BUF_ERROR */ && availIn == 0)
	{
		trace("Ignoring Z_BUF_ERROR with avail_in == 0.\n");
		return;
	}

	// get the entry name
	uint16_t fileIndex = (handle & 0xFFFF);
	uint16_t collectionIndex = (handle >> 16);

	auto spf = streaming::GetStreamingPackfileByIndex(collectionIndex);
	auto collection = (rage::fiCollection*)(spf ? spf->packfile : nullptr);

	if (!collection && collectionIndex == 0)
	{
		collection = getRawStreamer();
	}

	std::string name = fmt::sprintf("unknown - handle %08x", handle);
	std::string metaData;

	// get the input bytes
	auto compBytes = fmt::sprintf("%02x %02x %02x %02x %02x %02x %02x %02x", nextIn[0], nextIn[1], nextIn[2], nextIn[3], nextIn[4], nextIn[5], nextIn[6], nextIn[7]);

	// get collection metadata
	if (collection)
	{
		name = collection->GetEntryName(fileIndex);

		if (collectionIndex == 0)
		{
			// get the _raw_ file name
			char fileNameBuffer[1024];
			strcpy(fileNameBuffer, "CfxRequest");

			collection->GetEntryNameToBuffer(fileIndex, fileNameBuffer, sizeof(fileNameBuffer));

			auto virtualDevice = vfs::GetDevice(fileNameBuffer);

			// call into RCD
			GetRcdDebugInfoExtension ext;
			ext.fileName = fileNameBuffer;

			virtualDevice->ExtensionCtl(VFS_GET_RCD_DEBUG_INFO, &ext, sizeof(ext));

			metaData = ext.outData;
		}
	}
	else
	{
		metaData = "Null fiCollection.";
	}

	FatalError("Failed to call inflate() for streaming file %s.\n\n"
		"Error: %d: %s\nRead bytes: %s\nTotal in: %d\nAvailable in: %d\n"
		"%s\n\nPlease try restarting the game, or, if this occurs across servers, verifying your game files.",
		name,
		zlibError, (msg) ? msg : "(null)", compBytes, totalIn, availIn,
		metaData);
}

static void CompTrace()
{
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			mov(rcx, rdi);
			mov(rdx, r14);
			mov(r8d, eax);
			mov(r9, rbx);

			mov(rax, (uintptr_t)ErrorInflateFailure);
			jmp(rax);
		}
	} errorBit;

	hook::call_rcx(hook::get_pattern("B9 48 93 55 15 E8", 5), errorBit.GetCode());
}

static void* (*g_origSMPACreate)(void* a1, void* a2, size_t size, void* a4, bool a5);

static void* SMPACreateStub(void* a1, void* a2, size_t size, void* a4, bool a5)
{
       if (size == 0xD00000)
       {
               size = 0x1200000;
       }

       return g_origSMPACreate(a1, a2, size, a4, a5);
}

static void* GetNvapi(uint32_t hash)
{
	auto patternString = fmt::sprintf("74 27 B9 %02X %02X %02X %02x FF 15", hash & 0xFF, (hash >> 8) & 0xFF, (hash >> 16) & 0xFF, (hash >> 24) & 0xFF);
	auto p = hook::get_pattern(patternString, -0x97);

	return p;
}

static int NvAPI_Stereo_IsEnabled(bool* enabled)
{
	*enabled = 1;
	return 0;
}

static int NvAPI_Stereo_CreateHandleFromIUnknown(void* iunno, uintptr_t* hdl)
{
	*hdl = 1;
	return 0;
}

static int NvAPI_Stereo_Activate(uintptr_t hdl)
{
	return 0;
}

static int NvAPI_Stereo_IsActivated(uintptr_t hdl, uint8_t* on)
{
	*on = 1;
	return 0;
}

static void HookStereo()
{
	hook::jump(GetNvapi(0x348FF8E1), NvAPI_Stereo_IsEnabled);
	hook::jump(GetNvapi(0xAC7E37F4), NvAPI_Stereo_CreateHandleFromIUnknown);
	hook::jump(GetNvapi(0xF6A1AD68), NvAPI_Stereo_Activate);
	hook::jump(GetNvapi(0x1FB0BC30), NvAPI_Stereo_IsActivated);
}

static HookFunction hookFunction([] ()
{
#if 0
	hook::jump(hook::pattern("48 8B 48 08 48 85 C9 74  0C 8B 81").count(1).get(0).get<char>(-0x10), ReturnTrue);
	hook::put<uint8_t>(hook::pattern("80 3D ? ? ? ? ? 48 8B F1 74 07 E8 ? ? ? ? EB 05").count(1).get(0).get<void>(0xA), 0xEB);
	hook::put<uint8_t>(hook::pattern("0F 8E ? ? 00 00 80 3D ? ? ? ? 00 74 07 E8").count(1).get(0).get<void>(0xD), 0xEB);
	hook::put<uint8_t>(hook::pattern("74 12 48 FF C3 48 83 C0 04 48 81 FB").count(1).get(0).get<void>(-0xB), 0xEB);
	hook::put<uint8_t>(hook::pattern("74 63 45 8D 47 02 E8").count(1).get(0).get<void>(0), 0xEB);

	/*{
		auto m = hook::pattern("44 89 7C 24 60 81 FA DB  3E 14 7D 74 16").count(1).get(0);

		hook::put<uint8_t>(m.get<void>(5), 0x90);
		hook::put<uint8_t>(m.get<void>(6), 0xBA);
		hook::put<uint32_t>(m.get<void>(7), HashString("meow"));
		hook::nop(m.get<void>(11), 2);
	}

	{
		auto m = hook::pattern("44 89 44 24 60 81 FA DB  3E 14 7D 74 16").count(1).get(0);

		hook::put<uint8_t>(m.get<void>(5), 0x90);
		hook::put<uint8_t>(m.get<void>(6), 0xBA);
		hook::put<uint32_t>(m.get<void>(7), HashString("meow"));
		hook::nop(m.get<void>(11), 2);
	}*/

	hook::nop(hook::pattern("0F B6 05 ? ? ? ? 40 8A BB").count(1).get(0).get<char>(0), 0x7);
	hook::put<uint8_t>(hook::pattern("48 83 C6 04 49 2B EC 75 CB").count(1).get(0).get<void>(0x15), 0xEB);

	hook::put<uint8_t>(hook::pattern("F6 05 ? ? ? ? ? 74 08 84 C0 0F 84").count(1).get(0).get<void>(0x18), 0xEB);
#endif

	// 1604 (ported from 1737): increase rline allocator size using a hook (as Arxan)
	MH_Initialize();
	MH_CreateHook((void*)0x14127385C, SMPACreateStub, (void**)&g_origSMPACreate);
	MH_EnableHook(MH_ALL_HOOKS);

	if (!CfxIsSinglePlayer())
	{
		// population zone selection for network games
		hook::put<uint8_t>(hook::pattern("74 63 45 8D 47 02 E8").count(1).get(0).get<void>(0), 0xEB);

		// scenario netgame checks (NotAvailableInMultiplayer)
		hook::put<uint8_t>(hook::get_pattern("74 0D 8B 83 84 00 00 00 C1 E8 0F A8 01", 0), 0xEB);
		hook::put<uint8_t>(hook::get_pattern("74 1A 8B 49 38 E8", 0), 0xEB);

		// population netgame check
		hook::put<uint16_t>(hook::get_pattern("0F 84 8F 00 00 00 8B 44 24 40 8B C8"), 0xE990);

		// additional netgame checks for scenarios
		hook::nop(hook::get_pattern("B2 04 75 65 80 7B 39", 2), 2);
		hook::put<uint8_t>(hook::get_pattern("74 24 84 D2 74 20 8B 83", 4), 0xEB);
		hook::put<uint8_t>(hook::get_pattern("84 D2 75 41 8B 83", 0x5F), 0xEB);
		//hook::put<uint8_t>(hook::get_pattern("40 B6 01 74 52 F3 0F 10 01", 3), 0xEB); // this skips a world grid check, might be bad!

		// another scenario cluster network game check
		hook::put<uint8_t>(hook::get_pattern("80 78 1A 00 74 0F", 4), 0xEB);

		// more netgame (scenariovehicleinfo)
		hook::put<uint8_t>(hook::get_pattern("74 42 84 C0 75 42 83 C8"), 0xEB);

		// disabling animal types
		//hook::jump(hook::pattern("48 8B 48 08 48 85 C9 74  0C 8B 81").count(1).get(0).get<char>(-0x10), ReturnTrue);
		hook::jump(hook::get_pattern("75 1A 38 99 54 01 00 00 75 0E", -0xE), ReturnTrue);

		// scenario point network game check
		hook::put<uint8_t>(hook::get_pattern("74 0D 3C 02 0F 94 C0 38 05", 0), 0xEB);

		// netgame checks for vehicle fuel tank leaking
		hook::nop(hook::get_pattern("48 83 65 7F 00 0F 29 45 27 48 8D 45 77", -0xE3), 7);
		hook::nop(hook::get_call(hook::get_pattern<char>("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 84 C0 74 32 8A 83 D8", 9)) + 0x19, 2);
	}

	// increase the heap size for allocator 0
	hook::put<uint32_t>(hook::get_pattern("83 C8 01 48 8D 0D ? ? ? ? 41 B1 01 45 33 C0", 17), 600 * 1024 * 1024); // 600 MiB, default in 323 was 412 MiB

	// don't pass flag 4 for streaming requests of netobjs
	hook::put<int>(hook::get_pattern("BA 06 00 00 00 41 23 CE 44 33 C1 44 23 C6 41 33", 1), 2);

	// allow all procedural objects in network games
	hook::put<uint8_t>(hook::get_pattern("F6 42 30 20 75 09", 4), 0xEB);

	// make GTA default rage::fwMapData::ms_entityLevelCap to PRI_OPTIONAL_LOW, not PRI_OPTIONAL_MEDIUM (RAGE suite defaults)
	hook::put<uint32_t>(hook::get_pattern("BB 02 00 00 00 39 1D", 1), 3);

	// limit adjuster!
	AdjustLimits();

	// trace ERR_GEN_ZLIB_2 errors
	CompTrace();

	//HookStereo();
});
