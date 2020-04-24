#include "StdInc.h"
#include "Hooking.h"

#include "Streaming.h"
#include <fiCollectionWrapper.h>

#include <EntitySystem.h>

#include <Error.h>
#include <ICoreGameInit.h>

#include <nutsnbolts.h>

#include <MinHook.h>
#include <optick.h>

#include <unordered_map>
#include <unordered_set>

#include <VFSError.h>
#include <VFSManager.h>

static int(*g_origHandleObjectLoad)(streaming::Manager*, int, int, int*, int, int, int);

static std::unordered_map<std::string, std::tuple<rage::fiDevice*, uint64_t, uint64_t>> g_handleMap;
static std::unordered_map<std::string, int> g_failures;

hook::cdecl_stub<rage::fiCollection*()> getRawStreamer([]()
{
	return hook::get_call(hook::get_pattern("48 8B D3 4C 8B 00 48 8B C8 41 FF 90 ? 01 00 00", -5));
});

static std::unordered_map<std::string, uint32_t> g_thisSeenRequests;
static std::unordered_set<std::string> g_erasureQueue;

#include <mmsystem.h>

static void ProcessErasure()
{
	static uint32_t lastRanErasure;

	if ((timeGetTime() - lastRanErasure) < 500)
	{
		return;
	}

	lastRanErasure = timeGetTime();

	// this test isn't too accurate but it should at least erase abandoned handles from RCD
	{
		for (auto& request : g_thisSeenRequests)
		{
			if ((lastRanErasure - request.second) > 500)
			{
				auto it = g_handleMap.find(request.first);

				if (it != g_handleMap.end())
				{
					g_erasureQueue.insert(request.first);
				}
			}
			else
			{
				g_erasureQueue.erase(request.first);
			}
		}
	}

	// erasing these entries is deferred since closing an in-transit entry in ResourceCacheDevice will cause download corruption of sorts
	{
		for (auto it = g_erasureQueue.begin(); it != g_erasureQueue.end(); )
		{
			auto entry = *it;

			// should be removed from the erasure queue?
			bool erased = false;

			// get the handle data
			auto hIt = g_handleMap.find(entry);

			if (hIt != g_handleMap.end())
			{
				auto[device, handle, ptr] = hIt->second;

				// perform a status bulk read
				char readBuffer[2048];
				uint32_t numRead = device->ReadBulk(handle, ptr, readBuffer, 0xFFFFFFFD);

				// download completed?
				if (numRead != 0)
				{
					// erase the entry
					device->CloseBulk(handle);
					g_handleMap.erase(hIt);

					erased = true;
				}
			}
			else
			{
				// no longer relevant to us, remove from erasure queue
				erased = true;
			}

			// increase iterator if needed
			if (!erased)
			{
				it++;
			}
			else
			{
				it = g_erasureQueue.erase(it);
			}
		}
	}
}

static std::tuple<bool /* IsCache */, std::string /* FileName */, std::string /* FileNameBuffer */> GetCacheEntry(uint32_t strIdx)
{
	auto strReqMgr = streaming::Manager::GetInstance();
	auto entry = &strReqMgr->Entries[strIdx];
	auto pf = streaming::GetStreamingPackfileForEntry(entry);

	rage::fiCollection* collection = nullptr;

	if (pf)
	{
		collection = reinterpret_cast<rage::fiCollection*>(pf->packfile);
	}

	if (!collection && (entry->handle >> 16) == 0)
	{
		collection = getRawStreamer();
	}

	if (collection && collection == getRawStreamer())
	{
		char fileNameBuffer[1024];
		strcpy(fileNameBuffer, "CfxRequest");

		collection->GetEntryNameToBuffer(entry->handle & 0xFFFF, fileNameBuffer, sizeof(fileNameBuffer));

		bool isCache = false;
		std::string fileName;

		if (strncmp(fileNameBuffer, "cache:/", 7) == 0)
		{
			fileName = std::string("cache_nb:/") + &fileNameBuffer[7];
			isCache = true;
		}

		if (strncmp(fileNameBuffer, "compcache:/", 11) == 0)
		{
			fileName = std::string("compcache_nb:/") + &fileNameBuffer[11];
			isCache = true;
		}

		if (isCache)
		{
			return { isCache, fileName, fileNameBuffer };
		}
	}

	return { false, "", "" };
}

