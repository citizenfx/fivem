#include <StdInc.h>

#include <Hooking.h>

//
// When a network object fails to properly register (when exhuasting all net Ids, or running out of pool space) entities fail to be replicated at all or to certain players
// Leading to cases where a entity that is not properly registered attacks a player/ped owned by a client that is not aware of the client leading to a nullptr deref.
// 

static HookFunction hookFunction([]
{
	auto location = hook::get_pattern<char>("49 8B 0F 48 8B 01 FF 90 ? ? ? ? 48 85 C0");

	static struct : jitasm::Frontend
	{
		uintptr_t retnSuccess;
		uintptr_t retnFail;

		void Init(uintptr_t success, uintptr_t fail)
		{
			retnSuccess = success;
			retnFail = fail;
		}

		virtual void InternalMain() override
		{
			// Original code
			test(r15, r15); 
			jz("fail");

			mov(rcx, qword_ptr[r15]);
			test(rcx, rcx);

			jz("fail");

			mov(rax, qword_ptr[rcx]);
			test(rax, rax);

			mov(r11, retnSuccess);
			jmp(r11);

			L("fail");
			mov(r11, retnFail);
			jmp(r11);
		}
	} patchStub;

	const uintptr_t retnSuccess = (uintptr_t)location + 6;
	const uintptr_t retnFail = (uintptr_t)hook::get_pattern("33 C0 E9 ? ? ? ? 48 8B 00 FF 90");

	hook::nop(location, 6);
	patchStub.Init(retnSuccess, retnFail);
	hook::jump_reg<5>(location, patchStub.GetCode());
});
