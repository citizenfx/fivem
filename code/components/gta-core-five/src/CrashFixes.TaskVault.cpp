#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

static HookFunction hookFunction([]
{
	// mov rax, [rax+0xD0]
	auto location = hook::get_pattern<char>("48 8B 80 D0 00 00 00 41 0F B7 CE 48 8B 0C C8 0F B7 51 12 48 8B 4E 78");

	auto failLocation = hook::get_pattern("48 8B 6C 24 ? 48 8B 74 24 ? 0F B7 C3 48 8B 5C 24 ? 48 83 C4 ? 41 5F 41 5E 5F C3");

	static struct : jitasm::Frontend
	{
		uintptr_t retSuccess;
		uintptr_t retFail;

		void Init(uintptr_t location, uintptr_t failLocation)
		{
			// resume right after the instruction we patched over
			this->retSuccess = location + 7;
			this->retFail = failLocation;
		}

		void InternalMain() override
		{
			movzx(ecx, r14w);
			movzx(edx, byte_ptr[rax + 286]); // fragPhysicsLOD->m_NumChildren
			cmp(ecx, edx);                   // if (index >= m_NumChildren) bail
			jae("fail");

			mov(rax, qword_ptr[rax + 208]); // continue with the original code path
			mov(r11, retSuccess);
			jmp(r11);

			L("fail");
			mov(r11, retFail);
			jmp(r11);
		}
	} patchStub;

	patchStub.Init(reinterpret_cast<uintptr_t>(location), reinterpret_cast<uintptr_t>(failLocation));

	hook::nop(location, 7);
	hook::jump_rcx(location, patchStub.GetCode()); // Need to preserve rax, so jump via rcx
});
