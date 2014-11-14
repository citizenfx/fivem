#include "StdInc.h"
#include "BoundStreaming.h"
#include "Streaming.h"
#include "Pool.h"
#include "IdeStore.h"

struct BlockMap
{
	uint16_t virtualLen;
	uint16_t physicalLen;
	int data[384];
};

struct NextBound
{
	std::string filename;
	uint32_t size;
	uint32_t rscFlags;
	uint32_t rscVersion;
};

static std::unordered_map<uint16_t, NextBound> g_boundsMap;
static std::map<uint16_t, int> g_refCount;
//static std::map<uint16_t, uint32_t> g_resourceFlags;
static std::map<uint16_t, BlockMap> g_blockMaps;

void PreLoadImgArchivesTail();

uint32_t BoundStreaming::RegisterBound(const char* filename, uint32_t size, uint32_t rscFlags, uint32_t rscVersion)
{
	char tempPath[256];
	strcpy_s(tempPath, filename);
	strrchr(tempPath, '.')[0] = '\0';

	NextBound bound;

	uint16_t idx = ((uint16_t(*)(const char*))(0xC0A3D0))(tempPath);

	bound.filename = va("stream:/%u", 0x80000000 | idx);
	bound.size = size;
	bound.rscFlags = rscFlags;
	bound.rscVersion = rscVersion;

	g_boundsMap[idx] = bound;

	trace("registered bound %s\n", filename);

	return idx;
}

typedef void* (__cdecl* LoadResource_t)(const char* filename, const char* extension, int resourceType, BlockMap* blockMap, int* a5);
typedef bool(__stdcall* MakeSpaceFor_t)(uint32_t rscVersion);

struct RequestMeta
{
	int streamHandle;
	int pad;
	HANDLE hSemaphore;
	//StreamingItem* item;
	uint32_t rscFlags;
};

void EnqueueImStreamRequest(BlockMap* blockMap, RequestMeta* meta, int a3, int a4)
{
	__asm
	{
		push edi
		push esi
		mov esi, meta
		mov edi, blockMap

		push a4
		push a3
		
		mov eax, 5BB490h
		call eax

		add esp, 8h

		pop esi
		pop edi
	}
}

struct ColRequest
{
	BlockMap blockMap;
	HANDLE semaphore;
	uint32_t id;
	uint32_t streamHandle;
};

static unsigned int g_ongoingRequests;
static ColRequest g_requests[6];

ColRequest* AllocateColRequest()
{
	for (int i = 0; i < _countof(g_requests); i++)
	{
		if (g_requests[i].id == 0)
		{
			return &g_requests[i];
		}
	}

	return nullptr;
}

void BoundStreaming::Process()
{
	CIdeStore::Process();

	for (int i = 0; i < _countof(g_requests); i++)
	{
		if (g_requests[i].id)
		{
			if (WaitForSingleObject(g_requests[i].semaphore, 0) == WAIT_OBJECT_0)
			{
				//void* memory = ((LoadResource_t)(gameModule + 0x5BBA20))(g_boundsMap[id].c_str(), "#bn", 32, &blockMap, &a5);

				CloseHandle(g_requests[i].semaphore);

				((void(*)(int))0x5B1570)(g_requests[i].streamHandle);

				uint32_t streamingUsage3 = *(int*)(0xF21C84);

				trace("loaded %d\n", g_requests[i].id - 1);

				//char debugMsg[512];
				//sprintf(debugMsg, "Loading collision %s (diff 1-2 %d, diff 2-3 %d)...\n", g_boundsMap[id].filename.c_str(), streamingUsage2 - streamingUsage1, streamingUsage3 - streamingUsage2);
				//OutputDebugString(debugMsg);

				// store virtual count as for some reason the game deletes this
				uint16_t virtualLen = g_requests[i].blockMap.virtualLen;

				// relocate
				((void(*)(int, BlockMap*, int))(0xC0A5B0))(0, &g_requests[i].blockMap, 0);

				// put virtual count back
				g_requests[i].blockMap.virtualLen = virtualLen;

				// set data
				//((void(*)(int, void*))(gameModule + 0xC09F70))(id, memory);
				((void(*)(int, void*))(0xC09F70))(g_requests[i].id - 1, (void*)g_requests[i].blockMap.data[1]);

				//((void(*)(int))0x5B1570)(g_requests[i].streamHandle);

				g_refCount[g_requests[i].id - 1]++;

				g_requests[i].blockMap.virtualLen = 0;
				g_requests[i].id = 0;

				InterlockedDecrement(&g_ongoingRequests);
			}
		}
	}
}

