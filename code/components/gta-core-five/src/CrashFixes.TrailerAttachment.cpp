#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

static HookFunction hookFunction([]
{
	// fixes a crash when a CTrailer is attached to an entity not deriving from CVehicle
	auto location = hook::get_pattern("48 85 F6 0F 84 85 00 00 00 41", 3);

	static struct : jitasm::Frontend
	{
		intptr_t location;
		intptr_t retSuccess;
		intptr_t retFail;

		void Init(intptr_t location)
		{
			this->location = location;
			this->retSuccess = location + 6;
			this->retFail = location + 6 + 0x85;
		}

		void InternalMain() override
		{
			test(rsi, rsi);					// if (rsi)
			jz("fail");						// {
											//
			cmp(byte_ptr[rsi + 40], 3);		//     if (rsi->type == vehicle) // new check
			jne("fail");					//     {
											//
			mov(rax, retSuccess);			//         [run original code]
			jmp(rax);						//
											//
			L("fail");						//     }
			mov(rax, retFail);				// }
			jmp(rax);						//
		}
	} patchStub;

	patchStub.Init(reinterpret_cast<intptr_t>(location));
	hook::nop(location, 6);
	hook::jump(location, patchStub.GetCode());
});
