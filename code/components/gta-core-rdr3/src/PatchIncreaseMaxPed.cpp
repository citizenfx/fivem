#include <StdInc.h>

#include <Pool.h>
#include <PoolSizesState.h>
#include "Hooking.h"
#include "Hooking.Stubs.h"

//
// There is a max limit of 110 ped and 110 ped components. These ped pools are shared with every type of ped (Horses, NPC's, Animals) with the exception of player
// The game hardcode this logic and has some pool size checks on startup, fatally erroring if a certain size is exceeded. Limiting the ability to have more then 110 peds
// This patch resolves this by improving logic to account for the increase size of "CNetObjPedBase" pool and removing a ped pool size check that would otherwise fatally error.
//

template<int EntitySize = 160>
struct EntityCircularBuffer
{
	void* entity[EntitySize];

	uint32_t cursorPos;
	uint32_t baseOffset;
	uint32_t totalEntries;
};

template<int instrLen = 7, int instrOffset = 3>
static void PatchRelativeLocation(uintptr_t address, uintptr_t newLocation)
{
	uint8_t* instructions = reinterpret_cast<uint8_t*>(address);
	uintptr_t instrNext = reinterpret_cast<uintptr_t>(instructions) + instrLen;
	int32_t newOffset = (int32_t)((uintptr_t)newLocation - instrNext);
	hook::put<int32_t>(instructions + instrOffset, newOffset);
}

