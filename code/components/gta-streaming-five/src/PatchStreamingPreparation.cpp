#include "StdInc.h"

#include <jitasm.h>
#include "Hooking.h"

#include <MinHook.h>

#include "Streaming.h"
#include <fiCollectionWrapper.h>

#include <Error.h>
#include <ICoreGameInit.h>

#include <nutsnbolts.h>

#include <optick.h>

#include <unordered_map>
#include <unordered_set>

#include <VFSError.h>
#include <VFSManager.h>

#include <EntitySystem.h>
#include <GameInit.h>

#include <tbb/concurrent_queue.h>

static tbb::concurrent_queue<std::function<void()>> g_onCriticalFrameQueue;
static int (*g_origHandleObjectLoad)(streaming::Manager*, int, int, int*, int, int, int);

static std::unordered_map<std::string, std::tuple<rage::fiDevice*, uint64_t, uint64_t>> g_handleMap;
static std::unordered_map<std::string, int> g_failures;

hook::cdecl_stub<rage::fiCollection*()> getRawStreamer([]()
{
	return hook::get_call(hook::get_pattern("48 8B D3 4C 8B 00 48 8B C8 41 FF 90 ? 01 00 00 8B D8 E8", -5));
});

struct SemaAwaiter
{
	bool operator()(void* sema) const
	{
		return WaitForSingleObject(sema, 0) == WAIT_OBJECT_0;
	}
};

struct ByteAwaiter
{
	bool operator()(void* sema) const
	{
		return *((uint8_t*)sema + 0x1848);
	}
};

template<typename Awaiter>
static bool ProcessHandler(void* sema, char* a1)
{
	bool isSignaled = Awaiter()(sema);

	if (!isSignaled)
	{
		auto& ref = *(uint32_t*)&a1[199452];
		--*(DWORD*)(a1 + 199456);

		if (++ref == 32)
		{
			ref = 0;
		}

		auto v6 = *(DWORD64*)(a1 + 8i64 * ref + 199192);

		auto v7 = *(DWORD*)(a1 + 199456);
		if (v7 < 32)
		{
			auto v8 = *(DWORD*)(a1 + 199448) + 1;
			if (*(DWORD*)(a1 + 199448) == 31)
				v8 = 0;
			*(DWORD64*)(a1 + 8i64 * v8 + 199192) = v6;
			*(DWORD*)(a1 + 199448) = v8;
			*(DWORD*)(a1 + 199456) = v7 + 1;
		}
	}

	return isSignaled;
}

static void Hook_StreamingSema2699()
{
	{
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(rdx, rbx);

				mov(rax, (uint64_t)&ProcessHandler<ByteAwaiter>);
				jmp(rax);
			}
		} weirdStub;

		// rage::strStreamingLoader::ProcessStreamFiles
		auto location = hook::get_pattern("8A 81 48 18 00 00 84 C0 0F 84 E5");
		hook::nop(location, 6);
		hook::call(location, weirdStub.GetCode());
	}

	{
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(rdx, rsi);

				mov(rax, (uint64_t)&ProcessHandler<ByteAwaiter>);
				jmp(rax);
			}
		} weirdStub;

		// rage::strStreamingLoader::Flush
		auto location = hook::get_pattern("8A 81 48 18 00 00 84 C0 0F 84 10 01");
		hook::nop(location, 6);
		hook::call(location, weirdStub.GetCode());
	}
}

static void Hook_StreamingSema()
{
	// 2699+ uses a byte instead of a sema, so needs its own patch
	if (xbr::IsGameBuildOrGreater<2699>())
	{
		Hook_StreamingSema2699();
		return;	
	}

	{
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(rdx, rbx);

				mov(rax, (uint64_t)&ProcessHandler<SemaAwaiter>);
				jmp(rax);
			}
		} weirdStub;

		auto location = hook::get_pattern("48 8B 8C C3 18 0A 03 00 48", 12);
		hook::call(location, weirdStub.GetCode());
	}

	{
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(rdx, rsi);

				mov(rax, (uint64_t)&ProcessHandler<SemaAwaiter>);
				jmp(rax);
			}
		} weirdStub;

		auto location = hook::get_pattern("48 8B 8C C6 18 0A 03 00 48", 12);
		hook::call(location, weirdStub.GetCode());
	}
}

