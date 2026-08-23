#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <PlayerLimits.h>
#include <StackFrameResizer.h>

#include "PlayerPatches.h"

using rage::kMaxPlayers;

void ApplyPlayerStackResizes()
{
	// Support entity migration for >32 in CNetObjProximityMigrateable::_passOutOfScope & CNetObjPedBase::_passOutOfScope
	{
		// 256 * 8: 256 players, ptr size
		// 256 * 4: 256 players, int size
		constexpr int ptrsBase = 0x20;
		constexpr int stackSize = (ptrsBase + (256 * 8) + (256 * 4));
		constexpr int intsBase = ptrsBase + (256 * 8);

		// CNetObjProximityMigrateable::_passOutOfScope
		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 48 8B D9 0F 84", -0x15), { { 0x120, intsBase } });
		// CNetObjPedBase::_passOutOfScope
		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 48 8B 71 ? 48 8B D9 48 85 F6", -0x15), { { 0x120, intsBase } });
	}

	// Resize stack to support >32 players for boat population turn taking
	{
		constexpr int ptrsBase = 0x30;
		constexpr int stackSize = ptrsBase + (kMaxPlayers * 8) + 0x10;
		constexpr int intBase = ptrsBase + (kMaxPlayers * 8);

		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 8B E9 E8", -0x10), { { 0x120, intBase } });
	}

	// Resize stack to support >32 players when updating task sequences
	{
		constexpr int ptrsBase = 0x40;
		constexpr int stackSize = ptrsBase + (kMaxPlayers * 8) + 0x10;
		constexpr int intBase = ptrsBase + (kMaxPlayers * 8);

		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 41 8A D9 45 8B F8", -24), { { 0x188, intBase } });
	}

	// Resize stack to support >32 players with REQUEST_IS_VOLUME_EMPTY netEvent
	{
		constexpr int ptrsBase = 0x20;
		constexpr int arraySize = (kMaxPlayers * 8);
		constexpr int stackSize = ptrsBase + arraySize;

		IncreaseFunctionStack<stackSize, 2048, 8>(hook::get_pattern<char>("48 81 EC ? ? ? ? 41 8A D8 4C 8B F2 4C 8B F9", -0x14), { });
		// memset 0x100 -> arraySize
		hook::put<uint32_t>(hook::get_pattern<uint32_t>("41 B8 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 84 DB", 2), arraySize);
	}

	// Resize stack to support extra bitsets related to NETWORK_BOUNTY_HUNT_EVENT netEvent
	{
		constexpr int ptrsBase = 0x30;
		// 0x30: previous stack size (also containing an int[1])
		// 16: extra int[4] ontop of present int[1] allocation.
		constexpr int stackSize = ptrsBase + 16;
		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 83 EC ? 83 B9 ? ? ? ? ? 4C 8B FA 48 8B F9", -0x14), {});
	}

	// Resize stack to support new bitset size for CGivePickupRewardsEvent
	{
		const int ptrsBase = 0x20;
		// 0x20: previous stack size, containing an int[1] bitset.
		// +16 for an extra int[4] to go ontop of the int[1] allocation at the end of the old stack.
		constexpr int stackSize = ptrsBase + 16;
		IncreaseFunctionStack<stackSize>(hook::get_pattern("48 83 EC ? 8B 1D ? ? ? ? 4C 8B F1", -0x14), {});
	}

	// Resize stack for pending players
	{
		constexpr int ptrsBase = 0x130;
		constexpr int intSize = 0x20 + (sizeof(void*) * kMaxPlayers + 1);
		constexpr int stackSize = ptrsBase + (sizeof(void*) * kMaxPlayers);

		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 8D B1"), { { 0x120, intSize } });
	}

	{
		constexpr int base = 0x40;
		constexpr int stackSize = base + ((kMaxPlayers + 1) * 16);

		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 45 8A E0 48 8B FA 4C 8B F9 33 DB", -0x19), {});
	}

	{
		constexpr int oldStackSize = 0x230;
		constexpr int stackSize = oldStackSize + 0x20;

		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 8B C4 48 89 58 ? 44 89 48 ? 48 89 50 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC"), { { 0x40, oldStackSize }, { 0x270, 0x270 + 0x20 } });
	}

#if 0
	// Resize stack for CTheScripts::_getClosestPlayer
	{
		// 0x50: previous stack size
		// 16: extra int[3] ontop of present int[2] allocation and stack alignment
		constexpr int stackSize = 0x50 + 16;
		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 83 EC ? 0F 29 70 ? 33 ED 0F 29 78 ? 0F 57 FF", -0x10), {});
	}
#endif
}
