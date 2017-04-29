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

#include <Error.h>

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

struct BlockMapGap
{
	char* start;
	size_t size;
};

struct BlockMapMeta
{
	size_t maxSizes[128];
	size_t realMaxSizes[128];

	std::vector<BlockMapGap> gapList[2];

	bool isPerformingFinalAllocation;

	std::list<std::tuple<void*, size_t, bool>> allocations;

	size_t baseMemorySize;
};

static std::unordered_map<BlockMap*, BlockMapMeta> g_allocationData;

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

	FatalError("Pointer %016llx not found in passed block map in pgStreamManager::ResolveFilePointer.", *(uintptr_t*)&ptr);
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
static __declspec(thread) std::set<pgPtrRepresentation*>* g_packPointerSet;

void pgStreamManager::BeginPacking(BlockMap* blockMap)
{
	if (g_packEntries != nullptr)
	{
		delete g_packEntries;
	}

	g_packEntries = new std::vector<std::tuple<pgPtrRepresentation*, bool, void*>>();
	g_packPointerSet = new std::set<pgPtrRepresentation*>();

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
	g_packPointerSet->insert(ptrRepresentation);
	g_packEntries->push_back(std::make_tuple(ptrRepresentation, physical, tag));
}

bool pgStreamManager::IsInBlockMap(void* ptr, BlockMap* blockMap, bool physical)
{
	if (blockMap == nullptr)
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

	// a non-resolving block map can contain anything, so handle that
	if (!allocInfo.isPerformingFinalAllocation)
	{
		for (auto& allocation : allocInfo.allocations)
		{
			char* startPtr = reinterpret_cast<char*>(std::get<0>(allocation));
			char* endPtr = startPtr + std::get<1>(allocation);

			if (ptr >= startPtr && ptr < endPtr)
			{
				return true;
			}
		}

		return false;
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
	FinalizeAllocations(g_packBlockMap);

	/*auto& packEntries = *g_packEntries;

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
	}*/

	delete g_packEntries;
	g_packEntries = nullptr;
}

void pgStreamManager::FinalizeAllocations(BlockMap* blockMap)
{
	// find an allocation block for this block map
	auto allocBlock = g_allocationData.find(blockMap);

	if (allocBlock == g_allocationData.end())
	{
		return;
	}

	auto& allocInfo = allocBlock->second;

	allocInfo.isPerformingFinalAllocation = true;

	auto& allocations = allocInfo.allocations;

	// get the first allocation (to not mess with it at any later stage)
	auto firstAlloc = allocations.front();
	allocations.pop_front();

	// sort the remaining allocations by size - the biggest goes first (to allow gap-based allocation of pages at a later time)
	std::vector<std::tuple<void*, size_t, bool>> sortedAllocations(allocations.begin(), allocations.end());

	std::sort(sortedAllocations.begin(), sortedAllocations.end(), [] (const auto& left, const auto& right)
	{
		if (std::get<bool>(left) == std::get<bool>(right))
		{
			return (std::get<size_t>(left) > std::get<size_t>(right));
		}

		return std::get<bool>(left) < std::get<bool>(right);
	});

	// configure an ideal allocation base - this could get messy; as we'll allocate/free the block map a *lot* of times
	std::vector<void*> curAllocatedPtrs;

	BlockMap* curBlockMap = nullptr;

	auto attemptAllocation = [&] (size_t base, bool physical)
	{
		// create a new block map and a metadata set for it
		auto bm = CreateBlockMap();
		bm->baseAllocationSize[physical] = base;

		auto allocBlock = g_allocationData.find(bm);
		auto& allocInfo = allocBlock->second;

		allocInfo.isPerformingFinalAllocation = true;
		allocInfo.baseMemorySize = base;

		std::vector<void*> allocatedPtrs;

		bool fitting = true;

		if (!physical)
		{
			void* ptr = Allocate(std::get<1>(firstAlloc), false, bm);

			if (ptr)
			{
				allocatedPtrs.push_back(ptr);
			}
		}

		for (auto& alloc : sortedAllocations)
		{
			if (std::get<bool>(alloc) == physical)
			{
				void* ptr = Allocate(std::get<1>(alloc), std::get<2>(alloc), bm);

				if (!ptr)
				{
					fitting = false;
					break;
				}

				allocatedPtrs.push_back(ptr);
			}
		}

		if (!fitting)
		{
			DeleteBlockMap(bm);

			bm = nullptr;
			allocatedPtrs.clear();
		}

		return std::pair<BlockMap*, std::vector<void*>>(bm, allocatedPtrs);
	};

	{
		BlockMap* blockMaps[2] = { nullptr, nullptr };
		std::vector<void*> ptrLists[2];

		while (blockMaps[1] == nullptr)
		{
			size_t bestTotalMemory = -1;

			bool physical = (blockMaps[0] != nullptr);
			BlockMap*& curBlockMapRef = *&blockMaps[physical];
			std::vector<void*>& curAllocatedPtrsRef = *&ptrLists[physical];

			for (int i = 0; i < 16; i++)
			{
				// attempt to allocate a block map set for the base
				size_t newBase = (1 << i) << 13;
				auto pair = attemptAllocation(newBase, physical);

				// if allocation failed, continue
				if (pair.first == nullptr)
				{
					continue;
				}

				// count the total in-memory size
				size_t memorySize = 0;

#ifdef RAGE_FORMATS_GAME_FIVE
				const int8_t maxMults[] = { 16, 8, 4, 2, 1 };
				const uint8_t maxCounts[] = { 1, 3, 15, 63, 127 };
#else
				const int8_t maxMults[] = { 1, -2, -4, -8, -16 };
				const uint8_t maxCounts[] = { 0x7F, 1, 1, 1, 1 };
#endif

				auto getSize = [&] (int first, int count)
				{
					int last = first + count;
					int curMult = 0;
					int curCount = 0;
					size_t lastSize = 0;

					size_t thisSize = 0;

					for (int j = first; j < last; j++)
					{
						thisSize += lastSize = pair.first->blocks[j].size;

						curCount++;

						if (curCount >= maxCounts[curMult])
						{
							curMult++;
							curCount = 0;
						}
					}

					if (lastSize > 0)
					{
						for (int j = _countof(maxMults) - 1; j >= 0; j--)
						{
							size_t nextSize = (maxMults[j] >= 0) ? (newBase * maxMults[j]) : (newBase / -maxMults[j]);

							if (lastSize <= nextSize)
							{
								thisSize += (nextSize - lastSize);
								break;
							}
						}
					}

					return thisSize;
				};

				memorySize += getSize(0, pair.first->virtualLen);
				memorySize += getSize(pair.first->virtualLen, pair.first->physicalLen);

				if (memorySize < bestTotalMemory)
				{
					if (curBlockMapRef)
					{
						DeleteBlockMap(curBlockMapRef);
					}

					curBlockMapRef = pair.first;
					curAllocatedPtrsRef = pair.second;

					bestTotalMemory = memorySize;
				}
				else
				{
					DeleteBlockMap(pair.first);
				}

				if (memorySize > bestTotalMemory)
				{
					// probably not going to get any better
					break;
				}
			}
		}

		// merge both blockmaps' block lists
		curBlockMap = CreateBlockMap();

		curBlockMap->baseAllocationSize[0] = blockMaps[0]->baseAllocationSize[0];
		curBlockMap->baseAllocationSize[1] = blockMaps[1]->baseAllocationSize[1];

		curBlockMap->virtualLen = blockMaps[0]->virtualLen;
		curBlockMap->physicalLen = blockMaps[1]->physicalLen;

		memcpy(&curBlockMap->blocks[0], &blockMaps[0]->blocks[0], curBlockMap->virtualLen * sizeof(BlockMap::BlockInfo));
		memcpy(&curBlockMap->blocks[curBlockMap->virtualLen], &blockMaps[1]->blocks[0], curBlockMap->physicalLen * sizeof(BlockMap::BlockInfo));

		// merge ptr lists
		curAllocatedPtrs.insert(curAllocatedPtrs.end(), ptrLists[0].begin(), ptrLists[0].end());
		curAllocatedPtrs.insert(curAllocatedPtrs.end(), ptrLists[1].begin(), ptrLists[1].end());

		// delete the temp block maps
		g_allocationData.erase(blockMaps[0]);
		g_allocationData.erase(blockMaps[1]);

		delete blockMaps[0];
		delete blockMaps[1];
	}

	// copy allocated data to its true new home
	//sortedAllocations.push_front(firstAlloc);
	std::vector<std::tuple<void*, size_t, bool>> fullAllocations(sortedAllocations.size() + 1);
	fullAllocations[0] = firstAlloc;

	std::copy(sortedAllocations.begin(), sortedAllocations.end(), fullAllocations.begin() + 1);

	sortedAllocations.clear();

	int i = 0;

	for (auto& alloc : fullAllocations)
	{
		char* startPtr = reinterpret_cast<char*>(std::get<0>(alloc));
		char* endPtr = startPtr + std::get<1>(alloc);

		char* newStartPtr = reinterpret_cast<char*>(curAllocatedPtrs[i]);
		i++;

		memcpy(newStartPtr, startPtr, std::get<1>(alloc));

		auto begin = g_packPointerSet->lower_bound((pgPtrRepresentation*)startPtr);
		auto end = g_packPointerSet->upper_bound((pgPtrRepresentation*)endPtr);

		for (auto it = begin; it != end; it++)
		{
			auto ptrLoc = reinterpret_cast<char*>(*it);
			ptrLoc = ptrLoc - startPtr + newStartPtr;

			auto rawPtr = reinterpret_cast<char**>(ptrLoc);
			auto ptr = reinterpret_cast<pgPtrRepresentation*>(ptrLoc);

			// find the new allocation block that relates the most to this pointer
			// TODO: this is a slow manual search for now, could be better?

			int j = 0;

			for (auto& intAlloc : fullAllocations)
			{
				char* startPtr = reinterpret_cast<char*>(std::get<0>(intAlloc));
				char* endPtr = startPtr + std::get<1>(intAlloc);

				char* newStartPtr = reinterpret_cast<char*>(curAllocatedPtrs[j]);
				j++;

				if (*rawPtr >= startPtr && *rawPtr < endPtr)
				{
					// calculate the new relative pointer
					*rawPtr = (*rawPtr - startPtr + newStartPtr);

					// find the allocation block this is supposed to be in
					for (int k = 0; k < (curBlockMap->physicalLen + curBlockMap->virtualLen); k++)
					{
						auto& block = curBlockMap->blocks[k];

						if (*rawPtr >= block.data && *rawPtr < ((char*)block.data + block.size))
						{
							char* rawPtrValue = *rawPtr;

							ptr->blockType = (k >= curBlockMap->virtualLen) ? 6 : 5;

							ptr->pointer = (uintptr_t)((rawPtrValue - (char*)block.data) + block.offset);

							break;
						}
					}

					break;
				}
			}
		}
	}

	// swap the block map?
	g_allocationData[blockMap] = std::move(g_allocationData[curBlockMap]);
	memcpy(blockMap, curBlockMap, sizeof(*curBlockMap));

	g_allocationData.erase(curBlockMap);

	delete curBlockMap;
}

void* pgStreamManager::Allocate(size_t size, bool isPhysical, BlockMap* blockMap)
{
#ifdef RAGE_FORMATS_GAME_FIVE
	const int8_t maxMults[] = { 16, 8, 4, 2, 1 };
	const uint8_t maxCounts[] = { 1, 3, 15, 63, 127 };
#else
	const int8_t maxMults[] = { 1, -2, -4, -8, -16 };
	const uint8_t maxCounts[] = { 0x7F, 1, 1, 1, 1 };
#endif

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

	// are we performing pre-allocation?
	if (!allocInfo.isPerformingFinalAllocation)
	{
		void* retPtr = malloc(size);

		allocInfo.allocations.push_back(std::tuple<void*, size_t, bool>(retPtr, size, isPhysical));

		return retPtr;
	}

	// start at the current offset thing
	int curBlock = (isPhysical) ? (blockMap->physicalLen + blockMap->virtualLen) : blockMap->virtualLen;
	
	if ((!isPhysical && blockMap->virtualLen > 0) || (isPhysical && blockMap->physicalLen > 0))
	{
		curBlock--;
	}

	auto& curBlockInfo = blockMap->blocks[curBlock];

	size = ((size % 16) == 0) ? size : (size + (16 - (size % 16)));

	// make sure the allocation can't straddle a native page boundary (unless it's bigger than 0x200 bytes)
	uint32_t base = allocInfo.baseMemorySize;//0x4000; // base * 16 is the largest allocation possible - TODO: provide a means for the user to set this/relocate bases (that'd cause all existing pages to be invalidated?)
	size_t allocOffset;

	auto padAlloc = [&] (size_t padSize)
	{
		char* pad = (char*)Allocate(padSize, false, (BlockMap*)0x8001);

		for (int i = 0; i < padSize; i++)
		{
			pad[i] = '1';
		}

		allocInfo.gapList[isPhysical].push_back(BlockMapGap{ pad, padSize });

		size_t curSize = curBlockInfo.offset + curBlockInfo.size;
		assert((curSize % base) == 0);

		return Allocate(size, isPhysical, (BlockMap*)0x8001);
	};

	if ((curBlockInfo.size + size) <= allocInfo.realMaxSizes[curBlock])
	{
		// try finding a gap to allocate ourselves into
		for (auto& gap : allocInfo.gapList[isPhysical])
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

		curBlockInfo.size += size;

		memset(newPtr, 0xCD, size);

		*(int*)((char*)newPtr + size) = 0xDEADC0DE;

		return newPtr;
	}

	// apparently we need to allocate a new block...

	// pad the page to the maximum size so paging does not break
	if ((!isPhysical && blockMap->virtualLen > 0) || (isPhysical && blockMap->physicalLen > 0))
	{
		char* pad = (char*)curBlockInfo.data + curBlockInfo.size;
		size_t padSize = allocInfo.realMaxSizes[curBlock] - curBlockInfo.size;

		memset(pad, '1', padSize);

		allocInfo.gapList[isPhysical].push_back(BlockMapGap{ pad, padSize });

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

	for (int i = (isPhysical ? blockMap->virtualLen : 0); i < blockMap->virtualLen + (isPhysical ? blockMap->physicalLen : 0); i++)
	{
		curCount++;

		if (curCount == maxCounts[curMult])
		{
			curMult++;
			curCount = 0;
		}
	}

	curMult = maxMults[curMult];

	size_t multipliedBase = (curMult >= 0) ? (base * curMult) : (base / -curMult);

	if (newSize > multipliedBase)
	{
		trace("Tried to allocate more data than the base allocation unit (%d is current maximum page size) allows for, and relocation is not currently supported. Try increasing the base page multiplier (current is %d).\n", base * curMult, base);

		return nullptr;
	}

#if RAGE_NATIVE_ARCHITECTURE
	//newBlockInfo.data = malloc(newSize + 4);
	newBlockInfo.data = malloc(multipliedBase + 4);
#else
#ifdef _M_AMD64
	newBlockInfo.data = nullptr;

	size_t ntsize = (curMult * base) + 4;

	if (!NT_SUCCESS(ZwAllocateVirtualMemory(GetCurrentProcess(), &newBlockInfo.data, 0xFFFFFFF, &ntsize, MEM_COMMIT, PAGE_READWRITE)))
	{
		FatalError("ZwAllocateVirtualMemory failed!");
	}
#else
	FatalError(__FUNCTION__ " alloc for x86 not implemented");
#endif
#endif

	memset(newBlockInfo.data, 0xCD, multipliedBase);

	allocInfo.realMaxSizes[newStart] = multipliedBase;

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

BlockMap* pgStreamManager::CreateBlockMap()
{
	auto newMap = new BlockMap();
	memset(newMap, 0, sizeof(BlockMap));

	BlockMapMeta meta = { 0 };
	g_allocationData[newMap] = meta;

	return newMap;
}

void pgStreamManager::DeleteBlockMap(BlockMap* blockMap)
{
	for (int i = 0; i < blockMap->physicalLen + blockMap->virtualLen; i++)
	{
#if RAGE_NATIVE_ARCHITECTURE
		free(blockMap->blocks[i].data);
#else
		FatalError("?!");
#endif
	}

	g_allocationData.erase(blockMap);
	delete blockMap;
}

BlockMap* pgStreamManager::BeginPacking()
{
	auto newMap = CreateBlockMap();
	
	BeginPacking(newMap);

	return newMap;
}
}
}