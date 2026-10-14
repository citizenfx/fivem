#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <atArray.h>

//
// Audio metadata references are packed as `chunkId:8 | offset:24`, so a metadata manager can only address 256
// chunks. Every AUDIO_SOUNDDATA file is a chunk, and once there are more than 256 of them, references from the
// newer ones alias older chunks and their sounds break. Chunks from 128 onwards use `1:1 | chunkId - 128:9 | offset:22`
// instead, which addresses 640 chunks of up to 4 MiB each (the base game's own sounds.dat54 is ~3 MiB).
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
constexpr uint32_t kExtendedOffsetBits = 22;
constexpr uint32_t kExtendedOffsetMask = (1 << kExtendedOffsetBits) - 1;
constexpr uint32_t kMaxExtendedChunks = 1 << (31 - kExtendedOffsetBits);

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

	uint32_t extendedChunk = *chunkId - kFirstExtendedChunk;

	if (extendedChunk >= kMaxExtendedChunks || *offset > kExtendedOffsetMask)
	{
		return false;
	}

	uint32_t ref = kExtendedRefFlag | (extendedChunk << kExtendedOffsetBits) | *offset;
	*chunkId = ref >> 24;
	*offset = ref & 0xFFFFFF;

	return true;
}

static void* (*g_origGetObjectMetadataPtr)(audMetadataManager* self, uint32_t ref);

static void* GetObjectMetadataPtr(audMetadataManager* self, uint32_t ref)
{
	if ((ref & kExtendedRefFlag) == 0 || ref == ~0U)
	{
		return g_origGetObjectMetadataPtr(self, ref);
	}

	uint32_t chunkId = kFirstExtendedChunk + ((ref & ~kExtendedRefFlag) >> kExtendedOffsetBits);
	uint32_t offset = ref & kExtendedOffsetMask;

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
	g_origGetObjectMetadataPtr = hook::trampoline(hook::get_pattern("8D 42 01 4C 8B C1 A9 FE FF FF FF 75 03 33 C0 C3 41 0F B7 40 08 8B CA C1 E9 18 3B C8 73 EF 4D 8B 00 48 6B C9 78 42 80 7C 01 71 00 74 E0 81 E2 FF FF FF 00 42 3B 54 01 18 73 D3 8B C2 4A 03 44 01 10"), GetObjectMetadataPtr);
});
