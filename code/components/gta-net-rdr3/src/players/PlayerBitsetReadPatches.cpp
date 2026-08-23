#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <PlayerLimits.h>

#include "PlayerPatches.h"

static const uint8_t kOriginalPlayers = 32;

void ApplyBitsetReadPatches()
{
	{
		auto location = hook::get_pattern("8B C2 8B CA 83 E1 1F 48 C1 E8 05 41 D3 E0 44 23 44 87 28");

		static struct : jitasm::Frontend
		{
			uintptr_t readAddr;
			uintptr_t retnAddr;

			void Init(uintptr_t read, uintptr_t retn)
			{
				readAddr = read;
				retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				mov(eax, edx);
				mov(ecx, edx);
				shr(rax, 5);
				shl(r8d, cl);

				cmp(rax, 0);
				jnz("Clear");

				mov(r11, readAddr);
				jmp(r11);

				L("Clear");
				sub(r8d, r8d);
				mov(r11, retnAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 14);
		patchStub.Init((uintptr_t)location + 0xE, (uintptr_t)location + 0x13);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("0F B6 43 19 F6 84 87 54 32 00 00 10 75 32");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;
			uintptr_t outAddr;

			void Init(uintptr_t retn, uintptr_t out)
			{
				retnAddr = retn;
				outAddr = out;
			}

			virtual void InternalMain() override
			{
				movzx(eax, byte_ptr[rbx + 0x19]);

				cmp(eax, kOriginalPlayers);
				jae("Clear");

				test(byte_ptr[rdi + rax * 4 + 0x3254], 0x10);
				mov(r11, retnAddr);
				jmp(r11);

				L("Clear");
				cmp(eax, eax);
				mov(r11, outAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 12);
		patchStub.Init((uintptr_t)location + 0xC, (uintptr_t)location + 0x33);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("8B CA 44 8B C2 48 C1 E9 05 41 83 E0 1F 8B 94 8B C4 7A 02 00");

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
				mov(ecx, edx);
				mov(r8d, edx);
				shr(rcx, 5);
				jnz("Fail");

				mov(edx, dword_ptr[rbx + rcx * 4 + 0x27AC4]);
				mov(r11, retnAddr);
				jmp(r11);

				L("Fail");
				mov(r11, failAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 20);
		patchStub.Init((uintptr_t)location + 0x14, (uintptr_t)location + 0x1F);
		hook::jump_reg<1>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("8B C2 8B CA 48 C1 E8 05 83 E1 1F 41 D3 E0 44 23 44 84 60");

		static struct : jitasm::Frontend
		{
			uintptr_t readAddr;
			uintptr_t retnAddr;

			void Init(uintptr_t read, uintptr_t retn)
			{
				readAddr = read;
				retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				mov(eax, edx);
				mov(ecx, edx);
				shr(rax, 5);
				shl(r8d, cl);

				cmp(rax, 0);
				jnz("Clear");

				mov(r11, readAddr);
				jmp(r11);

				L("Clear");
				sub(r8d, r8d);
				mov(r11, retnAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 14);
		patchStub.Init((uintptr_t)location + 0xE, (uintptr_t)location + 0x13);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("41 8B C8 49 C1 E8 05 83 E1 1F 41 D3 E1 47 85 0C 87");

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
				mov(ecx, r8d);
				shr(r8, 5);
				jnz("Fail");

				shl(r9d, cl);
				mov(r11, retnAddr);
				jmp(r11);

				L("Fail");
				mov(r11, failAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 13);
		patchStub.Init((uintptr_t)location + 0xD, (uintptr_t)location + 0x20);
		hook::jump_reg<1>(location, patchStub.GetCode());
	}
}
