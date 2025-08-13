#include <StdInc.h>

#include <Hooking.h>

//
// When a network object fails to properly register (when exhuasting all net Ids, or running out of pool space) entities fail to be replicated at all or to certain players
// Leading to cases where a entity that is not properly registered attacks a player/ped owned by a client that is not aware of the client leading to a nullptr deref.
// 

static HookFunction hookFunction([]
{
	// Check if the netObject is a nullptr
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
				jz("fail");

				// Original code, gets the gameObject and checks if thats a null
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
	}

	// In some situations, usually when a ped is actively being created/destroyed on the client. ped components can be invalid
	{
		auto location = hook::get_pattern("48 23 C1 8B 88 ? ? ? ? C1 E1 ? 85 C9 0F 8E ? ? ? ? B8");

		static struct : jitasm::Frontend
		{
			uintptr_t retnSuccess;
			uintptr_t retnFail;
			uint32_t compOffset;

			void Init(uintptr_t success, uintptr_t fail, uint32_t componentOffset)
			{
				retnSuccess = success;
				retnFail = fail;
				compOffset = componentOffset;
			}

			virtual void InternalMain() override
			{
				// Original code
				test(rcx, rcx);
				jz("fail");

				and(rax, rcx);
				test(rax, rax);
				jz("fail");

				mov(ecx, dword_ptr[rax + compOffset]);

				mov(r11, retnSuccess);
				jmp(r11);

				L("fail");
				mov(r11, retnFail);
				jmp(r11);
			}
		} patchStubPedComponents;

		const uintptr_t retnSuccess = (uintptr_t)location + 9;
		const uintptr_t retnFail = (uintptr_t)hook::get_pattern("33 C0 E9 ? ? ? ? 48 8B 00 FF 90");
		uint32_t pedComponentOffset = *hook::get_pattern<uint32_t>("8B 88 ? ? ? ? C1 E1 ? 85 C9 0F 8E ? ? ? ? B8", 2);

		hook::nop(location, 9);
		patchStubPedComponents.Init(retnSuccess, retnFail, pedComponentOffset);
		hook::jump_reg<5>(location, patchStubPedComponents.GetCode());	
	}
});
