#include "StdInc.h"
#include "Hooking.h"

static HookFunction hookFunction([]()
{
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			// test whether or not we have to skip
			mov(rax, qword_ptr[rdi + 0x830]);				// rax = rdi+_CVehicle.handling
			test(dword_ptr[rax + 0x120], 0x2000000);		// if (rax+_CHandlingData.strHandlingFlags & 0x2000000)
			jnz("skip_entirely");							//     goto skip_entirely

			// run original code
			cmp(ebx, dword_ptr[rdi + 0xA98]);				// original code we overwrote
			ret();											// return;

			// skipping label
			L("skip_entirely");								// skip_entirely:
			mov(rax, qword_ptr[rsp]);						// rax = _ReturnAddress()
			add(rax, 0x3E - 5);								// rax += (0x3E - 5)
															// difference between end of CALL and the ending of the branch

			mov(qword_ptr[rsp], rax);						// *_AddressOfReturnAddress() = rax
			ret();											// return;
		}
	} tractionControlSkipStub;

	auto location = hook::get_pattern("73 69 3B 9F ? ? 00 00 7D 0D", 0x6B);
	hook::nop(location, 6);
	hook::call(location, tractionControlSkipStub.GetCode());
});