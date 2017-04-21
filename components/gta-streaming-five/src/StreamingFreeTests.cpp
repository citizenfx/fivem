/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include <atPool.h>

#include <Streaming.h>

#include <Error.h>

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

namespace rage
{
	class datBase
	{
	public:
		virtual ~datBase() {}
	};

	class strStreamingModule : public datBase
	{
	public:
		virtual ~strStreamingModule() {}

	public:
		uint32_t baseIdx;
	};

	class fwAssetStoreBase : public strStreamingModule
	{
	public:
		virtual ~fwAssetStoreBase() {}

		bool IsResourceValid(uint32_t idx);
	};
}

static rage::strStreamingModule**(*g_getStreamingModule)(void*, uint32_t);

static hook::cdecl_stub<bool(rage::fwAssetStoreBase*, uint32_t)> fwAssetStoreBase__isResourceValid([] ()
{
	return hook::pattern("85 D2 78 3A 48 8B 41 40 4C 63 C2 46").count(1).get(0).get<void>(-6);
});

bool rage::fwAssetStoreBase::IsResourceValid(uint32_t idx)
{
	return fwAssetStoreBase__isResourceValid(this, idx);
}

static std::map<std::string, uint32_t> g_streamingNamesToIndices;
static std::map<uint32_t, std::string> g_streamingIndexesToNames;

template<bool IsRequest>
rage::strStreamingModule** GetStreamingModuleWithValidate(void* streamingModuleMgr, uint32_t index)
{
	rage::strStreamingModule** streamingModulePtr = g_getStreamingModule(streamingModuleMgr, index);
	rage::strStreamingModule* streamingModule = *streamingModulePtr;

	std::string typeName = typeid(*streamingModule).name();
	rage::fwAssetStoreBase* assetStore = dynamic_cast<rage::fwAssetStoreBase*>(streamingModule);
	
	if (assetStore)
	{
		if (!assetStore->IsResourceValid(index - assetStore->baseIdx))
		{
			FatalError("Tried to %s non-existent streaming asset %s (%d) in module %s", (IsRequest) ? "request" : "release", g_streamingIndexesToNames[index].c_str(), index, typeName.c_str());
		}
	}

	return streamingModulePtr;
}

extern std::string g_lastStreamingName;

static FILE* sfLog;// = fopen("B:\\sf.log", "w");

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

		if (sfLog)
		{
			fprintf(sfLog, "registered %s as %d\n", g_lastStreamingName.c_str(), *indexRet);
			fflush(sfLog);
		}

		g_streamingNamesToIndices[g_lastStreamingName] = *indexRet;
		g_streamingIndexesToNames[*indexRet] = g_lastStreamingName;
	}

	return indexRet;
}

namespace streaming
{
	uint32_t GetStreamingIndexForName(const std::string& name)
	{
		return g_streamingNamesToIndices[name];
	}

	const std::string& GetStreamingNameForIndex(uint32_t index)
	{
		return g_streamingIndexesToNames[index];
	}
}

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
		trace("didn't like entry %d - %s :(\n", entry, g_streamingIndexesToNames[assetStore->baseIndex + entry].c_str());
	}
}

static HookFunction hookFunction([] ()
{
	void* getBlockMapCall = hook::pattern("CC FF 50 48 48 85 C0 74 0D").count(1).get(0).get<void>(17);

	hook::set_call(&g_getBlockMap, getBlockMapCall);
	hook::call(getBlockMapCall, GetBlockMapWrap);

	// debugging for non-existent streaming requests
	{
		void* location = hook::pattern("F3 AB 48 8D 8E B8 01 00 00 E8").count(1).get(0).get<void>(9);
		hook::set_call(&g_getStreamingModule, location);
		hook::call(location, GetStreamingModuleWithValidate<true>);
	}

	// debugging for non-existent streaming releases
	{
		void* location = hook::pattern("83 FD 01 0F 85 4D 01 00 00").count(1).get(0).get<void>(16);
		hook::call(location, GetStreamingModuleWithValidate<false>);
	}

	{
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
	}

	// avoid releasing released clipdictionaries
	{
		void* loc = hook::get_pattern("48 8B D9 E8 ? ? ? ? 48 8B 8B 90 00 00 00 48", 3);
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
});