#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

#include <MinHook.h>

//
// Fix crash at GTA5_b3258.exe+63A9CE / +63A9E2 ("quiet-avocado-fourteen")
//
// The function at +63A9CC is a hash lookup that searches for a weapon hash
// (e.g. WEAPON_UNARMED / 0xA2719263) in an entity's weapon hash table.
//
// It receives RCX = pointer to a hash table struct:
//   [RCX+0x00] = pointer to uint32_t hash array
//   [RCX+0x08] = uint16_t capacity
//   [RCX+0x18] = uint16_t count
//
// The crash occurs because RCX points to freed/uninitialized memory (use-after-free).
// This is reached via MULTIPLE call paths:
//
//   Path 1: +6A019B → +6AA96C → +7C67E2 (via CWeaponInfo_GetClipsetForWeaponSwap)
//   Path 2: +61B10A → +7CCE59 → +11AC318 → +11AC89E → +94CC5E → +95D0F4
//
// Because the same vulnerable function is called from many places, patching
// individual callers is insufficient. Instead, we hook the function itself
// and use SEH to safely probe the hash table pointer before accessing it.
//
// Observed RCX values from crash dumps (all freed/invalid, all non-null):
//   0x000003BC00010001, 0x00000000BDC99C23, 0x0000000000060004
//
// Evidence: 20+ crash dumps from production FiveM servers, R8 typically
// contains 0xA2719263 (WEAPON_UNARMED). Crash persists on Canary after
// 7c57040 because that fix only covers Path 1.
//

static bool (*g_origWeaponHashLookup)(void* hashTable, uint32_t hash);

static bool WeaponHashLookup_SafeCheck(void* hashTable, uint32_t hash)
{
	if (!hashTable)
	{
		return false;
	}

	// The hash table pointer is often freed memory that passes the null check.
	// Use SEH to safely probe the memory before calling the original function.
	// We need to verify that [rcx+0x18] (count) and [rcx] (data pointer) are
	// readable, as these are the two dereferences that cause the crash.
	__try
	{
		// Probe the count field at [hashTable+0x18]
		volatile uint16_t count = *(volatile uint16_t*)((char*)hashTable + 0x18);

		if (count == 0)
		{
			return false;
		}

		// Probe the data pointer at [hashTable+0x00]
		volatile void* dataPtr = *(volatile void**)hashTable;

		if (!dataPtr)
		{
			return false;
		}

		// Probe the first element of the data array
		volatile uint32_t firstHash = *(volatile uint32_t*)dataPtr;
		(void)firstHash;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	return g_origWeaponHashLookup(hashTable, hash);
}

static HookFunction hookFunction([]()
{
	// Pattern: xor edx,edx / cmp word ptr [rcx+18h], dx / jbe / movzx eax, word ptr [rcx+8]
	// This is the hash lookup function that crashes when RCX is invalid.
	auto location = hook::get_pattern("33 D2 66 39 51 18 76 ? 0F B7 41 08", 0);

	if (location)
	{
		MH_CreateHook(location, WeaponHashLookup_SafeCheck, (void**)&g_origWeaponHashLookup);
		MH_EnableHook(location);
	}
});
