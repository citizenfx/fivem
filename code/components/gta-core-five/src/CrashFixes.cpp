/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <Error.h>

#include <ICoreGameInit.h>
#include <gameSkeleton.h>

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

static int ReturnFalse()
{
	return 0;
}

static bool(*g_origLoadFromStructureChar)(void* parManager, const char* fileName, const char* extension, void* structure, void** outStruct, void* unk);

static bool LoadFromStructureCharHook(void* parManager, const char* fileName, const char* extension, void* structure, void** outStruct, void* unk)
{
	bool result = g_origLoadFromStructureChar(parManager, fileName, extension, structure, outStruct, unk);

	if (result && !*outStruct)
	{
		result = false;

		console::PrintError("gta:core", "Failed to parse %s in rage::parManager::LoadFromStructure(const char*, ...). Make sure this is a well-formed XML/PSO file.\n", fileName);
	}

	return result;
}

void(*g_origReinitRenderPhase)(void*);

static void* g_pendingReinit;

void ReinitRenderPhaseWrap(void* a)
{
	if (!Instance<ICoreGameInit>::Get()->HasVariable("shutdownGame"))
	{
		g_origReinitRenderPhase(a);
	}
	else
	{
		g_pendingReinit = a;
	}
}

static void(*g_origCVehicleModelInfo__init)(void* mi, void* data);

static hook::cdecl_stub<void*(uint32_t)> _getExplosionInfo([]()
{
	return hook::get_call(hook::get_pattern("BA 52 28 0C 03 85 D2 74 09 8B CA E8", 11));
});

void CVehicleModelInfo__init(char* mi, char* data)
{
	uint32_t explosionHash = *(uint32_t*)(data + 96);

	if (!explosionHash || !_getExplosionInfo(explosionHash))
	{
		explosionHash = HashString("explosion_info_default");
	}

	*(uint32_t*)(data + 96) = explosionHash;

	g_origCVehicleModelInfo__init(mi, data);
}

static uint32_t(*g_origScrProgramReturn)(void* a1, uint32_t a2);

static uint32_t ReturnIfMp(void* a1, uint32_t a2)
{
	if (Instance<ICoreGameInit>::Get()->HasVariable("storyMode"))
	{
		return g_origScrProgramReturn(a1, a2);
	}

	return -1;
}

#include <atArray.h>

struct grcVertexProgram
{
	void* vtbl;
	const char* name;
	char pad[568];
};

struct grmShaderProgram
{
	char pad[48];
	atArray<grcVertexProgram> vertexPrograms;
};

struct grmShaderFx
{
	void* padParams;
	grmShaderProgram* program;
};

static void(*g_origDrawModelGeometry)(grmShaderFx* shader, int a2, void* a3, int a4, bool a5);

static void DrawModelGeometryHook(grmShaderFx* shader, int a2, void* a3, int a4, bool a5)
{
	if (shader->program && shader->program->vertexPrograms.GetSize() && shader->program->vertexPrograms[0].name && strstr(shader->program->vertexPrograms[0].name, "_batch:") != nullptr)
	{
		return;
	}

	g_origDrawModelGeometry(shader, a2, a3, a4, a5);
}

static void(*g_origCText__UnloadSlot)(int slotId, bool a2);

static void CText__UnloadSlotHook(int slotId, bool a2)
{
	if (slotId > 20)
	{
		return;
	}

	g_origCText__UnloadSlot(slotId, a2);
}

static bool(*g_origCText__IsSlotLoaded)(void* text, int slot);

static bool CText__IsSlotLoadedHook(void* text, int slot)
{
	if (slot > 20)
	{
		return true;
	}

	return g_origCText__IsSlotLoaded(text, slot);
}

static void(*g_origCText__LoadSlot)(void* text, void* name, int slot, int a4);

static void CText__LoadSlotHook(void* text, void* name, int slot, int a4)
{
	if (slot > 20)
	{
		trace("REQUEST_ADDITIONAL_TEXT has a slot range of 0 to 19 (inclusive). Slot %d is out of this range, so it has been ignored.\n", slot);
		return;
	}

	return g_origCText__LoadSlot(text, name, slot, a4);
}

