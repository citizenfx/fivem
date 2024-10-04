/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <fiCollectionWrapper.h>

#include <jitasm.h>
#include <Hooking.h>

#include <fnv.h>

#include <Error.h>

#include <ICoreGameInit.h>

//#define CFX_COLLECTION_DISABLE 1

// unset _DEBUG so that there will be no range checking
#ifdef _DEBUG
#undef _DEBUG
#define DEBUG_WAS_SET
#endif

#include <tbb/concurrent_unordered_map.h>
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

atArray<StreamingPackfileEntry>* g_streamingPackfiles;

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
	uint32_t namePrefixLength;
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

std::string GetCurrentStreamingName();

static void PtrError()
{
	if (CoreIsDebuggerPresent())
	{
 		__debugbreak();
	}

	FatalError("Invalid fixup, address is neither virtual nor physical (in %s)", GetCurrentStreamingName());
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
		void* vtbl;
		uint32_t m_baseIndex;

	public:
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

static std::vector<std::pair<std::string, rage::ResourceFlags>> g_customStreamingFiles;
static std::map<std::string, std::vector<std::pair<std::string, rage::ResourceFlags>>, std::less<>> g_customStreamingFilesByTag;

void origCfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags)
{
#ifndef CFX_COLLECTION_DISABLE
	auto baseName = std::string(strrchr(fileName.c_str(), '/') + 1);

	g_customStreamingFilesByTag[tag].push_back({ fileName, flags });
#endif
}

void origCfxCollection_BackoutStreamingTag(const std::string& tag)
{
	g_customStreamingFilesByTag.erase(tag);
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
extern fwEvent<> OnReloadMapStore;

static const char* RegisterStreamingFileStrchrWrap(const char* str, const int ch)
{
	// do this here, as it's early
	static bool inited = ([]()
	{
		g_streamingPackfiles->Get(0).isHdd = false;

		return true;
	})();

	// set the name for the later hook to use
	g_lastStreamingName = str;

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
	StreamingPackfileEntry* GetStreamingPackfileByIndex(int index)
	{
		return &g_streamingPackfiles->Get(index);
	}

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

static int ReturnTrue()
{
	return true;
}

#include <ICoreGameInit.h>
#include <GameInit.h>

static HookFunction hookFunction([] ()
{
	assert(offsetof(CollectionData, name) == 120);
	assert(offsetof(StreamingPackfileEntry, enabled) == 68);

	static_assert(sizeof(StreamingPackfileEntry) == 104, "SFPE");

	ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();
	
	OnKillNetworkDone.Connect([] ()
	{
		// clear blacklisted files
		g_registeredFileSet.clear();

		// clear custom files
		g_customStreamingFiles.clear();

		// clear new custom files
		g_customStreamingFilesByTag.clear();
	}, -500);

	// will rescan dlc collisions, if a dlc collision is overriden it breaks(?)
	OnReloadMapStore.Connect([]()
	{
		g_registeredFileSet.clear();
	});

	{
		void* location = hook::pattern("48 8B FA 89 44 24 30 48 8B D9 E8 ? ? ? ? 0F").count(1).get(0).get<void>(10);

		hook::set_call(&g_origAddCollision, location);
		hook::call(location, AddCollisionWrap);
	}

	// streaming module
	char* location = hook::pattern("48 63 D9 A8 01 75 23 83 C8 01").count(1).get(0).get<char>(0xD);

	g_streamingModule = (rage::strStreamingModule*)(location + *(int32_t*)location + 4);

	// streaming entries
	g_streamingEntries = hook::get_address<StreamingEntry**>(hook::get_pattern("48 8B 05 ? ? ? ? 03 0D ? ? ? ? 8B 54 C8 04"), 3, 7);

	// streaming packfile entries
	location = hook::pattern("48 8B 05 ? ? ? ? 48 8B CB 48 6B C9 68 80 64").count(1).get(0).get<char>(3);

	g_streamingPackfiles = (decltype(g_streamingPackfiles))(location + *(int32_t*)location + 4);

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

	// streaming file registration - disable DLC override capability if the file is also existent in global streaming
	g_registerStreamingFile = hook::pattern("48 8B D8 41 8B C6 25 00 00 00 C0 3D").count(1).get(0).get<char>(-0x42);

	hook::call(g_registerStreamingFile + 0x3A, RegisterStreamingFileStrchrWrap);

	// (not temp dbg: )InvalidFile overwrite fuckery
	hook::nop(hook::get_pattern("33 D2 E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8D 15", -7), 58);

	// 'should packfile meta cache (pfm.dat) be used'
	//hook::call(hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? 00 00 44 39 35", 5), ReturnTrue);

	// make the pfm.dat read-only
	{
		auto loc = hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? 00 00 44 39 35", 70);
		hook::set_call(&rage__fiFile__Open, loc);
		hook::call(loc, rage__fiFile__OpenWrap);
	}
});
