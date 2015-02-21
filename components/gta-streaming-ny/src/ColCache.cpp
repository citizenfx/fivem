/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <fiDevice.h>
#include <Pool.h>

#include <unordered_set>

using rage::fiDevice;

struct ColPoolItem
{
	char pad[16];
	float floaters[7];
};

static fiDevice* g_colCacheDevice;
static uint32_t g_colCacheHandle = 0xFFFFFFFF;

static std::map<uint32_t, uint32_t> g_streamHashes;

static std::unordered_map<uint16_t, uint32_t> staticBoundHashesReverse;
static std::unordered_map<uint16_t, uint32_t> physBoundHashesReverse;

static std::unordered_set<int> g_isCachedSet;

void LogCollisionBuilding(uint16_t colID, void* building)
{
	if (g_colCacheHandle != 0xFFFFFFFF)
	{
		if (g_isCachedSet.find((*(int*)0xF3F224 << 24) | colID) == g_isCachedSet.end())
		{
			uint16_t zero = 0;
			uint32_t colHash = staticBoundHashesReverse[colID];

			g_colCacheDevice->write(g_colCacheHandle, &zero, sizeof(zero));
			g_colCacheDevice->write(g_colCacheHandle, &colHash, sizeof(colHash));

			CPool* colPool = *(CPool**)(0x16D7028);
			ColPoolItem* item = colPool->GetAt<ColPoolItem>(colID);

			g_colCacheDevice->write(g_colCacheHandle, item->floaters, sizeof(item->floaters));
		}
	}
}

struct CollisionShape
{
	uint32_t vtable;
	char data[168];
};

DWORD CreateStaticCollisionBuildingHook_ret = 0xC09726;

void __declspec(naked) CreateStaticCollisionBuildingHook()
{
	__asm
	{
		mov eax, [esp + 4h]
		mov ecx, [esp + 8h]

		push eax
		push ecx

		call LogCollisionBuilding

		add esp, 8h

		push ebp
		mov ebp, esp
		and esp, 0FFFFFFF0h

		mov eax, CreateStaticCollisionBuildingHook_ret
		jmp eax
	}
}

void SetDynamicCollisionDataHook(CollisionShape* shape, uint32_t modelHash, uint32_t* colIndex)
{
	if (g_colCacheHandle != 0xFFFFFFFF)
	{
		if (g_isCachedSet.find((*(int*)0xF2AAA0 << 24) | *(uint16_t*)colIndex) == g_isCachedSet.end())
		{
			uint16_t zero = 1;
			uint32_t colHash = physBoundHashesReverse[*(uint16_t*)colIndex];

			g_colCacheDevice->write(g_colCacheHandle, &zero, sizeof(zero));
			g_colCacheDevice->write(g_colCacheHandle, &colHash, sizeof(colHash));

			g_colCacheDevice->write(g_colCacheHandle, &modelHash, sizeof(modelHash));
			g_colCacheDevice->write(g_colCacheHandle, shape, 172);
		}
	}

	((void(*)(CollisionShape*, uint32_t, uint32_t*))(0x96FD00))(shape, modelHash, colIndex);
}

class CBaseModelInfo
{
public:
	virtual ~CBaseModelInfo() = 0;

	void SetBoundsFromShape(CollisionShape* shape);

	char m_pad[76];
	uint16_t m_pad2;
	uint16_t m_colIndex;
};

DWORD SetBoundsFromShape_loc = 0x98E850;
DWORD GetModelInfo_loc = 0x98AAE0;

__declspec(naked) void CBaseModelInfo::SetBoundsFromShape(CollisionShape* shape)
{
	__asm
	{
		mov eax, SetBoundsFromShape_loc
		jmp eax
	}
}

__declspec(naked) CBaseModelInfo* GetModelInfo(uint32_t hash, bool a2)
{
	__asm
	{
		mov eax, GetModelInfo_loc
		jmp eax
	}
}

std::string GetStreamName(int idx, int typeIdx);

static uint32_t WRAPPER NatHash(const char* str) { EAXJMP(0x7BDBF0); }

void RegisterWithColCache(const std::string& extn, int extnIndex, uint32_t hash)
{
	int hashIdx = (extn == "wbn") ? 0 : 1;

	g_streamHashes[(hashIdx << 16) | extnIndex] = hash;
}

// alter the hash in case of a resource-streamed file
static uint32_t AlterHash(int hashIdx, int typeIdx, uint32_t oldHash)
{
	auto it = g_streamHashes.find((hashIdx << 16) | typeIdx);

	if (it != g_streamHashes.end())
	{
		oldHash ^= it->second;
		oldHash |= 0x80000000;
	}

	return oldHash;
}

static int IsStreamingModuleItemCached(int itemIdx, int moduleIdx)
{
	return g_isCachedSet.find((moduleIdx << 24) | itemIdx) == g_isCachedSet.end();
}