struct datResourceChunk
{
	void* rscPtr;
	void* memPtr;
	size_t size;
};

#define VFS_RCD_REQUEST_HANDLE 0x30003

struct RequestHandleExtension
{
	vfs::Device::THandle handle;
	std::function<void(bool success, const std::string& error)> onRead;
};

#include <tbb/concurrent_queue.h>

static tbb::concurrent_queue<std::list<uint32_t>> g_removalQueue;
static tbb::concurrent_queue<HANDLE> g_forceSemaQueue;

static void ProcessRemoval()
{
	// we need to signal all semas first for rage::strStreamingLoader::CancelRequest to actually
	// cancel the request. this would be bad if we were already in pgStreamer, but since our custom requests
	// never hit the real pgStreamer and are canceled for all intents and purposes, we can signal the sema safely
	HANDLE h;
	std::list<HANDLE> semas;

	while (g_forceSemaQueue.try_pop(h))
	{
		ReleaseSemaphore(h, 1, NULL);
		semas.push_front(h);
	}

	std::list<uint32_t> r;
	auto str = streaming::Manager::GetInstance();

	while (g_removalQueue.try_pop(r))
	{
		for (auto i : r)
		{
			// ClearRequiredFlag
			str->ReleaseObject(i, 0xF1);

			// unmark as dependent (safe, as `r` should contain all dependents)
			str->Entries[i].flags &= ~0xFFFC;

			// RemoveObject
			str->ReleaseObject(i);
		}
	}

	// if any sema wasn't polled, unsignal so we won't accidentally trigger a load op
	// (if a sema has >1 count, this'd be wrong, but they're used as an auto-reset event here since many platforms do not
	// implement auto-reset events)
	for (HANDLE h : semas)
	{
		WaitForSingleObject(h, 0);
	}
}

bool IsHandleCache(uint32_t handle, std::string* outFileName)
{
	if (outFileName)
	{
		*outFileName = {};
	}

	rage::fiCollection* collection = nullptr;

	if ((handle >> 16) == 0)
	{
		collection = getRawStreamer();
	}

	bool isCache = false;

	if (collection)
	{
		char fileNameBuffer[1024];
		strcpy(fileNameBuffer, "CfxRequest");

		collection->GetEntryNameToBuffer(handle & 0xFFFF, fileNameBuffer, sizeof(fileNameBuffer));

		if (strncmp(fileNameBuffer, "cache:/", 7) == 0)
		{
			if (outFileName)
			{
				*outFileName = std::string("cache_nb:/") + &fileNameBuffer[7];
			}

			isCache = true;
		}
		else if (strncmp(fileNameBuffer, "compcache:/", 11) == 0)
		{
			if (outFileName)
			{
				*outFileName = std::string("compcache_nb:/") + &fileNameBuffer[11];
			}

			isCache = true;
		}
	}

	return isCache;
}

static void* (*g_origPgStreamerRead)(uint32_t handle, datResourceChunk* outChunks, int numChunks, int flags, void(*callback)(void*), void* userData, int streamerIdx, int streamerFlags, void* unk9, float unk10);

