#include "StdInc.h"
#include "Hooking.h"

#include "scrEngine.h"

static HookFunction hookFunction([] ()
{
	// CGameScriptHandlerObject::GetOwner vs. GetCurrentScriptHandler checks

	// common pattern:
	// call qword ptr [r?x + 38h]
	// mov rbx, rax
	// call GetGameScriptHandler
	// cmp rbx, rax

	// this pattern starts at the + 38h portion, and matches 7 functions, all of which are script calls.
	// these include:
	// - DELETE_ENTITY
	// - DOES_ENTITY_BELONG_TO_THIS_SCRIPT
	// - the common group of:
	//   - SET_ENTITY_AS_NO_LONGER_NEEDED
	//   - SET_PED_AS_NO_LONGER_NEEDED
	//   - SET_VEHICLE_AS_NO_LONGER_NEEDED
	//   - SET_OBJECT_AS_NO_LONGER_NEEDED
	//   (these all use the same function)
	// - DELETE_OBJECT
	// - DELETE_PED
	// - REMOVE_PED_ELEGANTLY
	{
		auto pattern = hook::pattern("38 48 8B D8 E8 ? ? ? ? 48 3B D8").count(7);

		for (int i = 0; i < 7; i++)
		{
			// is this DOES_ENTITY_BELONG_TO_THIS_SCRIPT?
			if (*pattern.get(i).get<uint16_t>(13) == 0xB004) // jnz $+4 (the 04 byte); mov al, 1 (the B0 byte)
			{
				// if so, then ship
				continue;
			}

			hook::nop(pattern.get(i).get<void>(0xC), 2);
		}
	}
});