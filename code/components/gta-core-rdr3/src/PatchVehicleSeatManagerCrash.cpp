#include <StdInc.h>

#include "atArray.h"
#include "Hooking.h"
#include "Hooking.Stubs.h"

static HookFunction hookFunction([]()	
{
	auto location = hook::get_pattern("48 8B D6 48 8D 48 ? E8 ? ? ? ? 3B 83");
	static struct : jitasm::Frontend
	{
		uintptr_t retnSuccess;
		uintptr_t retnFailure;

		void Init(uintptr_t success, uintptr_t failure)
		{
			retnSuccess = success;
			retnFailure = failure;
		}

		static void LogSeatError(hook::FlexStruct* self)
		{
			hook::FlexStruct* entity = self->At<hook::FlexStruct*>(0xB8);
			uint16_t entityId = entity->At<uint16_t>(0x10);
			trace("Failed to retrieve SeatManager info for vehicle %i (%p)\n", entityId, (void*)hook::get_unadjusted(_ReturnAddress()));
		}

		virtual void InternalMain() override
		{
			// Original Code
			mov(rdx, rsi);
			mov(rcx, qword_ptr[rax + 0x20]);

			test(rcx, rcx);
			jz("Fail");

			mov(r11, retnSuccess);
			jmp(r11);

			L("Fail");

			sub(rsp, 0x28);
			mov(rcx, rdi);

			mov(r11, reinterpret_cast<uintptr_t>(&LogSeatError));
			jmp(r11);
			add(rsp, 0x28);

			mov(r11, retnFailure);
			jmp(r11);

		}
	} patchStub;

	const uintptr_t retnSuccess = (uintptr_t)location + 7;
	const uintptr_t retnFailure = (uintptr_t)location + 0x18;

	hook::nop(location, 7);
	patchStub.Init(retnSuccess, retnFailure);
});
