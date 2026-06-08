#include "StdInc.h"
#include <jitasm.h>
#include "Hooking.Patterns.h"

static HookFunction hookFunction([]()
{
	// CDynamicEntityDrawHandler::AddFragmentToDrawList
	// This function renders fragment entities that have a cloth component attached
	//
	// The code assumes that any entity whose model type is MI_TYPE_VEHICLE is an
	// actual vehicle, and unconditionally sets isVehicleCloth = true. This causes
	// the draw handler to be cast to CVehicleDrawHandler* without verification.
	auto location = hook::get_pattern<char>("C6 85 ? ? ? ? ? E9 ? ? ? ? B9");
	auto flagOffset = *(uint32_t*)(location + 0x2);

	static struct : jitasm::Frontend
	{
		uintptr_t continueLoc;
		uint32_t flagOffset;

		void Init(uintptr_t loc, uint32_t flagOff)
		{
			continueLoc = loc;
			flagOffset = flagOff;
		}
		
		virtual void InternalMain() override
		{
			mov(al, byte_ptr[r15+0x28]); // entity->Type
			cmp(al, 0x3);                // ENTITY_TYPE_VEHICLE
			jne("skipFlagSet");
			
			mov(byte_ptr[rbp+flagOffset], 1);
			L("skipFlagSet");
			
			mov(rax, continueLoc);
			jmp(rax);
		}
	} stub;
	
	stub.Init((uintptr_t)location + 7, flagOffset);
	
	hook::nop(location, 7);
	hook::jump(location, stub.GetCode());
});
