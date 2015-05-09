/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <pgBase.h>
#include <unordered_set>
#include <tuple>

#ifdef _WIN32
#include <winternl.h>

extern "C" NTSTATUS ZwAllocateVirtualMemory(
	_In_    HANDLE    ProcessHandle,
	_Inout_ PVOID     *BaseAddress,
	_In_    ULONG_PTR ZeroBits,
	_Inout_ PSIZE_T   RegionSize,
	_In_    ULONG     AllocationType,
	_In_    ULONG     Protect
	);


#pragma comment(lib, "ntdll.lib")
#endif


namespace rage
{
namespace RAGE_FORMATS_GAME
{
static __declspec(thread) BlockMap* g_currentBlockMap;

void* pgStreamManager::ResolveFilePointer(pgPtrRepresentation& ptr, BlockMap* blockMap /* = nullptr */)
{
	if (ptr.blockType == 0)
	{
		return nullptr;
	}

	if (!blockMap)
	{
		blockMap = g_currentBlockMap;
	}

	int startIdx = (ptr.blockType == 5) ? 0 : blockMap->virtualLen;
	int endIdx = (ptr.blockType == 5) ? blockMap->virtualLen : (blockMap->virtualLen + blockMap->physicalLen);

	for (int i = startIdx; i < endIdx; i++)
	{
		auto& block = blockMap->blocks[i];

		if (ptr.pointer >= block.offset && ptr.pointer < (block.offset + block.size))
		{
			return (char*)block.data + (ptr.pointer - block.offset);
		}
	}

	FatalError("Pointer %p not found in passed block map in pgStreamManager::ResolveFilePointer.", *(uintptr_t*)&ptr);
}

void pgStreamManager::SetBlockInfo(BlockMap* blockMap)
{
	g_currentBlockMap = blockMap;
}

static std::unordered_set<const void*> g_resolvedEntries;

bool pgStreamManager::IsResolved(const void* ptrAddress)
{
	return g_resolvedEntries.find(ptrAddress) != g_resolvedEntries.end();
}

void pgStreamManager::MarkResolved(const void* ptrAddress)
{
	g_resolvedEntries.insert(ptrAddress);
}

void pgStreamManager::UnmarkResolved(const void* ptrAddress)
{
	g_resolvedEntries.erase(ptrAddress);
}

static __declspec(thread) BlockMap* g_packBlockMap;
static __declspec(thread) std::vector<std::tuple<pgPtrRepresentation*, bool, void*>>* g_packEntries;

void pgStreamManager::BeginPacking(BlockMap* blockMap)
{
	if (g_packEntries != nullptr)
	{
		delete g_packEntries;
	}

	g_packEntries = new std::vector<std::tuple<pgPtrRepresentation*, bool, void*>>();

	g_packBlockMap = blockMap;
}

void pgStreamManager::MarkToBePacked(pgPtrRepresentation* ptrRepresentation, bool physical, void* tag)
{
	if (!g_packBlockMap)
	{
		return;
	}

	// check if the block is in the block map
	auto inPtr = *(uintptr_t*)ptrRepresentation;

	if (!IsInBlockMap((void*)inPtr, g_packBlockMap, physical))
	{
		return;
	}

	// physical typically doesn't contain pointers to other physicals, so we use false here
	if (!IsInBlockMap(ptrRepresentation, g_packBlockMap, false))
	{
		return;
	}

	// add the block
	g_packEntries->push_back(std::make_tuple(ptrRepresentation, physical, tag));
}

bool pgStreamManager::IsInBlockMap(void* ptr, BlockMap* blockMap, bool physical)
{
	if (blockMap == nullptr)
	{
		blockMap = g_packBlockMap;
	}

	int startIndex = (!physical) ? 0 : blockMap->virtualLen;
	int endIndex = (!physical) ? blockMap->virtualLen : (blockMap->virtualLen + blockMap->physicalLen);

	uintptr_t inPtr = (uintptr_t)ptr;

	for (int i = startIndex; i < endIndex; i++)
	{
		auto& block = blockMap->blocks[i];
		auto offStart = (uintptr_t)block.data;
		auto offEnd = offStart + block.size;

		if (inPtr >= offStart && inPtr < offEnd)
		{
			return true;
		}
	}

	return false;
}

void pgStreamManager::EndPacking()
{
	auto& packEntries = *g_packEntries;

	for (auto& packPtr : packEntries)
	{
		uintptr_t inPtr = *(uintptr_t*)std::get<0>(packPtr);

		if (inPtr == 0)
		{
			continue;
		}

		bool physical = std::get<1>(packPtr);

		int startIndex = (!physical) ? 0 : g_packBlockMap->virtualLen;
		int endIndex = (!physical) ? g_packBlockMap->virtualLen : (g_packBlockMap->virtualLen + g_packBlockMap->physicalLen);

		bool valid = false;

		for (int i = startIndex; i < endIndex; i++)
		{
			auto& block = g_packBlockMap->blocks[i];
			auto offStart = (uintptr_t)block.data;
			auto offEnd = offStart + block.size;
			
			if (inPtr >= offStart && inPtr < offEnd)
			{
				std::get<0>(packPtr)->blockType = (!physical) ? 5 : 6;
				std::get<0>(packPtr)->pointer = (inPtr - offStart) + block.offset;

				valid = true;

				break;
			}
		}

		if (!valid)
		{
			FatalError("Stray pointer in pgPtr: %p %p", inPtr, std::get<2>(packPtr));
		}
	}

	delete g_packEntries;
	g_packEntries = nullptr;
}

struct BlockMapGap
{
	char* start;
	size_t size;
};

struct BlockMapMeta
{
	size_t maxSizes[128];
	size_t realMaxSizes[128];

