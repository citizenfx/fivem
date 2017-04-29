/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include <minhook.h>

//
// I wanted to put a funny story here, but this issue is pretty sad.
//
// Basically, AMD's 'Crimson' driver release ('15.11', not to be confused with 'Catalyst 15.11.1' which is actually an older branch) caused crashes in
// the D3D11 UMD for a typical FiveM scenario, on a background thread used for *whatever* processing.
//
// In a day-ish, this turned into the second most common crash in the crash statistics, affecting pretty much everyone with an AMD GPU who bothers
// to update their drivers.
//
// Of course, as there's no debugging symbols, RTTI nor even a bug reporting forum (Intel at least has the latter), and common stories exist of
// AMD/ATi not fixing bugs reported by small developers until *years* later, let alone an 'unauthorized' game modification, there was theoretically
// 'nothing' I could do.
//
// However, simply try/catching the memmove call that tries to write to a 'corrupted' address seems to work reliably to fix this crash (if there were RTTI
// I could've found out what exactly crashes, but of course there isn't), and it only seems to be invoked around twice in the game's runtime, so it doesn't
// really affect performance.
//
// Of course, if AMD doesn't fix this issue, and yet changes the function's pattern/the used compiler (from 15.11.1 to 15.11 the function's register allocation
// changed a *lot* already!), this workaround will come back to bite us in the future.
//
// For now, it works, though, so that'd make the users (and me, as I use a GCN GPU myself as well, at the time of this writing) happy.
//
// -- NTAuthority, 2015-11-26
//

static void* ProtectedMemmove(void* dest, const void* src, int size)
{
	__try
	{
		return memmove(dest, src, size);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// weird musical reference
		trace("have you heard about the storm?\n");
		trace("MOOOOOM, AMD IS DOING IT AGAIN\n");
		return nullptr;
	}
}

static InitFunction initFunction([] ()
{
	// load the ATi/AMD D3D11 UMD
	HMODULE amdPresent = LoadLibrary(L"atidxx64.dll");

	// if it's present
	if (amdPresent)
	{
		// look for a pattern in the function
		auto pattern = hook::module_pattern(amdPresent, "8B 83 ? ? 00 00 8B CF D3 E8 A8 ? 74");

		if (pattern.size() == 1)
		{
			// this pattern is pretty universal, though 15.11.1 uses 0xF here and 15.11 uses 0x1 (and before you ask,
			// changing it back to 0xF doesn't help)
			uint8_t* byteCheck = pattern.get(0).get<uint8_t>(11);

			if (*byteCheck == 1)
			{
				// log a message for posterity
				trace("patching atidxx64.dll crimson(r) edition\n");

				// offset to the first memmove call
				byteCheck += 71;

				// if it's a call...
				if (byteCheck[0] == 0xE8)
				{
					// and patch memmove (using minhook, so we don't have to handle allocating 32-bit relative jump stubs ourselves)
					void* memmovePoint = hook::get_call(byteCheck);

					static void* orig;
					MH_CreateHook(memmovePoint, ProtectedMemmove, &orig);
					MH_EnableHook(MH_ALL_HOOKS);
				}
			}
		}
	}
});

// minhook init bits
struct InitMHWrapper
{
	InitMHWrapper()
	{
		MH_Initialize();
	}

	~InitMHWrapper()
	{
		MH_Uninitialize();
	}
};

InitMHWrapper mh;