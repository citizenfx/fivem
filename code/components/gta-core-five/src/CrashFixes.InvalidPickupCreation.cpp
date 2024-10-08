#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <CrossBuildRuntime.h>

static void (*origCPedModelInfo__SetupPedBuoyancyInfo)(void* BuoyancyInfo, const void* pCapsuleInfo, const void* FragType, bool bIsWeightless);

static void CPedModelInfo__SetupPedBuoyancyInfo(void* BuoyancyInfo, const void* pCapsuleInfo, const void* FragType, bool bIsWeightless)
{
	if (!FragType)
	{
		return;
	}

	origCPedModelInfo__SetupPedBuoyancyInfo(BuoyancyInfo, pCapsuleInfo, FragType, bIsWeightless);
}

static HookFunction hookFunction([]
{
	{
		static struct : jitasm::Frontend
		{
			int32_t writeOffset;
			intptr_t retLocation;

			void Init(intptr_t location)
			{
				writeOffset = *(int32_t*)(location + 9);
				retLocation = location + 13;
			}

			void InternalMain() override
			{
				test(rax, rax);
				jz("end");

				mov(word_ptr[rax + writeOffset], bx);

				L("end");

				mov(rax, retLocation);
				jmp(rax);
			}
		} patchStub;

		// Fixes a crash trying to set a field on a null entity
		auto location = hook::get_pattern<char>("FF 90 ? ? ? ? 66 89 98");
		patchStub.Init(reinterpret_cast<intptr_t>(location));
		hook::jump_rcx(location + 6, patchStub.GetCode());
	}

	{
		// CPickupCreationDataNode::CanApplyData shadows the modelId variable when a customModelHash is used, which bypasses checking the model is streamed in.
		// Patch the stack offset to use the original variable instead.
		auto location = hook::get_pattern<char>("41 B8 ? ? ? ? 41 0B C0 44 89 45");

		// Get stack offset of the correct modelId variable
		auto offset = *(int8_t*)(location - 0xF);

		// Use previous variable offset wherever needed
		hook::put(location + 0x1C, offset);
		hook::put(location + 0x28, offset);
	}

	{
		// CPedModelInfo::SetupPedBuoyancyInfo doesn't check that FragType isn't null.
		origCPedModelInfo__SetupPedBuoyancyInfo = hook::trampoline(hook::get_call(hook::get_pattern("45 33 C9 4C 8B C0 48 8B D3 E8", 0x9)), &CPedModelInfo__SetupPedBuoyancyInfo);
	}

	// Not present on 1604 and we don't care about builds between 1604 and 2060
	if (xbr::IsGameBuildOrGreater<2060>())
	{
		// Test for a valid weapon component info pointer before de-referencing it.
		static struct : jitasm::Frontend
		{
			uint8_t vtableOffset;

			void Init(intptr_t location)
			{
				vtableOffset = *(uint8_t*)(location + 0x8);
			}

			void InternalMain() override
			{
				test(rbx, rbx);								// if ( rbx )
				jz("skip");									// {
															//
				// * original code                          //
				mov(rax, qword_ptr[rbx]);					//
															//        [run original code]
				mov(rcx, rbx);								//
															//
				mov(rax, qword_ptr[rax + vtableOffset]);	//
				call(rax);									//
				// * original code END                      //
															//
															// }
				L("skip");									//
															//
				ret();										//
			}

		} patchStub;

		// mov rax, [rbx]
		auto location = hook::get_pattern<char>("7E ? 33 DB 48 8B 03", 4);

		patchStub.Init(reinterpret_cast<intptr_t>(location));

		/*
		 * nop:
		 *
		 * mov rax, [rbx]
		 * mov rcx, rbx
		 * call    qword ptr [rax+38h]
		 */
		hook::nop(location, 9);

		hook::call(location, patchStub.GetCode());
	}
});
