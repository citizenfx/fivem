/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

static void* ProbePointer(char* pointer)
{
	__try
	{
		return *(void**)pointer;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		static bool hasCrashedBefore = false;

		if (!hasCrashedBefore)
		{
			hasCrashedBefore = true;

			trace("WARNING: TXD store reference crash triggered (invalid pointer: %p)\n", pointer);
		}

		return nullptr;
	}
}

template<int Integer>
static int ReturnInt()
{
	return Integer;
}

static HookFunction hookFunction([] ()
{
	// corrupt TXD store reference crash (ped decal-related?)
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			// original code bits
			and(rdx, rax);

			// move as argument (doing this inline and using 'call' breaks for some reason)
			mov(rcx, rdx);

			// jump to the probing function
			mov(rax, (uintptr_t)ProbePointer);
			jmp(rax);
		}
	} txdFixStub;

	void* txdFixStubLoc = hook::pattern("48 23 D0 48 8B 02 48 85 C0 74 7F 48").count(1).get(0).get<void>(0);
	hook::nop(txdFixStubLoc, 6);
	hook::call_rcx(txdFixStubLoc, txdFixStub.GetCode()); // call_rcx as the stub depends on rax being valid

	// unknown function doing 'something' to scrProgram entries for a particular scrThread - we of course don't have any scrProgram
	hook::jump(hook::pattern("8B 59 14 44 8B 79 18 8B FA 8B 51 0C").count(1).get(0).get<void>(-0x1D), ReturnInt<-1>);
});