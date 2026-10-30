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

// same parachute state handler, a bit further down: the pointer at parachuteObject+0x48 isn't
// null checked before reading +0x20 from it. a synced ruiner2 with that pointer missing crashes
// everyone around it (b3258: +F890F3). the code right after already handles rcx being null, so
// if the pointer is missing we just hand it a null rcx.
static HookFunction hookFunctionParachuteObject([]()
{
	// mov r8, [rdi]; mov rax, [r8+48h]; mov rcx, [rax+20h]
	// only checked on 3258, skip on builds where the code looks different
	auto matches = hook::pattern("4C 8B 07 49 8B 40 48 48 8B 48 20 48 85 C9 74 0A").count_hint(1);

	if (matches.empty())
	{
		return;
	}

	auto location = matches.get_first<char>();

	static struct : jitasm::Frontend
	{
		uintptr_t retAddr;

		void Init(uintptr_t location)
		{
			retAddr = location + 11; // test rcx, rcx
		}

		void InternalMain() override
		{
			mov(r8, qword_ptr[rdi]);            // parachuteObject
			mov(rax, qword_ptr[r8 + 0x48]);     //
                                                //
			test(rax, rax);                     // if (*(parachuteObject + 0x48))
			jz("fail");                         // {
                                                //
			mov(rcx, qword_ptr[rax + 0x20]);    //      [original code]
			mov(rax, retAddr);                  //
			jmp(rax);                           //
                                                // }
			L("fail");                          //
                                                //
			mov(rcx, 0);                        // null, skipped by the test rcx, rcx we return to
			mov(rax, retAddr);                  //
			jmp(rax);                           //
		}
	} patchStub;

	patchStub.Init(reinterpret_cast<uintptr_t>(location));

	hook::jump(location, patchStub.GetCode());
});
