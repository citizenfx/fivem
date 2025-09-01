#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <jitasm.h>

static HookFunction hookFunction([]
{
	auto location = hook::get_pattern<uint8_t>("48 8B 82 ? ? ? ? 8B 48 ? 89 8F");

	static struct : jitasm::Frontend
	{
		intptr_t location;
		intptr_t retSuccess;
		intptr_t retFail;

		uint32_t compositeBoundFlagsArrayOffset;

		void Init(intptr_t location)
		{
			this->location = location;
			this->retSuccess = location + 10;
			this->retFail = location + 30;

			this->compositeBoundFlagsArrayOffset = *(uint32_t*)(location + 3);
		}

		void InternalMain() override
		{
			mov(rax, qword_ptr[rdx + this->compositeBoundFlagsArrayOffset]); // [original code]
																			 //
			test(rax, rax); // if ( flagArray )
			jz("fail"); // {
						//
			mov(ecx, dword_ptr[rax + 4 /* hardcoded, hasn't changed */]); //        [original code]
																		  //
			mov(rax, retSuccess); //
			jmp(rax); // }
					  //
			L("fail"); //
					   //
			mov(rax, retFail); //
			jmp(rax); //
		}
	} patchStub;

	patchStub.Init(reinterpret_cast<intptr_t>(location));
	/**
	 * nop's:
	 *
	 *    mov     rax, [rdx+90h]
	 *    mov     [rax+4], ecx
	 */
	hook::nop(location, 10);
	hook::jump(location, patchStub.GetCode());
});
