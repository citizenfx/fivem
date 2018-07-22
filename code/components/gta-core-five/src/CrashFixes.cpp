/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <Error.h>

#include <LaunchMode.h>

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

static void(*g_origUnsetGameObj)(void*, void*, bool, bool);

static void UnsetGameObjAndContinue(void* netobj, void* ped, bool a3, bool a4)
{
	*(void**)((char*)netobj + 80) = nullptr;

	g_origUnsetGameObj(netobj, ped, a3, a4);
}

struct EntryPointInfo
{
	void* info;
	void* animInfo;
};

struct VehicleLayoutInfo
{
	char pad[24];
	EntryPointInfo* EntryPoints_data;
	uint16_t EntryPoints_size;
};

static bool VehicleEntryPointValidate(VehicleLayoutInfo* info)
{
	for (size_t i = 0; i < info->EntryPoints_size; ++i)
	{
		if (info->EntryPoints_data[i].info == nullptr)
		{
			info->EntryPoints_size = 0;
		}
	}

	return true;
}

#include <atPool.h>

static void(*g_origUnloadMapTypes)(void*, uint32_t);

void fwMapTypesStore__Unload(char* assetStore, uint32_t index)
{
	auto pool = (atPoolBase*)(assetStore + 56);
	auto entry = pool->GetAt<char>(index);

	if (entry != nullptr)
	{
		if (*(uintptr_t*)entry != 0)
		{
			g_origUnloadMapTypes(assetStore, index);
		}
		else
		{
			AddCrashometry("maptypesstore_workaround_2", "true");
		}
	}
	else
	{
		AddCrashometry("maptypesstore_workaround", "true");
	}
}