void BoundStreaming::LoadCollision(int id, int priority)
{
	/*int blockMap[385];
	blockMap[0] = 0;*/

	ColRequest* reqPtr = AllocateColRequest();

	if (!reqPtr)
	{
		return;
	}

	ColRequest& req = *reqPtr;
	req.blockMap.physicalLen = 0;
	req.blockMap.virtualLen = 0;

	int a5;

	if (g_refCount[id] == 0)
	{
		uint32_t streamingUsage1 = *(int*)(0xF21C84);

		bool isSpace = ((MakeSpaceFor_t)(0xAC1FF0))(g_boundsMap[id].rscFlags);

		//bool isSpace = true;

		uint32_t streamingUsage2 = *(int*)(0xF21C84);

		if (isSpace)
		{
			if (g_nextStreamingItem == -1)
			{
				trace("loading collision %s got a neg1\n", g_boundsMap[id].filename.c_str());

				return;
			}

			trace("loading collision %s (id %d)\n", g_boundsMap[id].filename.c_str(), id);

			auto& bound = g_boundsMap[id];

			if (g_blockMaps[id].virtualLen > 0)
			{
				__asm int 3
			}
			
			StreamingItem* item = &g_streamingItems[g_nextStreamingItem];
			item->blockSize = bound.size;
			item->device = (rage::fiDevice*)0xF21CA8;
			item->flags = bound.rscFlags;
			//item->fileName = bound.filename.c_str();

			// as the 'request free' function will attempt to free the filename
			item->fileName = (char*)rage::GetAllocator()->allocate(bound.filename.length() + 1, 16, 0);
			strcpy(item->fileName, bound.filename.c_str());

			g_nextStreamingItem = item->streamCounter;
			item->streamCounter = 0;

			RequestMeta meta;
			meta.pad = 0;
			meta.streamHandle = item->handle;
			meta.rscFlags = bound.rscFlags;
			meta.hSemaphore = (HANDLE)-1;

			InterlockedIncrement(&g_ongoingRequests);

			if (req.blockMap.virtualLen > 0)
			{
				__asm int 3
			}

			EnqueueImStreamRequest(&req.blockMap, &meta, 0, 0);

			req.semaphore = meta.hSemaphore;
			req.id = id + 1;
			req.streamHandle = meta.streamHandle;

			memcpy(&g_blockMaps[id], &req.blockMap, sizeof(BlockMap));

			g_refCount[id]++;

			// assign streaming memory used(?!)
			//uint32_t flags = g_resourceFlags[id];

			//*(int*)(gameModule + 0xF21C84) += ((flags & 0x7FF) << (int)(((flags >> 11) & 0xF) + 8));
			//*(int*)(gameModule + 0xF21C88) += ((flags & 0x7FF) << (int)(((flags >> 11) & 0xF) + 8));
		}
		else
		{
			char debugMsg[512];
			sprintf(debugMsg, "Wanted to load collision %s, but no space!\n", g_boundsMap[id].filename.c_str());
			OutputDebugStringA(debugMsg);

			return;
		}
	}
}

void __stdcall BoundStreaming::LoadAllObjectsTail(int)
{
	while (g_ongoingRequests == 1)
	{
		Sleep(5);

		Process();
	}

	CIdeStore::LoadAllRequestedArchetypes();
}

void BoundStreaming::ReleaseCollision(int id)
{
	if (g_refCount[id] == 1)
	{
		return;
	}

	CPool* collisionPool = *(CPool**)(0x16D7028);

	g_refCount[id] -= 2;

	if (g_refCount[id] <= 0)
	{
		g_refCount[id] = 0;

		if (collisionPool->GetAt<void>(id) == nullptr)
		{
			return;
		}

		//char debugMsg[512];
		//sprintf(debugMsg, "Unloading %s...\n", g_boundsMap[id].c_str());
		//OutputDebugString(debugMsg);
		trace("unloading %s...\n", g_boundsMap[id].filename.c_str());

		__try
		{
			// release data
			((void(*)(int))(0xC096A0))(id);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			trace("Exception releasing collision.\n");
		}

		BlockMap* blockMap = &g_blockMaps[id];

		((void(*)(BlockMap*))(0x5BB420))(blockMap);

		blockMap->virtualLen = 0;

		//*(uint32_t*)(gameMod`ule + 0xF21C8C) -= ((flags >> 15) & 0x7FF) << (int)(((flags >> 26) & 0xF) + 8);
	}
}