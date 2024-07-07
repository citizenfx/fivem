#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

static HookFunction hookFunction([]
{
	// Fixes a crash when an entity collides and has no drawhandler ( when invisible for e.g )

	auto location = hook::get_pattern<uint8_t>("48 8B 58 ? EB ? 48 8B D9");

	static struct : jitasm::Frontend
	{
		intptr_t location;
		intptr_t retSuccess;
		intptr_t retFail;

		void Init(intptr_t location)
		{
			this->location = location;
			this->retSuccess = location + 0x9;
			this->retFail = location + 0x30;
		}

		void InternalMain() override
		{
			mov(rax, qword_ptr[rsi + 0x50]);	// rax = rsi + 0x50;
			mov(rbx, qword_ptr[rax + 0x30]);	// creature = rax + 0x30;
												//
			test(rbx, rbx);						// if ( creature ) 
			jz("fail");							// {
												//
			mov(rax, retSuccess);				//		[run original code]
			jmp(rax);							//
												//
			L("fail");							//	}
			mov(rax, retFail);					//
			jmp(rax);							//
		}
	} patchStub;

	patchStub.Init(reinterpret_cast<intptr_t>(location));
	hook::nop(location, 6);
	hook::jump(location, patchStub.GetCode());
});
