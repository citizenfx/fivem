/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <CrossBuildRuntime.h>

static HookFunction hookFunction([] ()
{
	auto matches = hook::pattern("04 01 00 00 E8 ? ? ? ? EB 17 48 8D 15").count(2);

	for (int i = 0; i < matches.size(); i++)
	{
		char* location = matches.get(i).get<char>(-11);

		strcpy(hook::get_address<char*>(location), "fivem_set.bin");

		location = matches.get(i).get<char>(14);

		strcpy(hook::get_address<char*>(location), "fxu:/fivem_set.bin");

	}

	// testing hook to not kill blip data when changing to a network game
	{
		// #TODO2060: compiler redid it, 0x1401C16B0 
		if (!Is2060())
		{
			char* location = hook::pattern("84 C0 0F 84 ? 00 00 00 40 38 3D ? ? ? ? 48").count(1).get(0).get<char>();

			static struct : public jitasm::Frontend
			{
				void* retLoc;
				void* cmpLoc;

				void InternalMain() override
				{
					mov(rax, reinterpret_cast<uintptr_t>(cmpLoc));
					mov(eax, dword_ptr[rax]);

					cmp(eax, 0xFFFFFFFF);
					jne("doReturn");

					mov(qword_ptr[rsp + 8], rbx);
					mov(rax, reinterpret_cast<uintptr_t>(retLoc));

					jmp(rax);

					L("doReturn");
					ret();
				}
			} stub;

			stub.retLoc = (location - 0x69 + 5);

			char* cmpLocation = location - 28;
			stub.cmpLoc = (void*)(cmpLocation + *(int32_t*)cmpLocation + 4);

			hook::jump(location - 0x69, stub.GetCode());
		}
	}
});
