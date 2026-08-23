#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <netPlayerManager.h>

#include <PatchUtils.h>
#include <PlayerLimits.h>

#include "PlayerPatches.h"

using rage::kMaxPlayers;

static void** g_cachedPlayerArray;
static uint16_t* g_localPlayerInstanceId;

// This function is protected with arxan and is self-healed
static uint32_t GetPlayerFastInstanceState(uint8_t playerIndex)
{
	if (playerIndex < kMaxPlayers + 1)
	{
		hook::FlexStruct* player = reinterpret_cast<hook::FlexStruct*>(g_cachedPlayerArray[playerIndex]);
		if (player)
		{
			return player->At<uint32_t>(0x4);
		}
	}

	return 0;
}

static uint16_t GetPlayerFastInstanceId(uint8_t playerIndex)
{
	if (playerIndex < kMaxPlayers + 1)
	{
		if (playerIndex == rage::GetLocalPlayer()->physicalPlayerIndex())
		{
			return *g_localPlayerInstanceId;
		}

		hook::FlexStruct* player = reinterpret_cast<hook::FlexStruct*>(g_cachedPlayerArray[playerIndex]);
		if (player)
		{
			return player->At<uint16_t>(0x2);
		}
	}
	return 0x100;
}

void ApplyPlayerCachePatches()
{
	static size_t kCachedPlayerSize = sizeof(void*) * (kMaxPlayers + 1);
	g_cachedPlayerArray = (void**)hook::AllocateStubMemory(kCachedPlayerSize);
	memset(g_cachedPlayerArray, 0, kCachedPlayerSize);

	g_localPlayerInstanceId = hook::get_address<uint16_t*>(hook::get_pattern("66 89 05 ? ? ? ? B2", 2));

	RelocateRelative((void*)g_cachedPlayerArray, {
		{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 66 83 79", 3 },
		{ "48 8D 0D ? ? ? ? 48 8B F2 BD", 3 },
		{ "BD ? ? ? ? 8B C3 48 8D 0D", 10 },
		{ "0F B6 C3 48 8D 0D ? ? ? ? 48 8B 0C C1", 6 },
		{ "48 8D 0D ? ? ? ? 48 8B 0C ? 48 85 C9 74 ? 8B 41", 3},
		{ "48 8D 0D ? ? ? ? 48 8B 04 C1 8A 40", 3 },
		{ "4C 8D 0D ? ? ? ? 48 63 05 ? ? ? ? 48 8B C8", 3 },
		{ "48 8D 0D ? ? ? ? 88 15 ? ? ? ? 88 15 ? ? ? ? 88 15", 3 },
		{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 83 79", 3 },
		{ "48 8D 15 ? ? ? ? 48 8B 14 CA 48 85 D2 74 ? 39 72", 3 },
		{ "48 8D 0D ? ? ? ? 48 8B C3 48 8B 0C D9", 3 },
		{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 8A 41 ? EB", 3 },
		{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 40 88 79", 3 },
		{ "48 8D 15 ? ? ? ? 88 19", 3 },
		{ "48 8D 3D ? ? ? ? 48 8B 0C DF", 3 },
		{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 66 89 ?", 3 },
		{ "48 8D 1D ? ? ? ? 48 8B 1C C3 48 85 DB 74 ? 66 39 73", 3 },
		{ "48 8D 0D ? ? ? ? 48 8B 04 C1 40 88 78", 3 },
		{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 66 3B 59", 3 },
		{ "48 8D 15 ? ? ? ? 84 C9 75", 3 }
	}, 20);

	PatchValue<uint8_t>({
		// player cached getters
		{ "E8 ? ? ? ? 84 C0 74 ? 40 88 3D ? ? ? ? EB", 23, 0x20, kMaxPlayers + 1 },
		//{ "80 F9 ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? E8", 2, 0x20, kMaxPlayers + 1},
		//{ "83 F9 ? 7C ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8D 0D ? ? ? ? 48 8B 0C D9", 2, 0x20, kMaxPlayers + 1},
		{ "80 FA ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 8A CB", 2, 0x20, kMaxPlayers + 1},
		{ "80 7B ? ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 0F B6 43 ? 48 8D 0D", 3, 0x20, kMaxPlayers + 1},
		{ "80 F9 ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 0F B6 DB", 2, 0x20, kMaxPlayers + 1}, // removeCachedPlayerEntry
		{ "80 F9 ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 0F B6 C3", 2, 0x20, kMaxPlayers + 1}, // _setPlayerCachedFastInstanceId
		{ "80 FB ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 8A CB", 2, 0x20, kMaxPlayers + 1},
		{ "48 85 C0 74 ? 83 61 ? ? B8 ? ? ? ? 66 83 61", -57, 0x20, kMaxPlayers + 1 }, // addCachedPlayerEntry
		//{ "83 F9 ? 7C ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8D 0D ? ? ? ? 48 8B C3", 2, 0x20, kMaxPlayers + 1 }, // getPlayerCachedInTutorialState
		{ "83 F8 ? 72 ? C3 CC 0F 48 8B", 2, 0x20, kMaxPlayers + 1 },
		{ "80 FB ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 75 ? B0", 2, 0x20, kMaxPlayers + 1},
		{ "83 F9 ? 7C ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8D 0D ? ? ? ? 48 8B C3", 2, 0x20, kMaxPlayers + 1 }
	});

	// Handle GetPlayerFastInstanceState getters
	// This function is axran
	{
		std::initializer_list<PatternPair> pairs = {
			{ "E8 ? ? ? ? 83 F8 ? 75 ? B8 ? ? ? ? EB ? 8A CB", 0 },
			{ "E8 ? ? ? ? 83 F8 ? 75 ? 48 85 DB 0F 84", 0 },
			{ "E8 ? ? ? ? 83 F8 ? 74 ? 32 C0 EB ? B0 ? 48 83 C4 ? 5B C3 CC 79", 0 },
			{ "E8 ? ? ? ? 0F B6 4F ? 3B C1", 0 },
			{ "E8 ? ? ? ? 83 F8 ? 74 ? 40 8A CE", 0 }
		};

		for (auto& entry : pairs)
		{
			hook::call(hook::get_pattern(entry.pattern, entry.offset), GetPlayerFastInstanceState);
		}
	}

	// Handle GetPlayerFastInstanceId getters
	// This function is also axran
	{
		std::initializer_list<PatternPair> pairs = {
			{ "E8 ? ? ? ? 98 48 83 C4 ? 5B", 0 },
			{ "E8 ? ? ? ? B9 ? ? ? ? 66 3B C1 0F 85", 0 },
			{ "E8 ? ? ? ? 0F B7 0D ? ? ? ? 66 3B C1", 0 },
			{ "E8 ? ? ? ? 66 41 3B C4 75 ? 83 BF", 0 },
			{ "E8 ? ? ? ? 0F BF C8 41 3B CC", 0 },
			{ "E8 ? ? ? ? 66 3B C3 74 ? 49 8B CE", 0 },
			{ "E8 ? ? ? ? 0F BF F8 41 BF", 0 },
			{ "E8 ? ? ? ? 40 84 ED 74 ? 80 BC 24", 0 }
		};

		for (auto& entry : pairs)
		{
			hook::call(hook::get_pattern(entry.pattern, entry.offset), GetPlayerFastInstanceId);
		}
	}
}

void ApplyPlayerBandwidthPatches()
{
	constexpr size_t kBandwithArraySize = sizeof(uint32_t) * (static_cast<size_t>(kMaxPlayers) + 1);
	void** bandwidthRelatedArray = (void**)hook::AllocateStubMemory(kBandwithArraySize);
	memset(bandwidthRelatedArray, 0, kBandwithArraySize);

	RelocateRelative((void*)bandwidthRelatedArray, {
		{ "48 8D 3D ? ? ? ? B9 ? ? ? ? F3 AB 48 8D 8B", 3 },
		{ "48 8D 15 ? ? ? ? C7 04 82", 3 },
		{ "48 8D 0D ? ? ? ? 89 1C 81", 3 },
		{ "48 8D 05 ? ? ? ? 8B 14 B8 3B EA", 3 },
		{ "48 8D 0D ? ? ? ? 8B 04 81 C3 90 66 40 53", 3 }
	});

	PatchValue<uint32_t>({
		{ "B9 ? ? ? ? F3 AB 48 8D 8B ? ? ? ? 33 D2", 1, 0x20, kMaxPlayers + 1 },
	});
}
