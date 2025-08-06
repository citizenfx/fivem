#include "StdInc.h"

#include <jitasm.h>
#include "Hooking.Patterns.h"

#include "CrossBuildRuntime.h"

static HookFunction hookFunction([]()
{
	// Game build 3570 includes built-in validation for this logic, so applying this patch is unnecessary.
	if (xbr::IsGameBuildOrGreater<3570>())
	{
		return;
	}
	// CTaskClimbLadder::IsMovementBlocked
	// This function determines wether the ped can continue climbing the ladder
	// or if its movement should be blocked due some physical obstacle.
	//
	// The code assumes that the ped is a biped and tries to read the biped capsule
	// this leads to a crash when the ped is not a biped (e.g., animals, props, 
	// fish, etc.) because `GetBipedCapsuleInfo()` returns `nullptr`, and the 
	// code tries to read from it.
	
	auto location = hook::get_pattern<char>("F3 0F 10 40 ? 8A 42");
	auto failLocation = hook::get_pattern("4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 49 8B 73 ? 41 0F 28 73 ? 41 0F 28 7B ? 45 0F 28 43 ? 49 8B E3 41 5E 41 5D");

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
			// Restore original rax value
			mov(al, byte_ptr[rdx+0x0C]);
			dec(al);
			cmp(al, 1);
			setbe(cl);
			neg(cl);
			sbb(rax, rax);
			and(rax, rdx);

			test(rax, rax);
			jz("fail");

			movss(xmm0, dword_ptr[rax+0x64]);
			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			mov(rax, failLocation);
			jmp(rax);
		}
	} stub;
	
	assert(((intptr_t)failLocation - (intptr_t)location) < 2000);
	stub.Init((uintptr_t)location + 5, (uintptr_t)failLocation);
	
	hook::nop(location, 5);
	hook::jump(location, stub.GetCode());
});