void PreloadCollisions()
{
	if (g_colCacheHandle != 0xFFFFFFFF)
	{
		g_colCacheDevice->close(g_colCacheHandle);

		g_colCacheHandle = 0xFFFFFFFF;
	}

	// load existing collision data
	fiDevice* device = fiDevice::GetDevice("rescache:/", true);
	assert(device && "no rescache:/ device in ColCache!");

	const char* cacheFileName = "rescache:/citizen_cache_w.dat";

	uint32_t devHandle = device->open(cacheFileName, true);

	staticBoundHashesReverse.clear();
	physBoundHashesReverse.clear();

	// generate a table of collision IDs to hashes

	// static
	std::unordered_map<uint32_t, uint16_t> staticBoundHashes;

	CPool* colPool = *(CPool**)(0x16D7028);
	for (int i = 0; i < colPool->GetCount(); i++)
	{
		uint32_t hash = NatHash(GetStreamName(i, *(int*)0xF3F224).c_str()) & 0x7FFFFFFF; // StaticBounds module

		hash = AlterHash(0, i, hash);

		staticBoundHashes[hash] = i;
		staticBoundHashesReverse[i] = hash;
	}

	// physical
	std::unordered_map<uint32_t, uint16_t> physBoundHashes;

	CPool* physPool = *(CPool**)(0x15E3698);
	for (int i = 0; i < physPool->GetCount(); i++)
	{
		uint32_t hash = NatHash(GetStreamName(i, *(int*)0xF2AAA0).c_str()) & 0x7FFFFFFF; // Physics module

		hash = AlterHash(1, i, hash);

		physBoundHashes[hash] = i;
		physBoundHashesReverse[i] = hash;
	}

	if (devHandle != 0xFFFFFFFF)
	{
		// load the collisions from the file
		uint16_t type;

		while (device->read(devHandle, &type, sizeof(type)) == sizeof(type))
		{
			uint32_t colHash;

			if (type == 0)
			{
				if (device->read(devHandle, &colHash, sizeof(colHash)) == sizeof(colHash))
				{
					auto it = staticBoundHashes.find(colHash);

					if (it != staticBoundHashes.end())
					{
						uint16_t colId = it->second;

						ColPoolItem* item = colPool->GetAt<ColPoolItem>(colId);
						device->read(devHandle, item->floaters, sizeof(item->floaters));
						
						g_isCachedSet.insert((*(int*)0xF3F224 << 24) | colId);
					}
					else
					{
						device->seek(devHandle, sizeof(((ColPoolItem*)0)->floaters), SEEK_CUR);
					}
				}
			}
			else if (type == 1)
			{
				CollisionShape shape;
				uint32_t modelHash;

				if (device->read(devHandle, &colHash, sizeof(colHash)) == sizeof(colHash))
				{
					if (device->read(devHandle, &modelHash, sizeof(modelHash)) == sizeof(modelHash))
					{
						if (device->read(devHandle, &shape, sizeof(shape)) == sizeof(shape))
						{
							shape.vtable = 0xEBB998;

							CBaseModelInfo* modelInfo = GetModelInfo(modelHash, false);

							if (modelInfo)
							{
								auto it = physBoundHashes.find(colHash);

								if (it != physBoundHashes.end())
								{
									modelInfo->m_colIndex = it->second;
									modelInfo->SetBoundsFromShape(&shape);

									g_isCachedSet.insert((*(int*)0xF2AAA0 << 24) | it->second);
								}
							}
						}
					}
				}
			}
		}

		device->close(devHandle);

		// reopen the file with a pointer to the end of the file
		devHandle = device->open(cacheFileName, false);

		device->seek(devHandle, device->fileLength(devHandle), SEEK_SET);
	}
	else
	{
		devHandle = device->create(cacheFileName);
	}

	// mark cache as in-use (so we'll check cache flags)
	hook::put<uint8_t>(0xF411C1, true);
	hook::put<uint8_t>(0xF411C2, true);

	// mark the colcache file in a global variable
	g_colCacheDevice = device;
	g_colCacheHandle = devHandle;

	// load the remaining collisions from disk (and we'll store them to cache if need be)
	((void(*)())0x9704A0)(); // physical
	((void(*)())0xC0A170)(); // static

	// close the handle to flush data
	g_colCacheDevice->close(g_colCacheHandle);

	g_colCacheHandle = 0xFFFFFFFF;
}

static HookFunction hookFunction([] ()
{
	hook::nop(0x8D8772, 5);
	hook::call(0x8D8777, PreloadCollisions);

	hook::jump(0xC09720, CreateStaticCollisionBuildingHook);

	hook::put(0x96FF48, SetDynamicCollisionDataHook);

	hook::jump(0x832E80, IsStreamingModuleItemCached);
});