static void* pgStreamerRead(uint32_t handle, datResourceChunk* outChunks, int numChunks, int flags, void (*callback)(void*), void* userData, int streamerIdx, int streamerFlags, void* unk9, float unk10)
{
	std::string fileName;

	{
		if (IsHandleCache(handle, &fileName))
		{
			for (int i = 0; i < numChunks; i++)
			{
				if (outChunks[i].size > 100000000)
				{
					FatalError("ERR_STR_FAILURE_3: page %d in paged resource %s is over 100 MB in size (%.2f MB), which is unsupported by RAGE.", i, fileName, outChunks[i].size / 1000.0 / 1000.0);
				}
			}

			auto device = vfs::GetDevice(fileName);

			uint64_t ptr;
			auto deviceHandle = device->OpenBulk(fileName, &ptr);

			if (deviceHandle != INVALID_DEVICE_HANDLE)
			{
				RequestHandleExtension ext;
				ext.handle = deviceHandle;
				auto resultCb = [fileName, handle, outChunks, numChunks, flags, callback, userData, streamerIdx, streamerFlags, unk9, unk10](bool success, const std::string& error)
				{
					if (success)
					{
						g_origPgStreamerRead(handle, outChunks, numChunks, flags, callback, userData, streamerIdx, streamerFlags, unk9, unk10);
					}
					else
					{
						std::string errorRef;
						ICoreGameInit* init = Instance<ICoreGameInit>::Get();

						init->GetData("gta-core-five:loadCaller", &errorRef);

						trace("Failed to request %s: %s. %s\n", fileName, error, errorRef);
						rage::sysMemAllocator::UpdateAllocatorValue();

						std::list<uint32_t> removeIndices;

						// hunt down *requested* dependents on this failure
						auto strMgr = streaming::Manager::GetInstance();
						auto strIdx = *(uint32_t*)((char*)userData + 4);

						std::function<void(uint32_t)> addDependents = [&addDependents, &removeIndices, strMgr](uint32_t strIdx)
						{
							if (strIdx == -1)
							{
								return;
							}

							atArray<uint32_t> dependents;
							strMgr->FindAllDependentsCustomPred(dependents, strIdx, [](const StreamingDataEntry& entry)
							{
								auto flags = entry.flags & 3;

								return (flags == 2 || flags == 3);
							});

							for (auto dep : dependents)
							{
								addDependents(dep);
							}

							auto flags = strMgr->Entries[strIdx].flags & 3;

							if (flags == 2 || flags == 3)
							{
								removeIndices.push_back(strIdx);
							}
						};

						addDependents(strIdx);

						g_forceSemaQueue.push(*(HANDLE*)((char*)userData + 8));

						if (!removeIndices.empty())
						{
							g_removalQueue.push(std::move(removeIndices));
						}
					}
				};

				ext.onRead = [resultCb](bool success, const std::string& error)
				{
					// if succeeding, report sooner rather than later
					if (success)
					{
						resultCb(success, error);
						return;
					}

					g_onCriticalFrameQueue.push([resultCb, error]()
					{
						resultCb(false, error);
					});
				};

				device->ExtensionCtl(VFS_RCD_REQUEST_HANDLE, &ext, sizeof(ext));
			}

			static int hi;
			return &hi;
		}
	}

	return g_origPgStreamerRead(handle, outChunks, numChunks, flags, callback, userData, streamerIdx, streamerFlags, unk9, unk10);
}

#define VFS_RCD_SET_WEIGHT 0x30004

struct SetWeightExtension
{
	const char* fileName;
	int newWeight;
};

static void SetHandleDownloadWeight(uint32_t handle, int weight)
{
	std::string fileName;

	if (IsHandleCache(handle, &fileName))
	{
		auto device = vfs::GetDevice(fileName);

		if (device.GetRef())
		{
			SetWeightExtension ext;
			ext.fileName = fileName.c_str();
			ext.newWeight = weight;

			device->ExtensionCtl(VFS_RCD_SET_WEIGHT, &ext, sizeof(ext));
		}
	}
}

static bool (*g_origCancelRequest)(void* self, uint32_t index);

static bool CancelRequestWrap(void* self, uint32_t index)
{
	auto streaming = streaming::Manager::GetInstance();
	auto handle = streaming->Entries[index].handle;

	SetHandleDownloadWeight(handle, 1);

	return g_origCancelRequest(self, index);
}

static void (*g_origSetToLoading)(void*, uint32_t);

static void SetToLoadingWrap(streaming::Manager* self, uint32_t index)
{
	auto handle = self->Entries[index].handle;
	SetHandleDownloadWeight(handle, -1);

	return g_origSetToLoading(self, index);
}

