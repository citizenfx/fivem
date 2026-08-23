#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <GuardedBitset.h>
#include <PlayerLimits.h>

#include "PlayerPatches.h"

using rage::kMaxPlayers;

static const uint8_t kOriginalPlayers = 32;

void ApplyPlayerFlagBuilderPatches()
{
	{
		static const uint8_t patch[] = { 0x44, 0x8B, 0xC2, 0x49, 0xC1, 0xE8, 0x05, 0x75, 0x0B, 0x42, 0x8B, 0x0C, 0x83, 0x0F, 0xAB, 0xD1, 0x42, 0x89, 0x0C, 0x83, 0x90 };
		ApplyGuardedBitset(hook::get_pattern("44 8B C2 83 E2 1F 49 C1 E8 05 42 8B 0C 83 0F AB D1 42 89 0C 83"), patch, sizeof(patch));
	}

	{
		static const uint8_t patch[] = { 0x8B, 0xD1, 0x48, 0xC1, 0xEA, 0x05, 0x75, 0x09, 0x8B, 0x04, 0x97, 0x0F, 0xAB, 0xC8, 0x89, 0x04, 0x97, 0x90 };
		ApplyGuardedBitset(hook::get_pattern("8B D1 83 E1 1F 48 C1 EA 05 8B 04 97 0F AB C8 89 04 97 48"), patch, sizeof(patch));
	}

	{
		static const uint8_t patch[] = { 0x8B, 0xD1, 0x48, 0xC1, 0xEA, 0x05, 0x75, 0x0B, 0x8B, 0x44, 0x94, 0x48, 0x0F, 0xAB, 0xC8, 0x89, 0x44, 0x94, 0x48, 0x48, 0x8B, 0xCE, 0x90 };
		ApplyGuardedBitset(hook::get_pattern("8B D1 83 E1 1F 48 C1 EA 05 8B 44 94 48 0F AB C8 48 8B CE 89 44 94 48"), patch, sizeof(patch));
	}

	{
		auto location = hook::get_pattern("44 01 7C 9A 0C 81 7C 9A 0C 00 01 00 00 1B C0 21 44 9A 0C");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;

			void Init(uintptr_t retn)
			{
				retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				cmp(rbx, kOriginalPlayers);
				jae("Skip");

				add(dword_ptr[rdx + rbx * 4 + 0xC], r15d);
				cmp(dword_ptr[rdx + rbx * 4 + 0xC], 0x100);
				jb("Skip");

				mov(dword_ptr[rdx + rbx * 4 + 0xC], 0);

				L("Skip");
				mov(r11, retnAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 19);
		patchStub.Init((uintptr_t)location + 0x13);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}
}