static HookFunction hookFunction{[] ()
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
		//hook::jump(hook::pattern("8B 59 14 44 8B 79 18 8B FA 8B 51 0C").count(1).get(0).get<void>(-0x1D), ReturnInt<-1>);
		MH_Initialize();
		MH_CreateHook(hook::pattern("8B 59 14 44 8B 79 18 8B FA 8B 51 0C").count(1).get(0).get<void>(-0x1D), ReturnIfMp, (void**)&g_origScrProgramReturn);
		MH_EnableHook(MH_ALL_HOOKS);
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

	// vehicles.meta explosionInfo field invalidity
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("4C 8B F2 4C 8B F9 FF 50 08 4C 8D 05", -0x28), CVehicleModelInfo__init, (void**)&g_origCVehicleModelInfo__init);
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

	// test: disable ERR_GEN_MAPSTORE_X error asserts (RDR3 seems to not have any assertion around these at all, so this ought to be safe)
	{
		auto location = hook::get_pattern<char>("CD 36 41 A8 E8", 4);
		hook::nop(location, 5);
		hook::nop(location + 15, 5);
	}

	// increase size of unknown ped-related pointer array (with 100 entries max)
	{
		auto location = hook::get_pattern<char>("B8 64 00 00 00 48 83 C3 10 66 89 43 FA");
		hook::put<uint32_t>(location + 1, 400);
		hook::put<uint32_t>(location - 12, 400 * sizeof(void*));
	}

	// don't initialize/update render phases right after a renderer reinitialization (this *also* gets done by gameSkeleton update normally)
	// if the game is unloaded, this'll fail because camManager is not initialized
	// 1604 signature: magnesium-september-wisconsin (FIVEM-CLIENT-1604-34)
	//                 massachusetts-skylark-black   (FIVEM-CLIENT-1604-3D) <- CPedFactory::ms_playerPed being NULL with same call stack in CPortalTracker
	{
		auto location = hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 DB 74 0C 48 8D 0D", 23);
		hook::set_call(&g_origReinitRenderPhase, location);
		hook::call(location, ReinitRenderPhaseWrap);

		if (!CfxIsSinglePlayer())
		{
			Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([]()
			{
				if (g_pendingReinit)
				{
					trace("Attempted a mode change during shutdown - executing it now...\n");

					g_origReinitRenderPhase(g_pendingReinit);
					g_pendingReinit = nullptr;
				}
			});
		}
	}

	// CScene_unk_callsBlenderM58: over 50 iterated objects (CEntity+40 == 5, CObject) will lead to a stack buffer overrun
	// 1604 signature: happy-venus-purple (FIVEM-CLIENT-1604-NEW-18G4)
	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				cmp(rbx, 50);
				jge("nope");

				mov(qword_ptr[rbp + rbx * 8 + 0x190], rax);
				ret();

				L("nope");

				dec(edi);
				dec(rbx);

				ret();
			}
		} objectArrayStub;

		auto location = hook::get_pattern("48 89 84 DD 90 01 00 00");
		hook::nop(location, 8);
		hook::call_rcx(location, objectArrayStub.GetCode());
	}

	// don't allow rendering grass_batch from plain geometry draw functions
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("4D 8B F0 44 8A 44 24 50 41 8B", -0x19), DrawModelGeometryHook, (void**)&g_origDrawModelGeometry);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// parser errors: rage::parManager::LoadFromStructure(const char*/fiStream*) returns true when LoadTree fails, and
	// only returns false if LoadFromStructure(parTreeNode*) fails
	// make it return failure state on failure of rage::parManager::LoadTree as well, and log the failure.
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("4C 8B EA 48 8B F1 E8 ? ? ? ? 40 B5 01 48 8B F8", -0x2D), LoadFromStructureCharHook, (void**)&g_origLoadFromStructureChar);
	// TODO: fiStream version?

	// CText: don't allow loading additional text in slots above 19 (leads to arbitrary memory corruption)
	MH_CreateHook(hook::get_pattern("EB 08 C7 44 24 20 01 00 00 00 45 33 C9", -0x17), CText__LoadSlotHook, (void**)&g_origCText__LoadSlot);

	// hook to pretend any such slot is loaded
	MH_CreateHook(hook::get_pattern("75 0D F6 84 08 ? ? 00 00", -0xB), CText__IsSlotLoadedHook, (void**)&g_origCText__IsSlotLoaded);

	// and to prevent unloading
	MH_CreateHook(hook::get_pattern("41 BD D8 00 00 00 39 6B 60 74", -0x30), CText__UnloadSlotHook, (void**)&g_origCText__UnloadSlot);

	MH_EnableHook(MH_ALL_HOOKS);
} };