static uint32_t NoLSN(void* streamer, uint16_t idx)
{
	return idx;
}

extern int* g_archetypeStreamingIndex;

static auto GetArchetypeModule()
{
	auto mgr = streaming::Manager::GetInstance();
	return mgr->moduleMgr.modules[*g_archetypeStreamingIndex];
}

static auto GetModelIndex(rage::fwArchetype* archetype)
{
	return *(uint16_t*)((char*)archetype + 106);
}

static auto GetStrIndexFromArchetype(rage::fwArchetype* archetype)
{
	auto modelIndex = GetModelIndex(archetype);
	return GetArchetypeModule()->baseIdx + modelIndex;
}

static std::mutex strRefCountsMutex;
static std::unordered_map<uint32_t, int> strRefCounts;

static bool (*g_origSetModelId)(void* entity, void* id);

static bool CEntity_SetModelIdWrap(rage::fwEntity* entity, rage::fwModelId* id)
{
	auto rv = g_origSetModelId(entity, id);

	if (auto archetype = entity->GetArchetype())
	{
		auto mgr = streaming::Manager::GetInstance();

		// TODO: recurse?
		uint32_t outDeps[150];
		auto module = GetArchetypeModule();
		auto numDeps = module->GetDependencies(GetModelIndex(archetype), outDeps, std::size(outDeps));

		for (size_t depIdx = 0; depIdx < numDeps; depIdx++)
		{
			auto dep = outDeps[depIdx];
			bool rerequest = false;

			{
				std::unique_lock _(strRefCountsMutex);
				auto it = strRefCounts.find(dep);

				if (it == strRefCounts.end())
				{
					it = strRefCounts.insert({ dep, 0 }).first;
				}

				++it->second;

				if (it->second == 1)
				{
					rerequest = true;
				}
			}

			if (rerequest)
			{
				SetHandleDownloadWeight(mgr->Entries[dep].handle, -1);
			}
		}
	}

	return rv;
}

static hook::thiscall_stub<void(void*, const rage::fwModelId&)> _fwEntity_SetModelId([]()
{
	return hook::get_pattern("E8 ? ? ? ? 45 33 D2 3B 05", -12);
});

static void (*g_orig_fwEntity_Dtor)(void* entity);

static void fwEntity_DtorWrap(rage::fwEntity* entity)
{
	if (auto archetype = entity->GetArchetype())
	{
		auto midx = GetModelIndex(archetype);

		// check if the archetype is actually valid for this model index
		// (some bad asset setups lead to a corrupted archetype list)
		//
		// to do this, we use a little hack to not have to manually read the archetype list:
		// the base rage::fwEntity::SetModelId doesn't do anything other than setting (rcx + 32)
		// to the resolved archetype, or nullptr if none
		rage::fwModelId modelId;
		modelId.id = midx;

		void* fakeEntity[40 / 8] = { 0 };
		_fwEntity_SetModelId(fakeEntity, modelId);

		// not the same archetype - bail out
		if (fakeEntity[4] != archetype)
		{
			return g_orig_fwEntity_Dtor(entity);
		}

		auto mgr = streaming::Manager::GetInstance();

		// TODO: recurse?
		uint32_t outDeps[150];
		auto module = GetArchetypeModule();
		auto numDeps = module->GetDependencies(midx, outDeps, std::size(outDeps));

		for (size_t depIdx = 0; depIdx < numDeps; depIdx++)
		{
			auto dep = outDeps[depIdx];
			bool unrequest = false;

			{
				std::unique_lock _(strRefCountsMutex);

				if (auto it = strRefCounts.find(dep); it != strRefCounts.end())
				{
					auto newRefCount = --it->second;
					if (newRefCount <= 0)
					{
						// requested/loading
						if ((mgr->Entries[dep].flags & 3) >= 2)
						{
							unrequest = true;
						}

						strRefCounts.erase(dep);
					}
				}
			}

			if (unrequest)
			{
				SetHandleDownloadWeight(mgr->Entries[dep].handle, 1);
			}
		}
	}

	return g_orig_fwEntity_Dtor(entity);
}