static HookFunction hookFunction([]()
{
	constexpr size_t kDefaultMaxPeds = 110;

	int64_t increaseSize = 0;

	// We use "CNetObjPedBase" as the increase request for peds and all other components.
	auto sizeIncreaseEntry = fx::PoolSizeManager::GetIncreaseRequest().find("CNetObjPedBase");
	if (sizeIncreaseEntry != fx::PoolSizeManager::GetIncreaseRequest().end())
	{
		increaseSize = sizeIncreaseEntry->second;
	}

	// Don't fatally error if "Peds" pool is greater then 160.
	hook::put<uint8_t>(hook::get_pattern("76 ? BA ? ? ? ? 41 B8 ? ? ? ? 83 C9 ? E8 ? ? ? ? 33 D2 8B CF"), 0xEB);

	// Set total desired peds.
	*hook::get_address<uint32_t*>(hook::get_pattern("89 05 ? ? ? ? C6 05 ? ? ? ? ? 89 15", 2)) = kDefaultMaxPeds + increaseSize;
	
	// Set max amount of peds.
	*hook::get_address<uint32_t*>(hook::get_pattern("89 05 ? ? ? ? 89 05 ? ? ? ? 89 05 ? ? ? ? C6 05", 2)) = kDefaultMaxPeds + increaseSize;

	// Allow registration of script/mission peds up to and past the 110 limit.
	hook::put<uint32_t>(hook::get_pattern("BB ? ? ? ? E9 ? ? ? ? E8 ? ? ? ? 48 8B 0D", 1), kDefaultMaxPeds + increaseSize);

	// Resize ped circular buffer to support greater then 160 peds.
	{
		// Allocate 270 as:
		// + 150 for base peds
		// + 110 for ped increase
		// + 10 extra.
		constexpr int kMaxPedRingBuffer = 270;
		const size_t kMagicDivideNumber = 0x4EC4EC4F;

		using CircularBuffer = EntityCircularBuffer<kMaxPedRingBuffer>; 

		const size_t kNewEntrySize = sizeof(CircularBuffer);
		const size_t kNewCursorPosOffset = (size_t)offsetof(CircularBuffer, cursorPos);
		const size_t kNewBaseOffset = (size_t)offsetof(CircularBuffer, baseOffset);
		const size_t kNewTotalEntryOffset = (size_t)offsetof(CircularBuffer, totalEntries);

		// allocate new location
		static CircularBuffer* newArray = (CircularBuffer*)hook::AllocateStubMemory(kNewEntrySize);
		memset(newArray, 0, kNewEntrySize);

		// Patch RIP relatives
		const uintptr_t kTotalEntriesOffset = (uintptr_t)newArray + kNewTotalEntryOffset;
		const uintptr_t kBaseOffset = (uintptr_t)newArray + kNewBaseOffset;
		const uintptr_t kCurPosOffset = (uintptr_t)newArray + kNewCursorPosOffset;

		PatchRelativeLocation((uintptr_t)hook::get_pattern("44 8B 05 ? ? ? ? 83 F9 ? 8B F1"), kTotalEntriesOffset);
		PatchRelativeLocation((uintptr_t)hook::get_pattern("44 8B 05 ? ? ? ? FF C3 41 3B D8 0F 82 ? ? ? ? 48 8B 5C 24 ? 8B C7"), kTotalEntriesOffset);
		PatchRelativeLocation<6, 2>((uintptr_t)hook::get_pattern("8B 0D ? ? ? ? 8B 05 ? ? ? ? FF C0 3B C7"), kTotalEntriesOffset);
		PatchRelativeLocation<6, 2>((uintptr_t)hook::get_pattern("89 0D ? ? ? ? 48 8D 0D ? ? ? ? 48 98"), kTotalEntriesOffset);
		PatchRelativeLocation<6, 2>((uintptr_t)hook::get_pattern("8B 0D ? ? ? ? BF ? ? ? ? 3B F9"), kTotalEntriesOffset);
		PatchRelativeLocation<6, 2>((uintptr_t)hook::get_pattern("8B 05 ? ? ? ? 44 03 C9"), kTotalEntriesOffset);
		PatchRelativeLocation<6, 2>((uintptr_t)hook::get_pattern("39 0D ? ? ? ? 89 1D"), kTotalEntriesOffset);

		PatchRelativeLocation<6, 2>((uintptr_t)hook::get_pattern("8B 0D ? ? ? ? B8 ? ? ? ? FF C1 03 CB F7 E9 C1 FA ? 8B C2 C1 E8 ? 03 D0 8D 04 92"), kBaseOffset);

		PatchRelativeLocation<6, 2>((uintptr_t)hook::get_pattern("8B 05 ? ? ? ? FF C0 3B C7"), kCurPosOffset);
		PatchRelativeLocation<6, 2>((uintptr_t)hook::get_pattern("89 05 ? ? ? ? 89 0D ? ? ? ? 48 8D 0D"), kCurPosOffset);

		PatchRelativeLocation((uintptr_t)hook::get_pattern("4C 8D 3D ? ? ? ? 8B 0D"), (uintptr_t)newArray);
		PatchRelativeLocation((uintptr_t)hook::get_pattern("48 8D 0D ? ? ? ? 48 98 48 8D 3C C1"), (uintptr_t)newArray);

		// Patch out old magic numbers
		hook::put<uint32_t>(hook::get_pattern("B8 ? ? ? ? 44 03 C6", 1), kMagicDivideNumber);
		hook::put<uint32_t>(hook::get_pattern("B8 ? ? ? ? 41 F7 E8 C1 FA ? 8B C2 C1 E8 ? 03 D0 8D 04 92", 1), kMagicDivideNumber);

		// Patch new limit.
		hook::put<uint32_t>(hook::get_pattern("B8 ? ? ? ? EB ? FF C8 89 83 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 83 C4 ? 5F C3 40 53", 1), kMaxPedRingBuffer - 1);
		// Patch fatal error limit, from 160 to 270 also controls circular buffer cursorPos.
		hook::put<uint32_t>(hook::get_pattern("BF ? ? ? ? 3B F9 75", 1), kMaxPedRingBuffer);

		// Patch new offsets, some of these patterns are intended to break if a future updates alters the struct.
		//+0x500 (cursorPos)
		hook::put<uint32_t>(hook::get_pattern("8B 83 00 05 00 00 85 C0", 2), kNewCursorPosOffset);
		hook::put<uint32_t>(hook::get_pattern("89 83 00 05 00 00 48 8B 5C", 2), kNewCursorPosOffset);

		//+0x504 (baseOffset/unused)
		hook::put<uint32_t>(hook::get_pattern("44 8B 83 ? ? ? ? B8 ? ? ? ? 44 03 C6", 3), kNewBaseOffset);

		//+0x508 (totalEntries)
		hook::put<uint32_t>(hook::get_pattern("8B 81 ? ? ? ? 8D 72 ? 48 8B D9 E9 ? ? ? ? 44 8B 83", 2), kNewTotalEntryOffset);
		hook::put<uint32_t>(hook::get_pattern("8B 83 08 05 00 00 FF C6", 2), kNewTotalEntryOffset);
		hook::put<uint32_t>(hook::get_pattern("89 83 08 05 00 00 8B 83", 2), kNewTotalEntryOffset);
	}
});
