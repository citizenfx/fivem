/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <Error.h>

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

#include <MinHook.h>

static intptr_t(*origVerifyNetObj1)(void*, int);
static intptr_t(*origVerifyNetObj2)(void*, int);

intptr_t VerifyNetObj1(char* a1, int a2)
{
	if (*(void**)(a1 + 176) == 0)
	{
		return 0;
	}

	return origVerifyNetObj1(a1, a2);
}

intptr_t VerifyNetObj2(char* a1, int a2)
{
	if (*(void**)(a1 + 176) == 0)
	{
		return 0;
	}

	return origVerifyNetObj2(a1, a2);
}

struct VirtualBase
{
	virtual ~VirtualBase() {}
};

struct VirtualDerivative : public VirtualBase
{
	virtual ~VirtualDerivative() override {}
};

std::string GetType(void* d)
{
	VirtualBase* self = (VirtualBase*)d;

	std::string typeName = fmt::sprintf("unknown (vtable %p)", *(void**)self);

	try
	{
		typeName = typeid(*self).name();
	}
	catch (std::__non_rtti_object&)
	{

	}

	return typeName;
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

	// something related to netobjs, crash sig FiveM.exe@0x5cd7e4
	// (on disconnecting with certain cars)
	{
		auto matches = hook::pattern("41 0F B6 80 4A 03 00 00 3B D0").count(2);

		MH_CreateHook(matches.get(0).get<void>(-12), VerifyNetObj1, (void**)&origVerifyNetObj1);
		MH_CreateHook(matches.get(1).get<void>(-12), VerifyNetObj2, (void**)&origVerifyNetObj2);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// crash signature FiveM.exe@0x75c050
	// CPed collision handling with phInstFrag in certain cases (climbing on cars?)
	static struct : jitasm::Frontend
	{
		static void VerifyPedInst(char* inst)
		{
			char* archetype = *(char**)(inst + 16);
			char* bound = *(char**)(archetype + 32);

			// check if this is a composite bound
			if (*(uint8_t*)(bound + 16) != 10) // actually 10, but to verify this works
			{
				auto boundType = GetType(bound);
				auto instType = GetType(inst);

				std::string frag = "<unknown>";

				if (instType.find("frag") != std::string::npos)
				{
					char* fragment = *(char**)(inst + 120);

					if (fragment)
					{
						char* name = *(char**)(fragment + 0x58);

						if (name)
						{
							frag = name;
						}
					}
				}

				FatalError("Bound used in CPed collision function with instance %s (frag %s) was of type %s, but should be phBoundComposite!", instType, frag, boundType);
			}
		}

		void InternalMain() override
		{
			push(rdx);
			push(rax);

			push(rbp);
			mov(rbp, rsp);
			sub(rsp, 32);

			// rbx is the phInst
			mov(rcx, rbx);
			mov(rax, (uintptr_t)&VerifyPedInst);
			call(rax);

			add(rsp, 32);
			pop(rbp);

			pop(rax);
			pop(rdx);

			test(dword_ptr[rdx + rax * 8], 0x100000);

			ret();
		}
	} colFixStub1;

	{
		auto location = hook::get_pattern("F7 04 C2 00 00 10 00 74 5A");
		hook::nop(location, 7);
		hook::call_reg<1>(location, colFixStub1.GetCode());
	}
});