static HookFunction hookFunction([] ()
{
	OnCriticalGameFrame.Connect([]()
	{
		decltype(g_onCriticalFrameQueue)::value_type fn;

		while (g_onCriticalFrameQueue.try_pop(fn))
		{
			if (fn)
			{
				fn();
			}
		}
	});

	Hook_StreamingSema();

	OnMainGameFrame.Connect([]()
	{
		ProcessRemoval();
	});

	// redirect pgStreamer::Read for custom streaming reads
	{
		auto location = hook::get_pattern("45 8B ? 48 89 7C 24 28 48 89 44 24 20 E8", 13);
		hook::set_call(&g_origPgStreamerRead, location);
		hook::call(location, pgStreamerRead);
	}

	MH_Initialize();

	// rage::strStreamingLoader::CancelRequest hook (deprioritize canceled requests)
	{
		auto location = hook::get_pattern("B9 00 40 00 00 33 ED 48", (xbr::IsGameBuildOrGreater<2699>()) ? -0x21: -0x29);
		MH_CreateHook(location, CancelRequestWrap, (void**)&g_origCancelRequest);
		MH_EnableHook(location);
	}

	// rage::strStreamingInfoManager::SetObjectToLoading call for pending (canceled prior?) requests
	// in rage::strStreamingLoader::RequestStreamFiles to reprioritize
	{
		auto location = hook::get_pattern("83 F8 FF 74 72 48 8D 0D ? ? ? ? E8 ? ? ? ? 48", 12);
		hook::set_call(&g_origSetToLoading, location);
		hook::call(location, SetToLoadingWrap);
	}

	// parallelize streaming (force 'disable parallel streaming' flag off)
	hook::put<uint8_t>(hook::get_pattern("C0 C6 05 ? ? ? ? 01 44 88 35", 7), 0);

	// increase max streaming request threshold
	{
		auto location = hook::get_pattern<char>("74 13 C7 05 ? ? ? ? 01 00 00 00 C6 05", -0x3A);
		//hook::put<uint32_t>(hook::get_address<int*>(location + 0x24), 127);
		//hook::put<uint32_t>(location + 0x55, 100);

		// rate limit
		hook::put<uint32_t>(hook::get_address<int*>(location + 0x11F), 0x1FFFFF);

		// LSN tolerance
		//hook::put<uint32_t>(hook::get_address<int*>(location + 0x97), 0x00000000);
		//hook::put<uint32_t>(hook::get_address<int*>(location + 0x97) + 1, 0x00000000);

		// LSN for rawstreamer HDD<->DVD swap (no more 0x40000000 bit)
		hook::jump(hook::get_pattern("0F B7 C2 0F BA E8 1E C3"), NoLSN);
	}

	// don't adhere to some (broken?) streaming time limit
	hook::nop(hook::get_pattern("0F 2F C6 73 2D", 3), 2);

	// entity setmodelid/destructor for tracking entity unload
	{
		// CEntity::SetModelId
		// E8 ? ? ? ? 44 8B 0B 48 8D 4C 24 - 0x18
		auto location = hook::get_pattern("E8 ? ? ? ? 44 8B 0B 48 8D 4C 24", -0x18);
		MH_CreateHook(location, CEntity_SetModelIdWrap, (void**)&g_origSetModelId);
		MH_EnableHook(location);
	}

	{
		// rage::fwEntity::~fwEntity
		auto location = hook::get_pattern("E8 ? ? ? ? 48 8B 4B 48 48 85 C9 74 0A", -0x21);
		MH_CreateHook(location, fwEntity_DtorWrap, (void**)&g_orig_fwEntity_Dtor);
		MH_EnableHook(location);
	}

	OnKillNetworkDone.Connect([]()
	{
		std::unique_lock _(strRefCountsMutex);
		strRefCounts.clear();
	});
});
