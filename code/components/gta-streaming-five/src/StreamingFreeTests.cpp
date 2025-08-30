/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <shared_mutex>
#include <unordered_set>
#include <CrossBuildRuntime.h>
#include <Streaming.h>
#include "Hooking.h"
#include <Error.h>
#include <MinHook.h>
#include <jitasm.h>

#include <GameInit.h>
#include <CoreConsole.h>
#include <CL2LaunchMode.h>

#ifdef GTA_FIVE
#include <atPool.h>
#include "XBRVirtual.h"

struct ResBmInfoInt
{
	void* pad;
	uint8_t f8;
	uint8_t f9;
	uint8_t fA;
	uint8_t fB;
};

struct ResBmInfo
{
	void* pad;
	ResBmInfoInt* bm;
};

static void(*g_getBlockMap)(ResBmInfo*, void*);

void GetBlockMapWrap(ResBmInfo* info, void* bm)
{
	if (info->bm)
	{
		return g_getBlockMap(info, bm);
	}
	else
	{
		trace("tried to get a blockmap from a streaming entry without blockmap\n");
	}
}
#endif

namespace rage
{
class datBase
{
public:
	virtual ~datBase()
	{
	}
};

class strStreamingModule : public datBase
{
public:
	virtual ~strStreamingModule()
	{
	}

public:
	uint32_t baseIdx;
};

class fwAssetStoreBase : public strStreamingModule
{
public:
	virtual ~fwAssetStoreBase()
	{
	}

	bool IsResourceValid(uint32_t idx);
};
}

#if GTA_FIVE
static rage::strStreamingModule**(*g_getStreamingModule)(void*, uint32_t);

static hook::cdecl_stub<bool(rage::fwAssetStoreBase*, uint32_t)> fwAssetStoreBase__isResourceValid([] ()
{
	return hook::pattern("85 D2 78 3A 48 8B 41 40 4C 63 C2 46").count(1).get(0).get<void>(-6);
});

bool rage::fwAssetStoreBase::IsResourceValid(uint32_t idx)
{
	return fwAssetStoreBase__isResourceValid(this, idx);
}
#endif

static std::shared_mutex g_streamingMapMutex;

#ifdef _DEBUG
static std::map<std::string, uint32_t, std::less<>> g_streamingNamesToIndices;
static std::map<uint32_t, std::string> g_streamingIndexesToNames;
static std::map<uint32_t, std::string> g_streamingHashesToNames;
#else
static std::unordered_map<std::string, uint32_t> g_streamingNamesToIndices;
static std::unordered_map<uint32_t, std::string> g_streamingIndexesToNames;
static std::unordered_map<uint32_t, std::string> g_streamingHashesToNames;
#endif

// TODO: unordered_map with a custom hash
static std::map<std::tuple<streaming::strStreamingModule*, uint32_t>, uint32_t> g_streamingHashStoresToIndices;

extern std::unordered_set<std::string> g_streamingSuffixSet;

#ifdef GTA_FIVE
template<bool IsRequest>
rage::strStreamingModule** GetStreamingModuleWithValidate(void* streamingModuleMgr, uint32_t index)
{
	rage::strStreamingModule** streamingModulePtr = g_getStreamingModule(streamingModuleMgr, index);

	if (!xbr::IsGameBuildOrGreater<2802>())
	{
		rage::strStreamingModule* streamingModule = *streamingModulePtr;

		std::string typeName = typeid(*streamingModule).name();
		rage::fwAssetStoreBase* assetStore = dynamic_cast<rage::fwAssetStoreBase*>(streamingModule);

		if (assetStore)
		{
			if (!assetStore->IsResourceValid(index - assetStore->baseIdx))
			{
				trace("Tried to %s non-existent streaming asset %s (%d) in module %s\n", (IsRequest) ? "request" : "release", streaming::GetStreamingNameForIndex(index), index, typeName.c_str());

				AddCrashometry("streaming_free_validation", "true");
			}
		}
	}

	return streamingModulePtr;
}
#endif

#if GTA_FIVE
extern std::string g_lastStreamingName;
#elif IS_RDR3
std::string g_lastStreamingName;

static int (*g_origGetFileTypeIdFromExtension)(const char*, const char*);
static int GetFileTypeIdFromExtension(const char* extension, const char* fileName)
{
	// set the name for the later hook to use
	g_lastStreamingName = fileName;

	return g_origGetFileTypeIdFromExtension(extension, fileName);
}
#endif


