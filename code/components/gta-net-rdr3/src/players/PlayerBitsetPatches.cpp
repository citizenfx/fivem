#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <MinHook.h>

#include <PlayerLimits.h>

using rage::kMaxPlayers;

static const uint8_t kOriginalPlayers = 32;

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

static int (*g_origReassignAckPlayer)(void*, uint8_t, void*, int);
static int ReassignAckPlayer(void* reassignMgr, uint8_t playerIndex, void* peer, int value)
{
	if (playerIndex >= kOriginalPlayers)
	{
		return 0;
	}

	return g_origReassignAckPlayer(reassignMgr, playerIndex, peer, value);
}

static void (*g_origAudioPlayerChannelBits)(void*, uint8_t, uint32_t);
static void AudioPlayerChannelBits(void* self, uint8_t playerIndex, uint32_t shift)
{
	if (playerIndex >= kOriginalPlayers)
	{
		return;
	}

	g_origAudioPlayerChannelBits(self, playerIndex, shift);
}

static uint32_t* (*g_origCalcPlayersInScopeFlags)(void*, uint32_t*, void*);
static uint32_t* CalcPlayersInScopeFlags(void* event, uint32_t* outFlags, void* players)
{
	uint32_t bitset[(kMaxPlayers / 32) + 1] = {};
	g_origCalcPlayersInScopeFlags(event, bitset, players);
	*outFlags = bitset[0];

	return outFlags;
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
static void ApplyBubbleJoinPatch()
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

static void ApplyPlayerSyncDataPatch()
{
	auto location = hook::get_pattern("8B CE 48 8B D6 48 C1 EA 05 83 E1 1F 33 FF 8B 44 93 08 0F B3 C8");

	static struct : jitasm::Frontend
	{
		uintptr_t retnAddr;

		void Init(uintptr_t retn)
		{
			retnAddr = retn;
		}

		virtual void InternalMain() override
		{
			mov(ecx, esi);
			mov(rdx, rsi);
			shr(rdx, 5);
			mov(edi, 0);

			cmp(esi, 0x20);
			jae("Skip");

			mov(eax, dword_ptr[rbx + rdx * 4 + 8]);
			btr(eax, ecx);
			mov(dword_ptr[rbx + rdx * 4 + 8], eax);
			mov(eax, dword_ptr[rbx + rdx * 4 + 0xC]);
			btr(eax, ecx);
			mov(dword_ptr[rbx + rdx * 4 + 0xC], eax);

			L("Skip");
			mov(rcx, rbx);
			mov(r11, retnAddr);
			jmp(r11);
		}
	} patchStub;

	hook::nop(location, 39);
	patchStub.Init((uintptr_t)location + 39);
	hook::jump_reg<2>(location, patchStub.GetCode());
}

static void ApplyNodeDataSentPatch()
{
	{
		auto location = hook::get_pattern("8B 4C 98 08 44 0F B3 F9 89 4C 98 08 EB");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;

			void Init(uintptr_t retn)
			{
				retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				cmp(r12d, 0x20);
				jae("Skip");

				mov(ecx, dword_ptr[rax + rbx * 4 + 8]);
				btr(ecx, r15d);
				mov(dword_ptr[rax + rbx * 4 + 8], ecx);

				L("Skip");
				mov(r11, retnAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 14);
		patchStub.Init((uintptr_t)location + 0x23);
		hook::jump_reg<2>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("41 8B 44 9E 0C 48 8B CE 44 0F B3 F8 41 89 44 9E 0C");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;

			void Init(uintptr_t retn)
			{
				retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				mov(rcx, rsi);

				cmp(r12d, 0x20);
				jae("Skip");

				mov(eax, dword_ptr[r14 + rbx * 4 + 0xC]);
				btr(eax, r15d);
				mov(dword_ptr[r14 + rbx * 4 + 0xC], eax);

				L("Skip");
				mov(r11, retnAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 17);
		patchStub.Init((uintptr_t)location + 17);
		hook::jump_reg<2>(location, patchStub.GetCode());
	}
}

static void ApplySyncDataOwnerPatch()
{
	{
		auto location = hook::get_pattern<char>("48 C1 EA 05 8B 04 93 44 0F AB C0 89 04 93", 4);

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;

			void Init(uintptr_t retn)
			{
				retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				test(rdx, rdx);
				jnz("Skip");

				mov(eax, dword_ptr[rbx + rdx * 4]);
				bts(eax, r8d);
				mov(dword_ptr[rbx + rdx * 4], eax);

				L("Skip");
				mov(rax, retnAddr);
				jmp(rax);
			}
		} patchStub;

		hook::nop(location, 10);
		patchStub.Init((uintptr_t)location + 10);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern<char>("48 C1 EA 05 8B 04 93 0F B3 C8 48 8B CF 89 04 93", 4);

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;

			void Init(uintptr_t retn)
			{
				retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				test(rdx, rdx);
				jnz("Skip");

				mov(eax, dword_ptr[rbx + rdx * 4]);
				btr(eax, ecx);
				mov(dword_ptr[rbx + rdx * 4], eax);

				L("Skip");
				mov(rcx, rdi);
				mov(rax, retnAddr);
				jmp(rax);
			}
		} patchStub;

		hook::nop(location, 12);
		patchStub.Init((uintptr_t)location + 12);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}
}

static void ApplyScriptHandlerNodePatch()
{
	auto location = hook::get_pattern("40 0F B6 CD 8B D1 83 E1 1F 48 C1 EA ? 41 8B 44 90 ? 0F B3 C8 41 89 44 90");

	static struct : jitasm::Frontend
	{
		uintptr_t retnAddr;

		void Init(uintptr_t retn)
		{
			retnAddr = retn;
		}

		virtual void InternalMain() override
		{
			mov(rcx, rbp);
			movzx(ecx, cl);

			cmp(ecx, 0x20);
			jae("Skip");

			mov(edx, ecx);
			shr(rdx, 5);
			mov(eax, dword_ptr[r8 + rdx * 4 + 8]);
			btr(eax, ecx);
			mov(dword_ptr[r8 + rdx * 4 + 8], eax);

			L("Skip");
			mov(rax, retnAddr);
			jmp(rax);
		}
	} patchStub;

	hook::nop(location, 26);
	patchStub.Init((uintptr_t)location + 26);
	hook::jump_reg<0>(location, patchStub.GetCode());
}

static void ApplyGhostMaskPatch()
{
	auto location = hook::get_pattern("49 8B CB 41 83 E3 1F 48 C1 E9 ? 41 8B 04 89 44 0F AB D8 41 89 04 89");

	static struct : jitasm::Frontend
	{
		uintptr_t retnAddr;

		void Init(uintptr_t retn)
		{
			retnAddr = retn;
		}

		virtual void InternalMain() override
		{
			mov(rcx, r11);

			cmp(ecx, 0x20);
			jae("Skip");

			shr(rcx, 5);
			mov(eax, dword_ptr[r9 + rcx * 4]);
			bts(eax, r11d);
			mov(dword_ptr[r9 + rcx * 4], eax);

			L("Skip");
			mov(rax, retnAddr);
			jmp(rax);
		}
	} patchStub;

	hook::nop(location, 23);
	patchStub.Init((uintptr_t)location + 23);
	hook::jump_reg<0>(location, patchStub.GetCode());
}

static void ApplyRemoteScriptInfoPatch()
{
	auto location = hook::get_pattern("41 8B 44 90 ? 0F AB C8 41 89 44 90 ? 41 83 60 ? ?");

	static struct : jitasm::Frontend
	{
		uintptr_t retnAddr;

		void Init(uintptr_t retn)
		{
			retnAddr = retn;
		}

		virtual void InternalMain() override
		{
			test(rdx, rdx);
			jnz("Skip");

			mov(eax, dword_ptr[r8 + rdx * 4 + 0x28]);
			bts(eax, ecx);
			mov(dword_ptr[r8 + rdx * 4 + 0x28], eax);

			L("Skip");
			mov(rax, retnAddr);
			jmp(rax);
		}
	} patchStub;

	hook::nop(location, 13);
	patchStub.Init((uintptr_t)location + 13);
	hook::jump_reg<0>(location, patchStub.GetCode());
}

static void ApplyCachedPlayerDataPatch()
{
	auto location = hook::get_pattern<char>("0F B6 43 19 48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 32");

	static struct : jitasm::Frontend
	{
		uintptr_t retnAddr;
		uintptr_t failAddr;
		uintptr_t tableAddr;

		void Init(uintptr_t retn, uintptr_t fail, uintptr_t table)
		{
			retnAddr = retn;
			failAddr = fail;
			tableAddr = table;
		}

		virtual void InternalMain() override
		{
			movzx(eax, byte_ptr[rbx + 0x19]);

			cmp(eax, kOriginalPlayers);
			jae("Fail");

			mov(rcx, tableAddr);
			mov(r11, retnAddr);
			jmp(r11);

			L("Fail");
			mov(r11, failAddr);
			jmp(r11);
		}
	} patchStub;

	auto table = hook::get_address<uintptr_t>(location + 4, 3, 7);

	hook::nop(location, 11);
	patchStub.Init((uintptr_t)location + 11, (uintptr_t)location + 0x46, table);
	hook::jump_reg<0>(location, patchStub.GetCode());
}

static void ApplySyncMessageSequencePatch()
{
	auto location = hook::get_pattern("66 46 01 AC 76 D8 00 00 00");

	static struct : jitasm::Frontend
	{
		uintptr_t retnAddr;

		void Init(uintptr_t retn)
		{
			retnAddr = retn;
		}

		virtual void InternalMain() override
		{
			cmp(r14, kOriginalPlayers);
			jae("Skip");

			add(word_ptr[rsi + r14 * 2 + 0xD8], r13w);
			movzx(eax, word_ptr[rsi + r14 * 2 + 0xD8]);
			mov(word_ptr[rsi + rbp + 0x10F30], ax);

			L("Skip");
			mov(r11, retnAddr);
			jmp(r11);
		}
	} patchStub;

	hook::nop(location, 26);
	patchStub.Init((uintptr_t)location + 26);
	hook::jump_reg<0>(location, patchStub.GetCode());
}

static void ApplyCachedPlayerLookupPatches()
{
	{
		auto location = hook::get_pattern<char>("48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 06 83");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;
			uintptr_t failAddr;
			uintptr_t tableAddr;

			void Init(uintptr_t retn, uintptr_t fail, uintptr_t table)
			{
				retnAddr = retn;
				failAddr = fail;
				tableAddr = table;
			}

			virtual void InternalMain() override
			{
				mov(rcx, tableAddr);

				cmp(eax, kOriginalPlayers);
				jae("Fail");

				mov(rcx, qword_ptr[rcx + rax * 8]);
				mov(r11, retnAddr);
				jmp(r11);

				L("Fail");
				mov(r11, failAddr);
				jmp(r11);
			}
		} patchStub;

		auto table = hook::get_address<uintptr_t>(location, 3, 7);

		hook::nop(location, 11);
		patchStub.Init((uintptr_t)location + 0xB, (uintptr_t)location + 0x16, table);
		hook::jump_reg<1>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("BD 00 01 00 00 4C 8B 04 C1");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;
			uintptr_t failAddr;

			void Init(uintptr_t retn, uintptr_t fail)
			{
				retnAddr = retn;
				failAddr = fail;
			}

			virtual void InternalMain() override
			{
				mov(ebp, 0x100);

				cmp(eax, kOriginalPlayers);
				jae("Fail");

				mov(r8, qword_ptr[rcx + rax * 8]);
				mov(r11, retnAddr);
				jmp(r11);

				L("Fail");
				mov(r11, failAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 9);
		patchStub.Init((uintptr_t)location + 9, (uintptr_t)location + 0x15);
		hook::jump_reg<5>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("0F B6 42 19 48 8B 0C C1");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;
			uintptr_t failAddr;

			void Init(uintptr_t retn, uintptr_t fail)
			{
				retnAddr = retn;
				failAddr = fail;
			}

			virtual void InternalMain() override
			{
				movzx(eax, byte_ptr[rdx + 0x19]);

				cmp(eax, kOriginalPlayers);
				jae("Fail");

				mov(rcx, qword_ptr[rcx + rax * 8]);
				mov(r11, retnAddr);
				jmp(r11);

				L("Fail");
				mov(r11, failAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 8);
		patchStub.Init((uintptr_t)location + 8, (uintptr_t)location + 0x13);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}
}

static void ApplyPlayerBitsetPatches()
{
	// Jump over a 32 sized bitset tied to CNetObjPedBase ownership migration until we can resize it, corrupts a task related pointer which is not great.
	hook::put<uint8_t>(hook::get_pattern("74 ? 45 33 C0 48 8D 4C 24 ? 48 8B D3 E8 ? ? ? ? 0F 28 44 24"), 0xEB);
}

// Allocate greater sized bitsets to avoid stack corruption
static void CreatePlayerBitsetHooks()
{
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 4C 89 44 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 65 4C 8B 14 25"), sub_1422B40D4, (void**)&g_sub_1422B40D4);
	MH_CreateHook(hook::get_pattern("4D 8B 04 C0 4E 39 3C 01 75 ? 33 C0 89 02", -0x39), _unkPlayerFootstepBitset, (void**)&g_unkPlayerFootstepBitset);
	MH_CreateHook(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 32 DB 45 8B F1 45 32 D2"), ReassignAckPlayer, (void**)&g_origReassignAckPlayer);
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 45 33 C9 4C 8B D1 66 44 3B 49 ? 73 ? 41 8B C8 0F B6 C2"), AudioPlayerChannelBits, (void**)&g_origAudioPlayerChannelBits);
	MH_CreateHook(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 54 41 56 41 57 48 83 EC 30 65 48 8B 0C 25 58 00 00 00 48"), CalcPlayersInScopeFlags, (void**)&g_origCalcPlayersInScopeFlags);
}

static HookFunction hookFunction([]()
{
	ApplyBubbleJoinPatch();
	ApplyPlayerBitsetPatches();
	ApplyPlayerSyncDataPatch();
	ApplyNodeDataSentPatch();
	ApplySyncDataOwnerPatch();
	ApplyScriptHandlerNodePatch();
	ApplyGhostMaskPatch();
	ApplyRemoteScriptInfoPatch();
	ApplyCachedPlayerDataPatch();
	ApplySyncMessageSequencePatch();
	ApplyCachedPlayerLookupPatches();

	MH_Initialize();

	CreatePlayerBitsetHooks();

	MH_EnableHook(MH_ALL_HOOKS);
});
