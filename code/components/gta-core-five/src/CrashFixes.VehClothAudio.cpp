/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

#include <Error.h>
#include <CrossBuildRuntime.h>

//
// cloth data in vehicle audio, validity check
// for crash sig @4316ee
//

static uint64_t lastClothValue;
static char* lastClothPtr;

static void ReportCrash()
{
	static bool hasCrashedBefore = false;

	if (!hasCrashedBefore)
	{
		hasCrashedBefore = true;

		trace("WARNING: cloth data crash triggered (invalid pointer: %016llx, dummy: %016llx)\n", (uintptr_t)lastClothPtr, (uintptr_t)lastClothValue);

		AddCrashometry("cloth_data_crash", "true");
	}
}

static bool VerifyClothDataInst(char* pointer)
{
	__try
	{
		lastClothValue = *(uint64_t*)(pointer + 0x1E0);

		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		lastClothPtr = pointer;
		ReportCrash();

		return false;
	}
}

static HookFunction hookFunction([]()
{
	if (xbr::IsGameBuildOrGreater<3095>())
	{
		// Replacing 'add' with a call to a SEH guarded dereference
		//
		// 48 85 C0                 test    rax, rax
		// 74 2C                    jz      short loc_1403C612B
		// 48 05 48 01 00 00        add     rax, 148h           // Replacing weird pointer logic
		// 74 24                    jz      short loc_1403C612B
		// 48 83 B8 98 00 00 00 00  cmp qword ptr [rax+98h], 0
		// 8A 83 E4 0D 00 00        mov al, [rbx+3556]
		// 0F 95 C1                 setnz   cl
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				// set up a stack frame for function calling
				push(rax);
				sub(rsp, 0x20);

				// call VerifyClothDataInst with rax (the pointer to probe)
				mov(rcx, rax);
				mov(rax, (uintptr_t)&VerifyClothDataInst);
				call(rax);

				// pop stack frame
				add(rsp, 0x20);

				// ensure proper flags are set; bit of a hack but should be
				// preserved over subsequent pop/ret.
				test(al, al);
				pop(rax);

				// Return and execute next jz
				ret();
			}
		} clothFixStub3095;

		auto location = hook::get_pattern("0F 95 C1 24 FB 80 E1 01 80 C9 02", -0x16);
		hook::nop(location, 0x6);
		hook::call_reg<2>(location, clothFixStub3095.GetCode());
	}
	else
	{
		// https://github.com/citizenfx/fivem/commit/a0f35b50fd6ecb10f052341374fbe47eb9401764
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				// save rcx and rax as these are our instruction operands
				push(rcx);
				push(rax);

				// set up a stack frame for function calling
				push(rbp);
				mov(rbp, rsp);
				sub(rsp, 32);

				// call VerifyClothDataInst with rax (the pointer to probe)
				mov(rcx, rax);
				mov(rax, (uintptr_t)&VerifyClothDataInst);
				call(rax);

				// if 0, it's invalid
				test(al, al);
				jz("otherReturn");

				// pop stack frame and execute original cmp instruction
				add(rsp, 32);
				pop(rbp);

				pop(rax);
				pop(rcx);

				cmp(qword_ptr[rax + 0x1E0], rcx);

				ret();

				// pop stack frame and set ZF
				L("otherReturn");

				add(rsp, 32);
				pop(rbp);

				pop(rax);
				pop(rcx);

				xor(rdx, rdx);

				ret();
			}
		} clothFixStub1;

		auto location = hook::get_pattern("74 66 48 39 88 E0 01 00 00 74 5D", 2);
		hook::nop(location, 7);
		hook::call_reg<2>(location, clothFixStub1.GetCode());
	}
});
