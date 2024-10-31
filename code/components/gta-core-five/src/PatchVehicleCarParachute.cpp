#include "StdInc.h"

#include <Hooking.h>
#include <jitasm.h>

static HookFunction hookFunction([]()
{
	// mov rcx, [rax+30h]
	auto location = hook::get_pattern<char>("48 8B 48 ? 48 8B 49 ? 48 85 C9 74 ? BA");

	static struct : jitasm::Frontend
	{
		uintptr_t retSuccess;
		uintptr_t retFail;

		void Init(uintptr_t location)
		{
			retSuccess = location +  8;	// test rcx, rcx
			retFail	   = location + 23; // movzx eax, byte ptr [rbx+1A8Ch]
		}

		void InternalMain() override
		{
			mov(rcx, qword_ptr[rax + 0x30]);    // parachutePhysInst = *(parachuteObject + 0x30);
                                                //
			test(rcx, rcx);                     // if (parachutePhysInst)
			jz("fail");                         // {
                                                //
			mov(rcx, qword_ptr[rcx + 0x10]);    //      parachuteArchetype = *(parachutePhysInst + 0x10);
                                                //      
			mov(rax, retSuccess);               //      [original code] // which tests for a valid parachuteArchetype
			jmp(rax);                           //
                                                // }
			L("fail");                          //
                                                //
			mov(rax, retFail);                  //
			jmp(rax);                           //
		}
	} patchStub;

	patchStub.Init(reinterpret_cast<intptr_t>(location));

	hook::jump_rcx(location, patchStub.GetCode());
});
