#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.FlexStruct.h>
#include <Hooking.Stubs.h>
#include <CoreConsole.h>
#include <sysAllocator.h>
#include "Streaming.h"

extern std::string GetCurrentStreamingName();
extern uint32_t GetCurrentStreamingIndex();
extern bool IsHandleCache(uint32_t handle, std::string* outFileName);

constexpr const size_t MaxChunks = 128;
constexpr const ptrdiff_t BonesFieldOffset = 0x20;
constexpr const ptrdiff_t NumBonesFieldOffset = 0x5E;

struct datResourceChunk
{
	void* SrcAddr;
	void* DestAddr;
	size_t Size;
};
static_assert(sizeof(datResourceChunk) == 0x18);

struct datResourceSortedChunk
{
	void* Start;
	size_t Size : 56;
	size_t SelfIndex : 8;
};
static_assert(sizeof(datResourceSortedChunk) == 0x10);

struct datResourceMap
{
	uint8_t VirtualCount;
	uint8_t PhysicalCount;
	uint8_t RootVirtualChunk;
	uint8_t DisableMerge;
	uint8_t HeaderType;
	void* VirtualBase;
	datResourceChunk Chunks[MaxChunks];
};

class datResource
{
public:
	const datResourceMap& m_Map;
	datResource* m_Next;
	const char* m_DebugName;
	datResourceSortedChunk Src[MaxChunks];
	datResourceSortedChunk Dest[MaxChunks];
	uint8_t m_MapCount;
};

struct SearchByAddress
{
	bool operator()(const datResourceSortedChunk& lhs, const datResourceSortedChunk& rhs) const
	{
		return reinterpret_cast<size_t>(lhs.Start) + lhs.Size <= reinterpret_cast<size_t>(rhs.Start);
	}
};

static const datResourceChunk* GetResourceChunk(datResource& rsc, void* address)
{
	datResourceSortedChunk search;
	search.Start = address;
	search.Size = 1;

	const datResourceSortedChunk* start = rsc.Src;
	const datResourceSortedChunk* end = rsc.Src + rsc.m_MapCount;

	const datResourceSortedChunk* chunk = std::lower_bound(start, end, search, SearchByAddress());
	if (chunk != end && address >= chunk->Start && (size_t)address < (size_t)chunk->Start + chunk->Size)
	{
		return &rsc.m_Map.Chunks[chunk->SelfIndex];
	}

	return nullptr;
}

static void (*g_orig_crSkeletonData__crSkeletonData)(hook::FlexStruct* self, datResource& rsc);

static void crSkeletonData__crSkeletonData(hook::FlexStruct* self, datResource& rsc)
{
	void* bones = self->Get<void*>(BonesFieldOffset);
	if (bones)
	{
		const datResourceChunk* chunk = GetResourceChunk(rsc, bones);
		if (chunk)
		{
			ptrdiff_t pageOffset = (uintptr_t)bones - (uintptr_t)chunk->SrcAddr;
			if (pageOffset < 0x10)
			{
				std::string resourceName;
				if (auto index = GetCurrentStreamingIndex(); index != 0)
				{
					auto handle = streaming::Manager::GetInstance()->Entries[index].handle;

					std::string cacheName;
					if (IsHandleCache(handle, &cacheName))
					{
						auto slashIndex = cacheName.find('/') + 1;
						auto secondSlashIndex = cacheName.find('/', slashIndex);

						resourceName = cacheName.substr(slashIndex, secondSlashIndex - slashIndex);
					}
				}

				auto channel = (!resourceName.empty()) ? fmt::sprintf("stream:%s", resourceName) : "streaming";
				console::PrintWarning(channel, "Asset %s was made by a bad tool and did not allocate sufficient memory for the bone data. The asset may lead to crashes upon unload, it has been fixed for this load.\n", GetCurrentStreamingName());

				self->Set(BonesFieldOffset, nullptr);
				self->Set<uint16_t>(NumBonesFieldOffset, 0);
			}
		}
	}

	g_orig_crSkeletonData__crSkeletonData(self, rsc);
}

static HookFunction hookFunction([]
{
	g_orig_crSkeletonData__crSkeletonData = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F2 48 8B D9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 8D 4B"), crSkeletonData__crSkeletonData);
});