uint32_t* AddStreamingFileWrap(uint32_t* indexRet)
{
	if (*indexRet != -1)
	{
#if 0
		if (g_lastStreamingName.find(".ymap") != std::string::npos)
		{
			trace("registered mapdata %s as %d\n", g_lastStreamingName.c_str(), *indexRet);
		}
#endif

		auto store = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule(*indexRet);
		auto baseIdx = store->baseIdx;
		auto baseFn = g_lastStreamingName.substr(0, g_lastStreamingName.find_last_of('.'));

		{
			std::unique_lock _(g_streamingMapMutex);
			g_streamingNamesToIndices[g_lastStreamingName] = *indexRet;
			g_streamingIndexesToNames[*indexRet] = g_lastStreamingName;

			g_streamingHashesToNames[HashString(baseFn.c_str())] = baseFn;
			g_streamingHashStoresToIndices[{ store, HashString(baseFn.c_str()) }] = *indexRet - baseIdx;
		}

		auto splitIdx = g_lastStreamingName.find_first_of("_");

		if (splitIdx != std::string::npos)
		{
			auto splitBit = g_lastStreamingName.substr(splitIdx + 1);

			if (splitBit.find('_') != std::string::npos)
			{
				g_streamingSuffixSet.insert(splitBit);
			}
		}
	}

	return indexRet;
}

namespace streaming
{
	uint32_t GetStreamingIndexForName(const std::string& name)
	{
		std::shared_lock _(g_streamingMapMutex);

		auto it = g_streamingNamesToIndices.find(name);

		if (it != g_streamingNamesToIndices.end())
		{
			return it->second;
		}

		return 0;
	}

	std::string GetStreamingNameForIndex(uint32_t index)
	{
		std::shared_lock _(g_streamingMapMutex);

		auto it = g_streamingIndexesToNames.find(index);

		if (it != g_streamingIndexesToNames.end())
		{
			return it->second;
		}

		return "";
	}

	std::string GetStreamingBaseNameForHash(uint32_t hash)
	{
		std::shared_lock _(g_streamingMapMutex);

		auto it = g_streamingHashesToNames.find(hash);

		if (it != g_streamingHashesToNames.end())
		{
			return it->second;
		}

		return "";
	}

	uint32_t GetStreamingIndexForLocalHashKey(streaming::strStreamingModule* module, uint32_t hash)
	{
		auto entry = g_streamingHashStoresToIndices.find({ module, hash });

		if (entry != g_streamingHashStoresToIndices.end())
		{
			return entry->second;
		}

		return -1;
	}
}

struct strStreamingInterface
{
		virtual ~strStreamingInterface() = 0;

		virtual void LoadAllRequestedObjects(bool) = 0;

		virtual void RequestFlush() = 0;
};

static strStreamingInterface** g_strStreamingInterface;

#ifdef GTA_FIVE
void(*g_origAssetRelease)(void*, uint32_t);

struct AssetStore
{
	void* vtable;
	uint32_t baseIndex;
	uint32_t pad1;
	char pad[40];
	atPoolBase pool;
};

void WrapAssetRelease(AssetStore* assetStore, uint32_t entry)
{
	auto d = assetStore->pool.GetAt<void*>(entry);

	if (d && *d)
	{
		g_origAssetRelease(assetStore, entry);
	}
	else
	{
		trace("didn't like entry %d - %s :(\n", entry, streaming::GetStreamingNameForIndex(assetStore->baseIndex + entry));
	}
}

namespace rage
{
	struct PageMap
	{
		void* f0;
		uint8_t f8;
		uint8_t f9;
		uint8_t fA;

		void* pageInfo[3 * 128];
	};

	struct pgBase
	{
		void* vtbl;
		PageMap* pageMap;
	};

	struct datResourceMap
	{
		uint8_t numPages1;
		uint8_t numPages2;
	};
}

static void(*g_origPgBaseDtor)(rage::pgBase* self);

static void pgBaseDtorHook(rage::pgBase* self)
{
	g_origPgBaseDtor(self);

	if (self->pageMap)
	{
		auto extraAllocator = rage::GetAllocator()->GetAllocator(1);

		if (extraAllocator->GetSize(self->pageMap))
		{
			extraAllocator->Free(self->pageMap);
			self->pageMap = nullptr;
		}
	}
}

static void(*g_origMakeDefragmentable)(rage::pgBase*, const rage::datResourceMap&, bool);

static void MakeDefragmentableHook(rage::pgBase* self, const rage::datResourceMap& map, bool a3)
{
	auto pageMap = self->pageMap;

	if (pageMap && (pageMap->f8 != map.numPages1 || pageMap->f9 != map.numPages2))
	{
		auto extraAllocator = rage::GetAllocator()->GetAllocator(1);
		auto newPageMap = (rage::PageMap*)extraAllocator->Allocate(sizeof(rage::PageMap), 16, 0);
		memcpy(newPageMap, pageMap, offsetof(rage::PageMap, pageInfo) + (3 * sizeof(void*) * (map.numPages1 + map.numPages2)));

		self->pageMap = newPageMap;
	}

	g_origMakeDefragmentable(self, map, a3);
}

#endif

