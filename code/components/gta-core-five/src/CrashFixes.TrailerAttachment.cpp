#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.FlexStruct.h>
#include <EntitySystem.h>

static rage::fwModelId* (*orig_CVehicleModelInfo_GetTrailer)(hook::FlexStruct*, rage::fwModelId*, uint32_t, void**);
static rage::fwModelId* CVehicleModelInfo_GetTrailer(hook::FlexStruct* self, rage::fwModelId* result, uint32_t idx, void** vmi)
{
	if (!self || !self->Get<void*>(0x2E0))
	{
		if (result)
		{
			*result = rage::fwModelId{};
		}
		if (vmi)
		{
			*vmi = nullptr;
		}
		return result;
	}
	return orig_CVehicleModelInfo_GetTrailer(self, result, idx, vmi);
}

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

	orig_CVehicleModelInfo_GetTrailer = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 7C 24 ? 55 48 8B EC 48 83 EC 30 8B 45 ? 49 8B F9 41 B9 FF FF 00 00"), &CVehicleModelInfo_GetTrailer);
});
