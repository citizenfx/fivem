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

namespace rage
{
namespace RAGE_FORMATS_GAME
{
static __declspec(thread) BlockMap* g_currentBlockMap;

void* pgStreamManager::ResolveFilePointer(const pgPtrRepresentation& ptr, BlockMap* blockMap /* = nullptr */)
{
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

struct BlockMapMeta
{
	size_t maxSizes[128];
};

static std::unordered_map<BlockMap*, BlockMapMeta> g_allocationData;

void* pgStreamManager::Allocate(size_t size, bool isPhysical, BlockMap* blockMap)
{
	// is this the packing block map?
	if (!blockMap)
	{
		blockMap = g_packBlockMap;
	}

	// find an allocation block for this block map
	auto& allocBlock = g_allocationData.find(blockMap);

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

	if ((curBlockInfo.size + size) <= allocInfo.maxSizes[curBlock])
	{
		void* newPtr = (char*)curBlockInfo.data + curBlockInfo.size;
		curBlockInfo.size += size;

		memset(newPtr, 0xCD, size);

		*(int*)((char*)newPtr + size) = 0xDEADC0DE;

		return newPtr;
	}

	// apparently we need to allocate a new block...

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
	size_t newSize = 64 * 1024;

	if (isPhysical)
	{
		int lastIdx;

		// find the last physical block
		if (blockMap->physicalLen > 0)
		{
			lastIdx = blockMap->virtualLen + blockMap->physicalLen - 1;

			auto& lastBlock = blockMap->blocks[lastIdx];
	
			newOffset = lastBlock.offset + lastBlock.size;

			newSize = allocInfo.maxSizes[lastIdx] * 2;
		}
		else
		{
			newOffset = 0;
		}
	}

	// determine the new block index
	int newStart = (!isPhysical) ? blockMap->virtualLen : (blockMap->virtualLen + blockMap->physicalLen);
	auto& newBlockInfo = blockMap->blocks[newStart];

	newBlockInfo.data = malloc(newSize + 4);
	memset(newBlockInfo.data, 0xCD, newSize);

	allocInfo.maxSizes[newStart] = newSize;

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