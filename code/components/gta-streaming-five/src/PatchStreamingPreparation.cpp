#include "StdInc.h"
#include "Hooking.h"

#include "Streaming.h"
#include <fiCollectionWrapper.h>

#include <Error.h>
#include <ICoreGameInit.h>

#include <nutsnbolts.h>

#include <Brofiler.h>

#include <unordered_map>
#include <unordered_set>

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

static int HandleObjectLoadWrap(streaming::Manager* streaming, int a2, int a3, int* requests, int a5, int a6, int a7)
{
	PROFILE;

	int remainingRequests = g_origHandleObjectLoad(streaming, a2, a3, requests, a5, a6, a7);

	for (int i = remainingRequests - 1; i >= 0; i--)
	{
		bool shouldRemove = false;
		int index = requests[i];

		auto entry = &streaming->Entries[index];
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

		if (collection)
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
				BROFILER_EVENT("isCache");

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
						BROFILER_EVENT("readBulk");
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

						g_failures[fileName]++;

						if (g_failures[fileName] > 3)
						{
							std::string error;
							ICoreGameInit* init = Instance<ICoreGameInit>::Get();

							init->GetData("gta-core-five:loadCaller", &error);

							FatalError("Failed to request %s. %s", error);
						}
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
		}

		if (shouldRemove)
		{
			memmove(&requests[i], &requests[i + 1], (remainingRequests - i - 1) * sizeof(int));
			remainingRequests -= 1;
		}
	}

	return remainingRequests;
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

	//MH_Initialize();

	// queueing task on streamer
	//MH_CreateHook(hook::get_pattern("4D 69 ED 70 41 00 00 4C 03 E8", -0x82), QueueStreamerTaskWrap, (void**)&g_origQueueStreamerTask);

	//MH_EnableHook(MH_ALL_HOOKS);
});
