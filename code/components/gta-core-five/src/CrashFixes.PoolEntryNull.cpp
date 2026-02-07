#include "StdInc.h"
#include <jitasm.h>
#include "Hooking.Patterns.h"

static HookFunction hookFunction([]
{
	auto patchLocation = hook::get_pattern<char>("8B C8 E8 ? ? ? ? 8B 90 DC 00 00 00", 7);
	auto exitPath = hook::get_pattern<char>("0F 5C 8F ? ? ? ? 0F 59 4F", -7);

	static struct : jitasm::Frontend
	{
		uintptr_t continueAddr;
		uintptr_t exitAddr;

		void Init(uintptr_t cont, uintptr_t exit)
		{
			continueAddr = cont;
			exitAddr = exit;
		}

		void InternalMain() override
		{
			test(rax, rax);
			jz("skip");

			mov(edx, dword_ptr[rax + 0xDC]);
			test(edx, edx);

			mov(rax, continueAddr);
			jmp(rax);

			L("skip");
			mov(rax, exitAddr);
			jmp(rax);
		}
	} nullCheckStub;

	nullCheckStub.Init((uintptr_t)patchLocation + 8, (uintptr_t)exitPath);

	hook::nop(patchLocation, 8);
	hook::jump(patchLocation, nullCheckStub.GetCode());
});