static int ReturnFalse()
{
	return 0;
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

	if (!CfxIsSinglePlayer())
	{
		// unknown function doing 'something' to scrProgram entries for a particular scrThread - we of course don't have any scrProgram
		hook::jump(hook::pattern("8B 59 14 44 8B 79 18 8B FA 8B 51 0C").count(1).get(0).get<void>(-0x1D), ReturnInt<-1>);
	}

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
			// valid for 1032 as well
			movzx(ebx, word_ptr[rcx + 0x668]);
			ret();

			L("justReturn");
			mov(ebx, 0);
			ret();
		}
	} carFixStub;

	{
		auto location = hook::get_pattern("0F B7 99 ? ? 00 00 EB 38", 0);
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
			// valid for 1103 as well
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

	// netobj cloning: door count check
	// signature (1103): FiveM.exe@0xefeae1
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			mov(rax, qword_ptr[rax + 0xB0]);
			test(rax, rax);
			jz("justReturn");

			add(rax, 8);

			L("justReturn");
			ret();
		}
	} carFixStub3;

	{
		auto location = hook::get_pattern("48 8B 80 B0 00 00 00 48 83 C0 08 74 43", 0);
		hook::nop(location, 11);

		// register 2: dx
		hook::call_reg<2>(location, carFixStub3.GetCode());
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
		static bool VerifyPedInst(char* inst)
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

				trace("Bound used in CPed collision function with instance %s (frag %s) was of type %s, but should be phBoundComposite! ignoring...\n", instType, frag, boundType);

				return false;
			}

			return true;
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

			test(al, al);
			jz("otherReturn");

			add(rsp, 32);
			pop(rbp);

			pop(rax);
			pop(rdx);

			test(dword_ptr[rdx + rax * 8], 0x100000);

			ret();

			L("otherReturn");

			add(rsp, 32);
			pop(rbp);

			pop(rax);
			pop(rdx);

			// set ZF, primarily
			xor(rdx, rdx);

			ret();
		}
	} colFixStub1;

	{
		auto location = hook::get_pattern("F7 04 C2 00 00 10 00 74 5A");
		hook::nop(location, 7);
		hook::call_reg<1>(location, colFixStub1.GetCode());
	}

	// crash signature hash (1103): october-michigan-lithium (allocator corruption)
	// CPed destruction in CNetObjPlayer creation: unset netobj gameobject
	{
		auto location = hook::get_pattern("74 24 41 B1 01 45 33 C0 48 8B D3 48 8B CF E8", 14);
		hook::set_call(&g_origUnsetGameObj, location);
		hook::call(location, UnsetGameObjAndContinue);
	}

	// crash signature hash (1103): chicken-harry-undress
	// vehicle gadget attachment(?) used in towtrucks, CObject matrix stuff
	static const float dummy_matrices[] = {
		// 0
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		// 64
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		// 128
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		// 192
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		// 256
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		// 320
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			sub(rsp, 8);

			test(rax, rax);
			jz("returnDummy");

			mov(rax, qword_ptr[rax + 0x10]);
			jmp("return");

			L("returnDummy");
			mov(rax, (uintptr_t)&dummy_matrices);

			L("return");
			movss(xmm0, dword_ptr[rbp + 0x60]);

			add(rsp, 8);
			ret();
		}
	} gadgetFixStub1;

	{
		auto location = hook::get_pattern("48 8B 40 10 F3 0F 10 45 60 F3 0F 10 8D 80 00 00");
		hook::nop(location, 9);
		hook::call_reg<1>(location, gadgetFixStub1.GetCode());
	}

	// initialization of vehiclelayouts.meta, VehicleLayoutInfos->EntryPoints validation
	// causes ErrorDo signature because of null pointers - we'll just replace validation and force the array to be size 0
	hook::jump(hook::get_pattern("44 0F B7 41 20 33 D2 45"), VehicleEntryPointValidate);

	static uint64_t lastClothValue;
	static char* lastClothPtr;

	// cloth data in vehicle audio, validity check
	// for crash sig @4316ee
	static struct : jitasm::Frontend
	{
		static void ReportCrash()
		{
			static bool hasCrashedBefore = false;

			if (!hasCrashedBefore)
			{
				hasCrashedBefore = true;

				trace("WARNING: cloth data crash triggered (invalid pointer: %016llx, dummy: %016llx)\n", (uintptr_t)lastClothPtr, (uintptr_t)lastClothValue);

				AddCrashometry("cloth_data_crash", "true");
			}
		}

		static bool VerifyClothDataInst(char* pointer)
		{
			__try
			{
				lastClothValue = *(uint64_t*)(pointer + 0x1E0);

				return true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				lastClothPtr = pointer;
				ReportCrash();

				return false;
			}
		}

		void InternalMain() override
		{
			// save rcx and rax as these are our instruction operands
			push(rcx);
			push(rax);

			// set up a stack frame for function calling
			push(rbp);
			mov(rbp, rsp);
			sub(rsp, 32);

			// call VerifyClothDataInst with rax (the pointer to probe)
			mov(rcx, rax);
			mov(rax, (uintptr_t)&VerifyClothDataInst);
			call(rax);

			// if 0, it's invalid
			test(al, al);
			jz("otherReturn");

			// pop stack frame and execute original cmp instruction
			add(rsp, 32);
			pop(rbp);

			pop(rax);
			pop(rcx);

			cmp(qword_ptr[rax + 0x1E0], rcx);

			ret();

			// pop stack frame and set ZF
			L("otherReturn");

			add(rsp, 32);
			pop(rbp);

			pop(rax);
			pop(rcx);

			xor(rdx, rdx);

			ret();
		}
	} clothFixStub1;

	{
		auto location = hook::get_pattern("74 66 48 39 88 E0 01 00 00 74 5D", 2);
		hook::nop(location, 7);
		hook::call_reg<2>(location, clothFixStub1.GetCode());
	}

	// fix some deeply nested call used by CVehicle destructor (related to CApplyDamage?) crashing uncommonly on a null pointer
	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			test(rdx, rdx);
			jz("returnElse");

			mov(rdx, qword_ptr[rdx + 0xE8]);
			ret();

			L("returnElse");
			add(qword_ptr[rsp], 0xC + 2); // call is 5 bytes, original insn is 7 bytes - we skip the NOPs, CALL and other MOV instruction which are 12 bytes
			ret();
		}
	} vehicleDamageFixStub1;

	{
		auto location = hook::get_pattern("48 8B 92 E8 00 00 00 48 8B 8B", 0);
		hook::nop(location, 7);
		hook::call_rcx(location, vehicleDamageFixStub1.GetCode());
	}

	// fix STAT_SET_INT saving for unknown-typed stats directly using stack garbage as int64
	hook::put<uint16_t>(hook::get_pattern("FF C8 0F 84 85 00 00 00 83 E8 12 75 6A", 13), 0x7EEB);

	// fwMapTypesStore double unloading workaround
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("4C 63 C2 33 ED 46 0F B6 0C 00 8B 41 4C", -18), fwMapTypesStore__Unload, (void**)&g_origUnloadMapTypes);
	MH_EnableHook(MH_ALL_HOOKS);

	// disable TXD script resource unloading to work around a crash
	{
		auto vtbl = hook::get_address<void**>(hook::get_pattern("BA 07 00 00 00 48 8B D9 E8 ? ? ? ? 48 8D 05", 16));
		hook::return_function(vtbl[5]);
	}

	// always create OffscreenBuffer3 so that it can't not exist at CRenderer init time (FIVEM-CLIENT-1290-F)
	hook::put<uint8_t>(hook::get_pattern("4C 89 25 ? ? ? ? 75 0E 8B", 7), 0xEB);
	
	// test: disable 'classification' compute shader users by claiming it is unsupported
	hook::jump(hook::get_pattern("84 C3 74 0D 83 C9 FF E8", -0x14), ReturnFalse);
});
