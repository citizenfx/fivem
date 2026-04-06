#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

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
// At +63A9CE: `cmp word ptr [rcx+0x18], dx` reads from invalid address.
// At +63A9E2: `cmp dword ptr [rax], r8d` where rax = [rcx] = NULL.
//
// Disassembly of the original function:
//   +63A9CC: xor     edx, edx
//   +63A9CE: cmp     word ptr [rcx+18h], dx   ; <-- crash: rcx invalid
//   +63A9D2: jbe     short return_false
//   +63A9D4: movzx   eax, word ptr [rcx+8]
//   +63A9D8: mov     r9d, eax
//   +63A9DB: test    eax, eax
//   +63A9DD: jle     short return_false
//   +63A9DF: mov     rax, qword ptr [rcx]     ; load hash array pointer
//   +63A9E2: cmp     dword ptr [rax], r8d     ; <-- crash: rax = NULL
//   +63A9E5: je      short return_true
//   +63A9E7: inc     rdx
//   +63A9EA: add     rax, 4
//   +63A9EE: cmp     rdx, r9
//   +63A9F1: jl      short loop
//   return_false:
//   +63A9F3: xor     al, al
//   +63A9F5: ret
//   return_true:
//   +63A9F6: mov     al, 1
//   +63A9F8: ret
//
// Fix: hook the function and validate RCX before accessing it.
// If RCX is null or the data pointer at [RCX] is null, return false.
//
// Evidence: 10+ crash dumps from a production FiveM server, all showing
// ACCESS_VIOLATION READ at 0xFFFFFFFFFFFFFFFF with R8 = 0xA2719263
// (WEAPON_UNARMED joaat hash). Verified across multiple clients with
// different hardware (RTX 5090, RTX 4070 SUPER, etc.)
//

static bool (*g_origWeaponHashLookup)(void* hashTable, uint32_t hash);

static bool WeaponHashLookup_NullCheck(void* hashTable, uint32_t hash)
{
	if (!hashTable)
	{
		return false;
	}

	// Check if the data pointer (first field) is null
	void* dataPtr = *(void**)hashTable;
	if (!dataPtr)
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
		MH_CreateHook(location, WeaponHashLookup_NullCheck, (void**)&g_origWeaponHashLookup);
		MH_EnableHook(location);
	}
});