static HookFunction hookFunction([] ()
{
	static ConVar<bool> enableFlush("str_enableFlush", ConVar_Replicated, false);
	static ConsoleCommand flushCommand("str_requestFlush", []()
	{
#ifndef _DEBUG
		if (!launch::IsSDK() && !launch::IsSDKGuest() && !enableFlush.GetValue())
		{
			trace("str_requestFlush requires the 'str_enableFlush' replicated convar to be enabled");
			return;
		}
#endif
		if (*g_strStreamingInterface)
		{
			(*g_strStreamingInterface)->RequestFlush();
		}
	});

#if GTA_FIVE
	g_strStreamingInterface = hook::get_address<decltype(g_strStreamingInterface)>(hook::get_pattern("48 8B 0D ? ? ? ? 48 8B 01 FF 90 90 00 00 00 B9", 3));
#elif IS_RDR3
	g_strStreamingInterface = hook::get_address<decltype(g_strStreamingInterface)>(hook::get_pattern("F6 04 01 01 74 0D 48 8B 0D", 9));
#endif

#if GTA_FIVE
	if (xbr::IsGameBuildOrGreater<2802>())
	{
		void* getBlockMapCall = hook::pattern("4D 85 E4 74 0D 48 8D 54 24 20 49 8B CC E8").count(1).get(0).get<void>(13);
		hook::set_call(&g_getBlockMap, getBlockMapCall);
		hook::call(getBlockMapCall, GetBlockMapWrap);
	}
	else
	{
		void* getBlockMapCall = hook::pattern("CC FF 50 48 48 85 C0 74 0D").count(1).get(0).get<void>(17);
		hook::set_call(&g_getBlockMap, getBlockMapCall);
		hook::call(getBlockMapCall, GetBlockMapWrap);
	}

	// debugging for non-existent streaming requests
	{
		void* location = hook::pattern("F3 AB 48 8D 8E B8 01 00 00 E8").count(1).get(0).get<void>(9);
		hook::set_call(&g_getStreamingModule, location);
		hook::call(location, GetStreamingModuleWithValidate<true>);
	}

	// debugging for non-existent streaming releases
	{
		void* location = hook::pattern("83 FD 01 0F 85 ? 01 00 00").count(1).get(0).get<void>(16);
		hook::call(location, GetStreamingModuleWithValidate<false>);
	}
#endif

	{
#ifdef GTA_FIVE
		void* location = hook::pattern("83 CE FF 89 37 48 8B C7 48 8B 9C 24").count(1).get(0).get<void>(29);

		static struct : public jitasm::Frontend
		{
			void* addFunc;

			void InternalMain() override
			{
				pop(r12);
				pop(rdi);
				pop(rsi);
				pop(rbp);

				mov(rcx, rax);
				mov(rax, (uint64_t)addFunc);
				jmp(rax);
			}
		} doStub;

		doStub.addFunc = AddStreamingFileWrap;
		hook::jump_rcx(location, doStub.GetCode());
#elif IS_RDR3
		void* location = hook::pattern("83 0F FF 48 8D 4D C8 E8 3C 93 FE FF").count(1).get(0).get<void>(27);

		static struct : public jitasm::Frontend
		{
			void* addFunc;

			void InternalMain() override
			{
				pop(rdi);
				pop(rsi);
				pop(rbx);
				pop(rbp);

				mov(rcx, rax);
				mov(rax, (uint64_t)addFunc);
				jmp(rax);
			}
		} doStub;

		doStub.addFunc = AddStreamingFileWrap;
		hook::jump_rcx(location, doStub.GetCode());
#endif
	}

#if IS_RDR3
	{
		void* location = hook::get_pattern("33 D2 48 8D 4C 24 60 8B D8 E8", -5);
		hook::set_call(&g_origGetFileTypeIdFromExtension, location);
		hook::call(location, GetFileTypeIdFromExtension);
	}
#endif

#if GTA_FIVE
	// avoid releasing released clipdictionaries
	{
		void* loc = hook::get_pattern("48 8B D9 E8 ? ? ? ? 48 8B 8B 98 00 00 00 48", 3);
		hook::set_call(&g_origAssetRelease, loc);
		hook::call(loc, WrapAssetRelease);
	}

	// same for mapdatastore
	{
		// vehicle metadata unmount: don't die if a vehicle is not loaded
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				not(rax);
				and(rbx, rax);

				jz("justReturn");

				mov(rax, qword_ptr[rbx]);
				test(rax, rax);

				L("justReturn");

				ret();
			}
		} fixStub;

		{
			auto location = hook::get_pattern("48 F7 D0 48 23 D8 0F 84 C6 00 00 00 48", 0);
			hook::nop(location, 6);
			hook::call_reg<1>(location, fixStub.GetCode());
		}
	}

	// since various versions of ZModeler don't write valid page map pointers (not reserving enough memory for the page map,
	// instead reserving a much smaller amount) and this leads to corrupting random values

	// common crash hash (1604): lithium-leopard-fourteen, crSkeletonBase bone list counter overwritten

	MH_Initialize();
	
	// pgBase constructor -> MakeDefragmentable
	MH_CreateHook(hook::get_pattern("41 8A F0 4D 85 C9 74 07 41 80 79 0B 00", -0x18), MakeDefragmentableHook, (void**)&g_origMakeDefragmentable);

	// pgBase destructor, to free the relocated page map we created
	MH_CreateHook(hook::get_pattern("48 81 EC 48 0C 00 00 48 8B"), pgBaseDtorHook, (void**)&g_origPgBaseDtor);

	MH_EnableHook(MH_ALL_HOOKS);
#endif
});
