#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <PatchUtils.h>
#include <PlayerLimits.h>

using rage::kMaxPlayers;

// Extend AnimScene handler array, as we need to support kMaxPlayers instead of just 32
// Theres a few finicky bits in AnimScenes that need extra attention.
// It may be best to leave this until its patched in the future to work under onesync.
static void ApplyAnimSceneHandlerPatches()
{
#if 0
	const size_t kPlayerEntriesSize = (static_cast<size_t>((kMaxPlayers + 1)) * 0x3200);
	const size_t kObjectEntriesSize = (static_cast<size_t>(1600) * 0x40);
	const size_t kStructSize = kPlayerEntriesSize + kObjectEntriesSize;
	void** animSceneArray = (void**)hook::AllocateStubMemory(kStructSize);

	RelocateRelative((void*)animSceneArray, {
		{ "48 8D 05 ? ? ? ? 48 03 D8 48 85 DB", 3},
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 0F B7 47 ? 66 39 01 75 ? 80 79", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 41 0F B7 46 ? 66 39 01 75 ? 8A 41 ? 2C ? 3C", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 0F B7 43", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 41 0F B7 46 ? 66 39 01 75 ? 8A 41 ? 2C ? 75", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 0F B7 46 ? 66 39 01 75 ? 8A 41", 3 },
		{ "48 8D 05 ? ? ? ? 8B D3 48 03 C8 48 85 C9 74 ? 41 0F B7 46", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 0F B7 47 ? 66 39 01 75 ? 8A 41", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 0F B7 46 ? 66 39 01 75 ? 8B D5", 3 },
		{ "48 8D 35 ? ? ? ? 33 C9 E8", 3 },
		{ "48 8D 1D ? ? ? ? 8B F5", 3 },
		{ "48 8D 15 ? ? ? ? 48 69 C9 ? ? ? ? 44 8B CD", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 0F B7 47 ? 66 39 01 74", 3},
		{ "48 8D 05 ? ? ? ? 48 69 D1 ? ? ? ? 45 33 C0", 3 }
	});

	// struct
	// {
	//   char playerEntries[0x3200 * kMaxPlayers + 1];
	//   char objectEntries[0x40 * 1600];
	//}

	// Start of object entries
	PatchRelativeLocation((uintptr_t)hook::get_pattern("48 8D 0D ? ? ? ? BA ? ? ? ? 45 33 C0 66 44 89 41"), (uintptr_t)animSceneArray + kPlayerEntriesSize);
	PatchRelativeLocation((uintptr_t)hook::get_pattern("48 8D 1D ? ? ? ? 33 F6 48 FF CF 48 8D 5B ? 66 39 33"), (uintptr_t)animSceneArray + kStructSize);

	// Patch 8-bit registers
	PatchValue<uint8_t>({
		{ "40 80 FF ? 72 ? 40 8A C5", 3, 0x20, kMaxPlayers + 1 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 0F B7 47 ? 66 39 01 75 ? 80 79", 48, 0x20, kMaxPlayers + 1 },
		{ "40 80 FF ? 72 ? 40 8A C6 EB ? 83 B8 ? ? ? ? ? 0F 9D C0", 3, 0x20, kMaxPlayers + 1 },
		{ "40 80 FF ? 72 ? 40 8A C6 EB ? F6 80", 3, 0x20, kMaxPlayers + 1 },
		{ "40 80 FF ? 72 ? 32 C0", 3, 0x20, kMaxPlayers + 1 },
		{ "40 80 FF ? 72 ? 40 8A C6 EB ? 83 B8 ? ? ? ? ? 7D", 3, 0x20, kMaxPlayers + 1 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 48 85 C9 74 ? 0F B7 47 ? 66 39 01 75 ? 8A 41", 51, 0x20, kMaxPlayers + 1 },
		{ "80 FB ? 0F 82 ? ? ? ? B0", 2, 0x20, kMaxPlayers + 1 }
	});
#endif
}

static void ApplyAnimSceneArrayHandlerPatches()
{
#if 0
	static size_t kPlayerArraySize = sizeof(void*) * (kMaxPlayers + 1);
	void** playerArrayHandler = (void**)hook::AllocateStubMemory(kPlayerArraySize);

	RelocateRelative((void*)playerArrayHandler, {
		{ "48 8D 3D ? ? ? ? BD ? ? ? ? 48 8D 35", 3 },
		{ "48 8D 3D ? ? ? ? 48 8B 3C C7 48 85 FF 75", 3 },
		{ "48 8D 1D ? ? ? ? 48 8B 33 48 85 F6 74 ? 48 8B 06", 3 }
	});

	// Patch 32-bit registers
	PatchValue<uint32_t>({
		{ "BD ? ? ? ? 48 8D 35 ? ? ? ? 33 C9", 1, 0x20, kMaxPlayers + 1 },
		{ "BD ? ? ? ? 48 8D 1D ? ? ? ? 8B F5", 1, 0x20, kMaxPlayers + 1 }
	});
#endif
}

static HookFunction hookFunction([]()
{
	ApplyAnimSceneHandlerPatches();
	ApplyAnimSceneArrayHandlerPatches();
});
