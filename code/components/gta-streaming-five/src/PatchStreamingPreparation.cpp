#include "StdInc.h"
#include "Hooking.h"

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

static int(*g_origHandleObjectLoad)(streaming::Manager*, int, int, int*, int, int, int);

static std::unordered_map<std::string, std::tuple<rage::fiDevice*, uint64_t, uint64_t>> g_handleMap;
static std::unordered_map<std::string, int> g_failures;

hook::cdecl_stub<rage::fiCollection*()> getRawStreamer([]()
{
	return hook::get_call(hook::get_pattern("48 8B D3 4C 8B 00 48 8B C8 41 FF 90 ? 01 00 00", -5));
});

static std::unordered_map<std::string, uint32_t> g_thisSeenRequests;
static std::unordered_set<std::string> g_erasureQueue;

static bool ProcessHandler(HANDLE sema, char* a1)
{
	bool isSignaled = WaitForSingleObject(sema, 0) == WAIT_OBJECT_0;

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

static void Hook_StreamingSema()
{
	{
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(rdx, rbx);

				mov(rax, (uint64_t)&ProcessHandler);
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

				mov(rax, (uint64_t)&ProcessHandler);
				jmp(rax);
			}
		} weirdStub;

		auto location = hook::get_pattern("48 8B 8C C6 18 0A 03 00 48", 12);
		hook::call(location, weirdStub.GetCode());
	}
}

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
	OPTICK_EVENT();

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
				OPTICK_EVENT("isCache");

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

						FatalError("Failed to request %s: %s. %s", fileName, vfs::GetLastError(vfs::GetDevice(fileName)), error);
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

struct datResourceChunk
{

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

	while (g_removalQueue.try_pop(r))
	{
		for (auto i : r)
		{
			// ClearRequiredFlag
			streaming::Manager::GetInstance()->ReleaseObject(i, 0xF1);

			// RemoveObject
			streaming::Manager::GetInstance()->ReleaseObject(i);
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

static void* (*g_origPgStreamerRead)(uint32_t handle, datResourceChunk* outChunks, int numChunks, int flags, void(*callback)(void*), void* userData, int streamerIdx, int streamerFlags, void* unk9, float unk10);

static void* pgStreamerRead(uint32_t handle, datResourceChunk* outChunks, int numChunks, int flags, void (*callback)(void*), void* userData, int streamerIdx, int streamerFlags, void* unk9, float unk10)
{
	rage::fiCollection* collection = nullptr;

	if ((handle >> 16) == 0)
	{
		collection = getRawStreamer();
	}

	if (collection)
	{
		char fileNameBuffer[1024];
		strcpy(fileNameBuffer, "CfxRequest");

		collection->GetEntryNameToBuffer(handle & 0xFFFF, fileNameBuffer, sizeof(fileNameBuffer));

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
			auto device = vfs::GetDevice(fileName);

			uint64_t ptr;
			auto deviceHandle = device->OpenBulk(fileName, &ptr);

			if (deviceHandle != INVALID_DEVICE_HANDLE)
			{
				RequestHandleExtension ext;
				ext.handle = deviceHandle;
				ext.onRead = [fileName, handle, outChunks, numChunks, flags, callback, userData, streamerIdx, streamerFlags, unk9, unk10](bool success, const std::string& error)
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

				device->ExtensionCtl(VFS_RCD_REQUEST_HANDLE, &ext, sizeof(ext));
			}

			static int hi;
			return &hi;
		}
	}

	return g_origPgStreamerRead(handle, outChunks, numChunks, flags, callback, userData, streamerIdx, streamerFlags, unk9, unk10);
}

static uint32_t NoLSN(void* streamer, uint16_t idx)
{
	return idx;
}

static HookFunction hookFunction([] ()
{
	Hook_StreamingSema();

	// dequeue GTA streaming request function
	//auto location = hook::get_pattern("89 7C 24 28 4C 8D 7C 24 40 89 44 24 20 E8", 13);
	//hook::set_call(&g_origHandleObjectLoad, location);
	//hook::call(location, HandleObjectLoadWrap);

	OnMainGameFrame.Connect([]()
	{
		ProcessRemoval();
		//ProcessErasure();
	});

	// redirect pgStreamer::Read for custom streaming reads
	{
		auto location = hook::get_pattern("45 8B CC 48 89 7C 24 28 48 89 44 24 20 E8", 13);
		hook::set_call(&g_origPgStreamerRead, location);
		hook::call(location, pgStreamerRead);
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
});
