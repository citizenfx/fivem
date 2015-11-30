/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

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

		g_streamingIndexesToNames[*indexRet] = g_lastStreamingName;
	}

	return indexRet;
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
});