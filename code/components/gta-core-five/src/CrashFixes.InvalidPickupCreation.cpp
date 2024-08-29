#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

static HookFunction hookFunction([]
{
	// Fixes a crash trying to set a field on a null entity
	auto location = hook::get_pattern<char>("FF 90 ? ? ? ? 66 89 98");

	static struct : jitasm::Frontend
	{
		int32_t writeOffset;
		intptr_t retLocation;

		void Init(intptr_t location)
		{
			writeOffset = *(int32_t*)(location + 9);
			retLocation = location + 13;
		}

		void InternalMain() override
		{
			test(rax, rax);
			jz("end");

			mov(word_ptr[rax + writeOffset], bx);

			L("end");

			mov(rax, retLocation);
			jmp(rax);
		}
	} patchStub;

	patchStub.Init(reinterpret_cast<intptr_t>(location));
	hook::jump_rcx(location + 6, patchStub.GetCode());
});