static int HandleObjectLoadWrap(streaming::Manager* streaming, int a2, int a3, int* requestsOut, int numRequestsOut, int a6, int a7)
{
	OPTICK_EVENT();

	int requests[1][256];
	int remainingRequests[] = { g_origHandleObjectLoad(streaming, a2, a3, requests[0], numRequestsOut, a6, a7) };

	int numCachePending = 0;

	auto outReqs = 0;

	for (int r = 0; r < std::size(remainingRequests); r++)
	{
		for (int i = remainingRequests[r] - 1; i >= 0; i--)
		{
			bool shouldRemove = false;
			int index = requests[r][i];

			auto entry = &streaming->Entries[index];

			auto [isCache, fileName, fileNameBuffer] = GetCacheEntry(index);

			if (isCache)
			{
				OPTICK_EVENT("isCache");

				numCachePending++;

				if (numCachePending < 20)
				{
					g_thisSeenRequests[fileName] = timeGetTime();

					rage::fiDevice* device;
					uint64_t handle;
					uint64_t ptr;

					if (g_handleMap.find(fileName) != g_handleMap.end())
					{
						std::tie(device, handle, ptr) = g_handleMap[fileName];
					}
					else
					{
						device = rage::fiDevice::GetDevice(fileName.c_str(), true);
						handle = device->OpenBulk(fileName.c_str(), &ptr);
					}

					if (handle != -1)
					{
						char readBuffer[2048];
						uint32_t numRead;

						{
							OPTICK_EVENT("readBulk");
							numRead = device->ReadBulk(handle, ptr, readBuffer, 0xFFFFFFFE);
						}

						bool killHandle = true;

						if (numRead == 0)
						{
							killHandle = false;
							shouldRemove = true;

							g_handleMap.insert({ fileName, { device, handle, ptr } });
						}
						else if (numRead == -1)
						{
							shouldRemove = true;

							std::string error;
							ICoreGameInit* init = Instance<ICoreGameInit>::Get();

							init->GetData("gta-core-five:loadCaller", &error);

							FatalError("Failed to request %s: %s / %s\n", fileName, vfs::GetLastError(vfs::GetDevice(fileName)), error);

							// release object so we don't even try requesting it anymore
							streaming->ReleaseObject(index);
						}

						if (killHandle)
						{
							device->CloseBulk(handle);
							g_handleMap.erase(fileName);
						}
					}
					else
					{
						shouldRemove = true;
					}
				}
				else
				{
					shouldRemove = true;
				}
			}

			if (!shouldRemove)
			{
				if (outReqs < numRequestsOut)
				{
					requestsOut[outReqs] = requests[r][i];
					outReqs++;
				}
			}
		}
	}

	return outReqs;
}

#define VFS_CANCEL_REQUEST 0x30002

struct CancelRequestExtension
{
	const char* fn; // in
};

static void(*g_orig_rage__strStreamingLoader__CancelRequest)(void* loader, uint32_t idx);

static void rage__strStreamingLoader__CancelRequest_Hook(void* loader, uint32_t idx)
{
	g_orig_rage__strStreamingLoader__CancelRequest(loader, idx);

	auto strReqMgr = streaming::Manager::GetInstance();

	auto [isCache, fileName, fileNameBuffer] = GetCacheEntry(idx);

	if (isCache)
	{
		// cancel logic starts here
		auto device = vfs::GetDevice(fileName);

		CancelRequestExtension ext;
		ext.fn = fileName.c_str();

		if (device.GetRef())
		{
			device->ExtensionCtl(VFS_CANCEL_REQUEST, &ext, sizeof(ext));
		}
	}
}

static void(*g_origRemoveRequest)(void* mgr, uint32_t idx);

static void RemoveRequestHook(void* mgr, uint32_t idx)
{
	g_origRemoveRequest(mgr, idx);

	auto [isCache, fileName, fileNameBuffer] = GetCacheEntry(idx);

	if (isCache)
	{
		// cancel logic starts here
		// TODO: maybe pause?
		auto device = vfs::GetDevice(fileName);

		CancelRequestExtension ext;
		ext.fn = fileName.c_str();

		if (device.GetRef())
		{
			device->ExtensionCtl(VFS_CANCEL_REQUEST, &ext, sizeof(ext));
		}
	}
}