	std::vector<BlockMapGap> gapList;
};

static std::unordered_map<BlockMap*, BlockMapMeta> g_allocationData;

void* pgStreamManager::Allocate(size_t size, bool isPhysical, BlockMap* blockMap)
{
	const uint8_t maxMults[] = { 16, 8, 4, 2, 1 };
	const uint8_t maxCounts[] = { 1, 3, 15, 63, 127 };

	// is this the packing block map?
	void* oldBlockMap = blockMap;

	if (!blockMap || (uintptr_t)blockMap <= 0xF000)
	{
		blockMap = g_packBlockMap;
	}

	// find an allocation block for this block map
	auto allocBlock = g_allocationData.find(blockMap);

	if (allocBlock == g_allocationData.end())
	{
		return nullptr;
	}

	auto& allocInfo = allocBlock->second;

	// start at the current offset thing
	int curBlock = (isPhysical) ? (blockMap->physicalLen + blockMap->virtualLen) : blockMap->virtualLen;
	
	if ((!isPhysical && blockMap->virtualLen > 0) || (isPhysical && blockMap->physicalLen > 0))
	{
		curBlock--;
	}

	auto& curBlockInfo = blockMap->blocks[curBlock];

	size = ((size % 16) == 0) ? size : (size + (16 - (size % 16)));

	// make sure the allocation can't straddle a native page boundary (unless it's bigger than 0x200 bytes)
	uint32_t base = 0x4000; // base * 16 is the largest allocation possible - TODO: provide a means for the user to set this/relocate bases (that'd cause all existing pages to be invalidated?)
	size_t allocOffset;

	auto padAlloc = [&] (size_t padSize)
	{
		char* pad = (char*)Allocate(padSize, false, (BlockMap*)0x8001);

		for (int i = 0; i < padSize; i++)
		{
			pad[i] = '1';
		}

		allocInfo.gapList.push_back(BlockMapGap{ pad, padSize });

		size_t curSize = curBlockInfo.offset + curBlockInfo.size;
		assert((curSize % base) == 0);

		return Allocate(size, isPhysical, (BlockMap*)0x8001);
	};

	/*if (size < base/* && oldBlockMap != (BlockMap*)1* /)
	{
		size_t curSize = curBlockInfo.offset + curBlockInfo.size;
		size_t nextBoundary = curSize + (base - (curSize % base));

		/*if (nextBoundary >= 0x62000 && nextBoundary <= 0x66000)
		{
			__debugbreak();
		}* /

		if ((curSize % base) != 0)
		{
			// get the nearest 0x2000 boundary (as pages are always beyond this)
			if ((curSize + size) > nextBoundary)
			{
				size_t padSize = nextBoundary - curSize;
				return padAlloc(padSize);
			}
		}
	}
	else if ((uintptr_t)oldBlockMap < 0x8000) // hacky way of preventing reentrancy
	{
		size_t curSize = curBlockInfo.offset + curBlockInfo.size;
		size_t nextBoundary = curSize + (base - (curSize % base));

		if ((curSize % base) != 0)
		{
			return padAlloc(nextBoundary - curSize);
		}
	}*/

	if ((curBlockInfo.size + size) <= allocInfo.realMaxSizes[curBlock])
	{
		// try finding a gap to allocate ourselves into
		for (auto& gap : allocInfo.gapList)
		{
			if (gap.size >= size)
			{
				void* ptr = gap.start;

				gap.size -= size;
				gap.start += size;

				return ptr;
			}
		}

		if ((curBlockInfo.size + size) > allocInfo.maxSizes[curBlock])
		{
			allocInfo.maxSizes[curBlock] *= 2;
		}

		void* newPtr = (char*)curBlockInfo.data + curBlockInfo.size;

		/*if (oldSize % 16)
		{
			newPtr = (void*)(((uintptr_t)newPtr + 16) & ~(uintptr_t)0xF);
		}*/

		curBlockInfo.size += size;

		memset(newPtr, 0xCD, size);

		*(int*)((char*)newPtr + size) = 0xDEADC0DE;

		return newPtr;
	}

	// apparently we need to allocate a new block...

	// pad the page to the maximum size so paging does not break
	if (blockMap->virtualLen > 0)
	{
		char* pad = (char*)curBlockInfo.data + curBlockInfo.size;
		size_t padSize = allocInfo.realMaxSizes[curBlock] - curBlockInfo.size;

		memset(pad, '1', padSize);

		allocInfo.gapList.push_back(BlockMapGap{ pad, padSize });

		curBlockInfo.size += padSize;
	}

	// move along all physical pages if we need a new virtual page
	if (!isPhysical && blockMap->physicalLen > 0)
	{
		int moveStart = blockMap->virtualLen;
		int moveEnd = moveStart + blockMap->physicalLen;

		for (int i = (moveEnd - 1); i >= moveStart; i--)
		{
			if ((i + 1) >= _countof(blockMap->blocks))
			{
				FatalError("ran out of blocks in pgBase.cpp");
			}

			blockMap->blocks[i + 1] = blockMap->blocks[i];
		}
	}

	// determine the new offset
	int newOffset = 0;
	size_t newSize = base;//64 * 1024;

	if (isPhysical)
	{
		int lastIdx;

		// find the last physical block
		if (blockMap->physicalLen > 0)
		{
			lastIdx = blockMap->virtualLen + blockMap->physicalLen - 1;

			auto& lastBlock = blockMap->blocks[lastIdx];
	
			newOffset = lastBlock.offset + lastBlock.size;

			//newSize = allocInfo.maxSizes[lastIdx] * 2;
		}
		else
		{
			newOffset = 0;
		}
	}
	else
	{
		int lastIdx;

		// find the last physical block
		if (blockMap->virtualLen > 0)
		{
			lastIdx = blockMap->virtualLen - 1;

			auto& lastBlock = blockMap->blocks[lastIdx];

			newOffset = lastBlock.offset + lastBlock.size;

			//newSize = allocInfo.maxSizes[lastIdx] * 2;
		}
		else
		{
			newOffset = 0;
		}
	}
	
	while (size >= newSize)
	{
		newSize *= 2;
	}

	// determine the new block index
	int newStart = (!isPhysical) ? blockMap->virtualLen : (blockMap->virtualLen + blockMap->physicalLen);
	auto& newBlockInfo = blockMap->blocks[newStart];

	allocInfo.maxSizes[newStart] = newSize;
	
	// calculate the real maximum size
	int curMult = 0;
	int curCount = 0;

	for (int i = 0; i < blockMap->virtualLen; i++)
	{
		curCount++;

		if (curCount == maxCounts[curMult])
		{
			curMult++;
			curCount = 0;
		}
	}

	curMult = maxMults[curMult];

	if (newSize > (base * curMult))
	{
		FatalError("Tried to allocate more data than the base allocation unit (%d is current maximum page size) allows for, and relocation is not currently supported. Try increasing the base page multiplier (current is %d).", base * curMult, base);
	}

#if RAGE_NATIVE_ARCHITECTURE
	//newBlockInfo.data = malloc(newSize + 4);
	newBlockInfo.data = malloc((curMult * base) + 4);
#else
	newBlockInfo.data = nullptr;

	size_t ntsize = (curMult * base) + 4;

	if (!NT_SUCCESS(ZwAllocateVirtualMemory(GetCurrentProcess(), &newBlockInfo.data, 0xFFFFFFF, &ntsize, MEM_COMMIT, PAGE_READWRITE)))
	{
		FatalError("ZwAllocateVirtualMemory failed!");
	}
#endif

	memset(newBlockInfo.data, 0xCD, base * curMult);

	allocInfo.realMaxSizes[newStart] = base * curMult;

	newBlockInfo.offset = newOffset;
	newBlockInfo.size = size;

	*(int*)((char*)newBlockInfo.data + size) = 0xDEADC0DE;

	if (isPhysical)
	{
		blockMap->physicalLen++;
	}
	else
	{
		blockMap->virtualLen++;
	}

	return newBlockInfo.data;
}

BlockMap* pgStreamManager::GetBlockMap()
{
	return g_packBlockMap;
}

void* pgStreamManager::AllocatePlacement(size_t size, void* hintPtr, bool isPhysical, BlockMap* blockMap)
{
	if (IsInBlockMap(hintPtr, blockMap, isPhysical))
	{
		return hintPtr;
	}

	return Allocate(size, isPhysical, blockMap);
}

BlockMap* pgStreamManager::BeginPacking()
{
	auto newMap = new BlockMap();
	memset(newMap, 0, sizeof(BlockMap));

	BlockMapMeta meta = { 0 };
	g_allocationData[newMap] = meta;

	pgStreamManager::BeginPacking(newMap);

	return newMap;
}
}
}