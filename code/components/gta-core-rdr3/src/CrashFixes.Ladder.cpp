#include "StdInc.h"

#include <jitasm.h>
#include "Hooking.Patterns.h"

static HookFunction hookFunction([]()
{
	// CTaskClimbLadder::IsMovementBlocked
	// This function determines wether the ped can continue climbing the ladder
	// or if its movement should be blocked due some physical obstacle.
	//
	// The code assumes that the ped is a biped and tries to read the biped capsule
	// this leads to a crash when the ped is not a biped (e.g., animals, props, 
	// fish, etc.) because `GetBipedCapsuleInfo()` returns `nullptr`, and the 
	// code tries to read from it.

	auto location = hook::get_pattern<char>("F3 0F 10 98 ? ? ? ? 48 8B C7");
	auto failLocation = hook::get_pattern("E8 ? ? ? ? 40 8A C7 4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 49 8B 73 ? 49 8B 7B ? 41 0F 28 73 ? 49 8B E3", 0x8);

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			test(rax, rax);
			jz("fail");

			mov(rax, rdi);
			cmovbe(rax, r10);
			test(rax, rax);
			jz("fail");
			
			movss(xmm3, dword_ptr[rax+0xBC]); // Original instruction
			mov(r15, successLocation);
			jmp(r15);

			L("fail");
			mov(r15, failLocation);
			jmp(r15);
		}
	} stub;

	assert(((intptr_t)failLocation - (intptr_t)location) < 2000);
	stub.Init((uintptr_t)location + 8, (uintptr_t)failLocation);

	hook::nop(location, 8);
	hook::jump_rcx(location, stub.GetCode());
});