static hook::cdecl_stub<void(const uint32_t&, int)> _setRequiredFlagForModelInfo([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 8A 86 9D 00 00 00 24 1F 3C 06"));
});

static hook::cdecl_stub<void(const uint32_t&, int)> _clearRequiredFlagForModelInfo([]()
{
	return hook::get_call(hook::get_pattern("48 8B F9 74 59 48 8B 49 20 E8 ? ? ? ? 48 8B C8", 0x55));
});

static hook::cdecl_stub<const char* (int, uint32_t)> _getHashFromNs([]()
{
	return hook::get_pattern("75 04 33 C0 EB 61 48 63 F9 48 8D 0D", -0x1A);
});

static int* g_archetypeManager_streamingId;
static bool(*g_origRequestArchetype)(fwArchetype* archetype, int bucket);

static bool RequestArchetypeHook(fwArchetype* archetype, int bucket)
{
	auto rv = g_origRequestArchetype(archetype, bucket);

	uint32_t miInfo = archetype->streamingIndex;
	_setRequiredFlagForModelInfo(miInfo, 128);

	return rv;
}

static void(*g_origRemoveEntityModelInfo)(fwEntity* entity);

static void RemoveEntityModelInfoHook(fwEntity* entity)
{
	if (entity->GetArchetype())
	{
		uint32_t miInfo = entity->GetArchetype()->streamingIndex;
		/*
		auto mgr = streaming::Manager::GetInstance();
		auto base = mgr->moduleMgr.GetStreamingModuleFromId(*g_archetypeManager_streamingId)->baseIdx;
		auto strIdx = base + (miInfo & 0xFFFF);

		if ((mgr->Entries[strIdx].flags & 3) >= 2)
		{
			auto name = _getHashFromNs(1, entity->GetArchetype()->hash);

			if (!name) name = "(null)";

			trace("pre-unload MI %08x %s\n", entity->GetArchetype()->hash, name);
		}
		*/

		_clearRequiredFlagForModelInfo(miInfo, 128);
	}

	g_origRemoveEntityModelInfo(entity);
}

static HookFunction hookFunction([] ()
{
	// dequeue GTA streaming request function
	auto location = hook::get_pattern("89 7C 24 28 4C 8D 7C 24 40 89 44 24 20 E8", 13);
	hook::set_call(&g_origHandleObjectLoad, location);
	hook::call(location, HandleObjectLoadWrap);

	OnMainGameFrame.Connect([]()
	{
		ProcessErasure();
	});

	// parallelize streaming (force 'disable parallel streaming' flag off)
	hook::put<uint8_t>(hook::get_pattern("C0 C6 05 ? ? ? ? 01 44 88 35", 7), 0);

	// don't adhere to some (broken?) streaming time limit
	hook::nop(hook::get_pattern("0F 2F C6 73 2D", 3), 2);

	MH_Initialize();
	MH_CreateHook(hook::get_call(hook::get_pattern("66 41 85 4C F6 06 48 8D", 15)), rage__strStreamingLoader__CancelRequest_Hook, (void**)&g_orig_rage__strStreamingLoader__CancelRequest);
	MH_CreateHook(hook::get_pattern("48 8B F9 74 59 48 8B 49 20 E8 ? ? ? ? 48 8B C8", -14), RemoveEntityModelInfoHook, (void**)&g_origRemoveEntityModelInfo);
	MH_EnableHook(MH_ALL_HOOKS);

	// requesting archetype from CSceneStreamer
	{
		auto location = hook::get_pattern<char>("75 4B 41 8B D7 49 8B C8 E8", 8);
		g_archetypeManager_streamingId = hook::get_address<int*>(hook::get_call(location) + 9);
		hook::set_call(&g_origRequestArchetype, location);
		hook::call(location, RequestArchetypeHook);
	}

	// RemoveRequest from RemoveObject
	{
		auto location = hook::get_pattern("41 83 FD 02 75 46 49 8B CF E8", 9);
		hook::set_call(&g_origRemoveRequest, location);
		hook::call(location, RemoveRequestHook);
	}

	//MH_Initialize();

	// queueing task on streamer
	//MH_CreateHook(hook::get_pattern("4D 69 ED 70 41 00 00 4C 03 E8", -0x82), QueueStreamerTaskWrap, (void**)&g_origQueueStreamerTask);

	//MH_EnableHook(MH_ALL_HOOKS);
});
