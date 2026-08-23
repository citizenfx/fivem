#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <MinHook.h>

#include <PlayerLimits.h>

#include "PlayerPatches.h"

using rage::kMaxPlayers;

static uint32_t* (*g_unkPlayerFootstepBitset)(void*, uint32_t*);
static uint32_t* _unkPlayerFootstepBitset(void* self, uint32_t* oldBitset)
{
	// a single uint32_t bitset is passed to this function and is used to set player indexes as flags.
	// This causes issues if we want to go above a playerIndex of 32.
	// In all cases the bitset above is only used for this function. So we can allocate our own bitset here
	// and pass that into the original function restoring behaviour
	uint32_t bitset[(kMaxPlayers / 32) + 1] = {};
	return g_unkPlayerFootstepBitset(self, bitset);
}

static void* (*g_sub_1422B40D4)(void*, uint32_t*, uint32_t*, uint32_t*);
static void* sub_1422B40D4(void* a1, uint32_t* oldBitset, uint32_t* unk, uint32_t* unk2)
{
	// a single uint32_t bitset is passed to this function and is used to set player indexes as flags.
	// This causes issues if we want to go above a playerIndex of 32.
	// In all cases the bitset above is only used for this function. So we can allocate our own bitset here
	// and pass that into the original function restoring behaviour
	uint32_t bitset[(kMaxPlayers / 32) + 1] = {};
	return g_sub_1422B40D4(a1, bitset, unk, unk2);
}

// Patch bubble join to prevent writing out of bounds for player objects
void ApplyBubbleJoinPatch()
{
	auto location = hook::get_pattern("44 0F B6 4E ? 0F B6 40");

	static struct : jitasm::Frontend
	{
		uintptr_t retnSuccess;
		uintptr_t retnFail;

		void Init(uintptr_t success, uintptr_t failure)
		{
			retnSuccess = success;
			retnFail = failure;
		}

		virtual void InternalMain() override
		{
			// Original code
			movzx(r9d, byte_ptr[rsi + 0x10]);
			movzx(eax, byte_ptr[rax + 0x20]);

			cmp(eax, 0x20);
			jge("Fail");

			mov(r11, retnSuccess);
			jmp(r11);

			L("Fail");
			mov(r11, retnFail);
			jmp(r11);
		}
	} patchStub;

	const uintptr_t retnSuccess = (uintptr_t)location + 9;
	const uintptr_t retnFail = retnSuccess + 0x19;

	hook::nop(location, 9);
	patchStub.Init(retnSuccess, retnFail);
	hook::jump_reg<5>(location, patchStub.GetCode());
}

void ApplyPlayerBitsetPatches()
{
	// Extend bitshift in order to not lead to crashes
	//TODO: Resize struct, stack and rewrite logic to handle the new bitset size. But for now this is a good enough temporary solution.
	hook::put<uint8_t>(hook::get_pattern("48 C1 EA ? 8B 44 94 ? 0F AB C8 48 8B CE", 3), 8);

	// Jump over a 32 sized bitset tied to CNetObjPedBase ownership migration until we can resize it, corrupts a task related pointer which is not great.
	hook::put<uint8_t>(hook::get_pattern("74 ? 45 33 C0 48 8D 4C 24 ? 48 8B D3 E8 ? ? ? ? 0F 28 44 24"), 0xEB);
}

// Allocate greater sized bitsets to avoid stack corruption
void CreatePlayerBitsetHooks()
{
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 4C 89 44 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 65 4C 8B 14 25"), sub_1422B40D4, (void**)&g_sub_1422B40D4);
	MH_CreateHook(hook::get_pattern("4D 8B 04 C0 4E 39 3C 01 75 ? 33 C0 89 02", -0x39), _unkPlayerFootstepBitset, (void**)&g_unkPlayerFootstepBitset);
}
