/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

static void* ProbePointer(char* pointer)
{
	__try
	{
		return *(void**)pointer;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		static bool hasCrashedBefore = false;

		if (!hasCrashedBefore)
		{
			hasCrashedBefore = true;

			trace("WARNING: TXD store reference crash triggered (invalid pointer: %p)\n", (void*)pointer);
		}

		return nullptr;
	}
}

template<int Integer>
static int ReturnInt()
{
	return Integer;
}

static void*(*origGetModelInfo)(void*);
static void*(*origGetFrag)(void*);

static void* GetModelInfoOnlyIfFrag(void* data)
{
	void* retval = origGetModelInfo(data);

	if (retval)
	{
		if (origGetFrag(retval))
		{
			return retval;
		}
	}

	return nullptr;
}

static HookFunction hookFunction([] ()
{
	// corrupt TXD store reference crash (ped decal-related?)
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			// original code bits
			and(rdx, rax);

			// move as argument (doing this inline and using 'call' breaks for some reason)
			mov(rcx, rdx);

			// jump to the probing function
			mov(rax, (uintptr_t)ProbePointer);
			jmp(rax);
		}
	} txdFixStub;

	void* txdFixStubLoc = hook::pattern("48 23 D0 48 8B 02 48 85 C0 74 7F 48").count(1).get(0).get<void>(0);
	hook::nop(txdFixStubLoc, 6);
	hook::call_rcx(txdFixStubLoc, txdFixStub.GetCode()); // call_rcx as the stub depends on rax being valid

	// unknown function doing 'something' to scrProgram entries for a particular scrThread - we of course don't have any scrProgram
	hook::jump(hook::pattern("8B 59 14 44 8B 79 18 8B FA 8B 51 0C").count(1).get(0).get<void>(-0x1D), ReturnInt<-1>);

	// make sure a drawfrag dc doesn't actually run if there's no frag (bypass ERR_GFX_DRAW_DATA)
	// 505-specific, possibly
	{
		char* location = hook::get_pattern<char>("33 FF 48 85 F6 0F 84 ? ? 00 00 F7 83 BC 00 00", -5);
		hook::set_call(&origGetModelInfo, location - 16);
		hook::call(location - 16, GetModelInfoOnlyIfFrag);

		hook::set_call(&origGetFrag, location + 106);
	}

	// vehicle metadata unmount: don't die if a vehicle is not loaded
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			test(rcx, rcx);
			jz("justReturn");

			// 505-specific offset
			movzx(ebx, word_ptr[rcx + 0x668]);
			ret();

			L("justReturn");
			mov(ebx, 0);
			ret();
		}
	} carFixStub;

	{
		auto location = hook::get_pattern("0F B7 99 ? ? 00 00 EB 35", 0);
		hook::nop(location, 7);
		hook::call(location, carFixStub.GetCode());
	}

	// vehicle model unload: similar patch
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			test(rax, rax);
			jz("justReturn");

			// 505-specific offset
			cmp(qword_ptr[rax + 0x670], 0);

			L("justReturn");
			ret();
		}
	} carFixStub2;

	{
		auto location = hook::get_pattern("00 48 83 B8 ? 06 00 00 00 74 46 E8", 1);
		hook::nop(location, 8);

		// register 2: dx
		hook::call_reg<2>(location, carFixStub2.GetCode());
	}
});