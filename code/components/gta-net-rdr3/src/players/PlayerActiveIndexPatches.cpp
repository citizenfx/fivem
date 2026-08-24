#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <PlayerLimits.h>

static const uint8_t kOriginalPlayers = 32;

static void ApplyActiveIndexPatches()
{
	{
		auto location = hook::get_pattern("48 89 5C 24 18 48 89 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 40 0F");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;

			void Init(uintptr_t retn)
			{
				retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				cmp(byte_ptr[rdx + 0x18], kOriginalPlayers);
				jb("Run");

				ret();

				L("Run");
				mov(qword_ptr[rsp + 0x18], rbx);
				mov(qword_ptr[rsp + 0x10], rdx);

				mov(r11, retnAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 10);
		patchStub.Init((uintptr_t)location + 0xA);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("40 0F B6 C6 39 9C 87 B0 00 00 00");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;
			uintptr_t skipAddr;

			void Init(uintptr_t retn, uintptr_t skip)
			{
				retnAddr = retn;
				skipAddr = skip;
			}

			virtual void InternalMain() override
			{
				mov(rax, rsi);
				movzx(eax, al);

				cmp(eax, kOriginalPlayers);
				jae("Skip");

				cmp(dword_ptr[rdi + rax * 4 + 0xB0], ebx);
				mov(r11, retnAddr);
				jmp(r11);

				L("Skip");
				mov(r11, skipAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 11);
		patchStub.Init((uintptr_t)location + 0xB, (uintptr_t)location + 0xD);
		hook::jump_reg<0>(location, patchStub.GetCode());
	}
}

static HookFunction hookFunction([]()
{
	ApplyActiveIndexPatches();
});
