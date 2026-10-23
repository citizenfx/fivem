#include <StdInc.h>
#include <Hooking.h>
#include <jitasm.h>

#include "CrossBuildRuntime.h"

static HookFunction hookFunction([]()
{
	// CScenarioPoint::GetModelSetIndex() indexes m_ModelSets[kVehicleModelSets] for vehicle scenario points
	// and m_ModelSets[kPedModelSets] for everything else, with no bounds check.
	//
	// Stock data hides it (151 vehicle sets vs 262 ped sets, so a vehicle index never overruns the ped
	// array). A resource streaming extra AMBIENT_VEHICLE_MODEL_SET_FILE entries can push the vehicle count
	// past the ped count, turning the mismatch into an OOB read.
	{
		auto location = hook::get_pattern<char>("48 8B 05 ? ? ? ? 48 8B 48 10 48 8B 04 F9 48 85 C0 74 ? 8B 40 08 3D 2F 1D A7 6F");

		static struct : jitasm::Frontend
		{
			void** modelSetManager;
			uintptr_t retSuccess;
			uintptr_t retFail;

			void Init(char* location)
			{
				modelSetManager = hook::get_address<void**>(location, 3, 7);
				retSuccess = reinterpret_cast<uintptr_t>(location) + 15;                                       // test rax, rax
				retFail = reinterpret_cast<uintptr_t>(location) + 20 + *reinterpret_cast<int8_t*>(location + 19); // its jz target
			}

			void InternalMain() override
			{
				mov(rax, reinterpret_cast<uintptr_t>(modelSetManager));
				mov(rax, qword_ptr[rax]); // CAmbientModelSetManager::sm_Instance

				movzx(ecx, word_ptr[rax + 0x18]); // m_ModelSets[kPedModelSets].m_ModelSets.GetCount()
				cmp(edi, ecx);                    // modelSetIndex
				jae("fail");

				mov(rcx, qword_ptr[rax + 0x10]);
				mov(rax, qword_ptr[rcx + rdi * 8]); // [original code]

				mov(rcx, retSuccess);
				jmp(rcx);

				L("fail");
				mov(rcx, retFail);
				jmp(rcx);
			}
		} patchStub;

		patchStub.Init(location);

		hook::nop(location, 15);
		hook::jump_rcx(location, patchStub.GetCode());
	}
	
	// Same bug in an unrelated helper ("does this ped model set spawn animals?"), added around b2944.
	// Its callers pass CScenarioPoint::GetModelSetIndex() with no vehicle-type guard.
	{
		if (!xbr::IsGameBuildOrGreater<2944>())
		{
			return;
		}
		
		auto location = hook::get_pattern<char>("48 8B 05 ? ? ? ? 8B D1 33 DB 48 8B 48 10 48 8B 0C D1 48 85 C9");

		static struct : jitasm::Frontend
		{
			void** modelSetManager;
			uintptr_t retAddr;

			void Init(char* location)
			{
				modelSetManager = hook::get_address<void**>(location, 3, 7);
				retAddr = reinterpret_cast<uintptr_t>(location) + 0x13; // test rcx, rcx
			}

			void InternalMain() override
			{
				mov(rax, reinterpret_cast<uintptr_t>(modelSetManager));
				mov(rax, qword_ptr[rax]);

				mov(edx, ecx); // [original code]
				mov(ebx, 0);   // [original code]

				movzx(r8d, word_ptr[rax + 0x18]); // m_ModelSets[kPedModelSets].m_ModelSets.GetCount()
				cmp(edx, r8d);
				jae("fail");

				mov(rcx, qword_ptr[rax + 0x10]);    // [original code]
				mov(rcx, qword_ptr[rcx + rdx * 8]); // [original code]

				mov(rax, retAddr);
				jmp(rax);

				L("fail");
				mov(ecx, 0);
				mov(rax, retAddr);
				jmp(rax);
			}
		} patchStub;

		patchStub.Init(location);

		hook::nop(location, 0x13);
		hook::jump(location, patchStub.GetCode()); // ecx holds modelSetIndex here, rax is dead
	}

	// CAmbientModelSets::Append() is how any AMBIENT_{PED,PROP,VEHICLE}_MODEL_SET_FILE streamed via a
	// resource's data_file gets merged into the live set (the base level meta loads straight into the
	// array and is unaffected). Right after adopting each incoming CAmbientModelSet*, it walks its models
	// and nulls out every one's Variations pointer, on the very same object that just became the
	// permanent, live entry. The variations (mod kit, colours, livery, extras) are never destroyed, just
	// orphaned, so nothing applies them and nothing frees them either.
	{
		hook::nop(hook::get_pattern<char>("49 8B 06 8B D5 4C 8B 04 F8 66 41 3B 68 18 73 1A 8B C2 FF C2 48 8D 0C 40 49 8B 40 10 48 89 6C C8 08 41 0F B7 40 18 3B D0 7C E6"), 42);
	}
});
