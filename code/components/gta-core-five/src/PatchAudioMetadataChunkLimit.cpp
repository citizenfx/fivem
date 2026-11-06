#include <StdInc.h>
#include <Hooking.h>

#include <atArray.h>

//
// Audio metadata references are packed as `chunkId:8 | offset:24`, so a metadata manager can only address 256
// chunks. Every AUDIO_SOUNDDATA, AUDIO_GAMEDATA and AUDIO_SYNTHDATA file is a chunk of its type's manager, and once
// there are more than 256 of one type, references from the newer ones alias older chunks and their sounds break.
// Chunks from 128 onwards instead get a range of a shared 2 GiB address space and are referenced as `1:1 |
// address:31`. A table maps every 64 KiB block of that space back to its chunk, so these chunks can still be as
// large as the 16 MiB a 24-bit offset allowed.
//

struct audMetadataChunk
{
	char pad_0[16];
	uint8_t* data;
	uint32_t dataSize;
	char pad_1C[85];
	bool enabled;
	char pad_72[6];
};

static_assert(offsetof(audMetadataChunk, data) == 0x10);
static_assert(offsetof(audMetadataChunk, dataSize) == 0x18);
static_assert(offsetof(audMetadataChunk, enabled) == 0x71);
static_assert(sizeof(audMetadataChunk) == 0x78);

struct audMetadataManager
{
	atArray<audMetadataChunk> chunks;
};

constexpr uint32_t kExtendedRefFlag = 0x80000000;
constexpr uint32_t kFirstExtendedChunk = 128;
constexpr uint32_t kBlockBits = 16;
constexpr uint32_t kBlockMask = (1 << kBlockBits) - 1;
constexpr uint32_t kBlockCount = 1 << (31 - kBlockBits);

struct ExtendedBlock
{
	uint16_t chunkId;
	uint16_t chunkBlock;
};

struct ExtendedRange
{
	uint32_t firstBlock;
	uint32_t blockCount;
};

// unassigned blocks name chunk 0xFFFF, which is never loaded, so they decode to null like the reference 0xFFFFFFFF
static ExtendedBlock g_extendedBlocks[kBlockCount];
static std::map<std::pair<audMetadataManager*, uint32_t>, ExtendedRange> g_extendedRanges;
static uint32_t g_nextExtendedBlock;
static std::mutex g_extendedMutex;

static bool (*g_origSearchForObjectFromHash)(audMetadataManager* self, uint32_t hash, uint32_t* chunkId, uint32_t* offset);

// the callers pack the result as `chunkId << 24 | (offset & 0xFFFFFF)`, so split an extended reference the same way
static bool SearchForObjectRef(audMetadataManager* self, uint32_t hash, uint32_t* chunkId, uint32_t* offset)
{
	if (!g_origSearchForObjectFromHash(self, hash, chunkId, offset))
	{
		return false;
	}

	if (*chunkId < kFirstExtendedChunk)
	{
		return true;
	}

	uint32_t dataSize = self->chunks.m_offset[*chunkId].dataSize;
	uint32_t blockCount = uint32_t((uint64_t(dataSize) + kBlockMask) >> kBlockBits);

	if (*offset >= dataSize)
	{
		return false;
	}

	std::lock_guard lock(g_extendedMutex);
	auto& range = g_extendedRanges[{ self, *chunkId }];

	// a chunk that was reloaded larger moves to a new range, and the old one keeps resolving to it like before
	if (range.blockCount < blockCount)
	{
		// the last block is kept unassigned for the null reference
		if (blockCount > kBlockCount - 1 - g_nextExtendedBlock)
		{
			return false;
		}

		range = { g_nextExtendedBlock, blockCount };
		g_nextExtendedBlock += blockCount;

		for (uint32_t i = 0; i < blockCount; i++)
		{
			g_extendedBlocks[range.firstBlock + i] = { uint16_t(*chunkId), uint16_t(i) };
		}
	}

	uint32_t ref = kExtendedRefFlag | ((range.firstBlock << kBlockBits) + *offset);
	*chunkId = ref >> 24;
	*offset = ref & 0xFFFFFF;

	return true;
}

static void* GetObjectMetadataPtr(audMetadataManager* self, uint32_t ref)
{
	uint32_t chunkId = ref >> 24;
	uint32_t offset = ref & 0xFFFFFF;

	if (ref & kExtendedRefFlag)
	{
		auto block = g_extendedBlocks[(ref & ~kExtendedRefFlag) >> kBlockBits];
		chunkId = block.chunkId;
		offset = (uint32_t(block.chunkBlock) << kBlockBits) | (ref & kBlockMask);
	}
	else if (ref == 0)
	{
		return nullptr;
	}

	if (chunkId >= self->chunks.GetCount())
	{
		return nullptr;
	}

	auto& chunk = self->chunks.m_offset[chunkId];

	if (!chunk.enabled || offset >= chunk.dataSize)
	{
		return nullptr;
	}

	return chunk.data + offset;
}

static HookFunction hookFunction([]
{
	std::fill(std::begin(g_extendedBlocks), std::end(g_extendedBlocks), ExtendedBlock{ 0xFFFF, 0 });

	// BuildObjectMap, BuildObjectMap_External and GetObjectMetadataRefFromHash
	for (auto pattern : {
		"E8 ? ? ? ? 84 C0 75 05 83 0B FF EB 15 8B 85 68 01 00 00 8B 4D 00 C1 E1 18",
		"E8 ? ? ? ? 84 C0 75 06 41 83 0E FF EB 14 8B 4D 58 8B 45 00 81 E1 FF FF FF 00 C1 E0 18",
		"E8 ? ? ? ? 84 C0 75 05 83 0B FF EB 15 8B 4C 24 58 8B 44 24 20 81 E1 FF FF FF 00 C1 E0 18",
	})
	{
		auto location = hook::get_pattern(pattern);
		hook::set_call(&g_origSearchForObjectFromHash, location);
		hook::call(location, SearchForObjectRef);
	}

	// the whole function, so a change to the audMetadataChunk layout fails here instead of at runtime
	hook::jump(hook::get_pattern("8D 42 01 4C 8B C1 A9 FE FF FF FF 75 03 33 C0 C3 41 0F B7 40 08 8B CA C1 E9 18 3B C8 73 EF 4D 8B 00 48 6B C9 78 42 80 7C 01 71 00 74 E0 81 E2 FF FF FF 00 42 3B 54 01 18 73 D3 8B C2 4A 03 44 01 10"), GetObjectMetadataPtr);
});
