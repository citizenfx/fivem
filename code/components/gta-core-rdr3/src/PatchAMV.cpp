#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Patterns.h>
#include <Pool.h>

//
// There is a global bitset used to store the enabled/disabled state of every AMV. This bitset has a hardcoded size
// of 6016 entries, once there are more AMVs than the bitset size, it starts reading/writing out-of-bounds. This causes
// the AMVs to start blinking and can potentially crash.
// 
// Here we replace that bitset with a dynamic bitset resized to fit the capacity of all the AMV pools.
//

static std::vector<bool> enabledAMVs[2]; // double-buffered, one for render thread and another one for update thread

static void AMVInitEnabledBitset()
{
	const size_t totalAMVs = rage::GetPoolBase("CAmbientMaskVolume")->GetSize() +
							 rage::GetPoolBase("CAmbientMaskVolumeDoor")->GetSize() +
							 rage::GetPoolBase("CAmbientMaskVolumeEntity")->GetSize();
	enabledAMVs[0].clear();
	enabledAMVs[1].clear();
	enabledAMVs[0].resize(totalAMVs, false);
	enabledAMVs[1].resize(totalAMVs, false);
}

static void AMVSetEnabled(uint16_t amvIndex, bool enable, uint32_t bufferIndex)
{
	enabledAMVs[bufferIndex][amvIndex] = enable;
}

static bool AMVIsEnabled(uint16_t amvIndex, uint32_t bufferIndex)
{
	return enabledAMVs[bufferIndex][amvIndex];
}

static void AMVPresentEnabledBitset(uint32_t srcBufferIndex)
{
	// copy update buffer to render buffer
	const uint32_t dstBufferIndex = 1 - srcBufferIndex;
	enabledAMVs[dstBufferIndex] = enabledAMVs[srcBufferIndex];
}

static HookFunction hookFunction([]()
{
	// AMV init
	{
		auto location = hook::get_pattern<char>("48 8D 0D ? ? ? ? E8 ? ? ? ? 8A 05 ? ? ? ? 84 C0 75 ? 49 8B CC");
		hook::nop(location, 12);
		hook::call_rcx(location, AMVInitEnabledBitset);
	}

	// AMV set enabled
	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				//  eax = buffer index
				//   cx = AMV index
				// r10b = enable

				sub(rsp, 0x28);
				
				// cx already correct
				mov(dl, r10b);
				mov(r8d, eax);
				mov(rax, reinterpret_cast<uintptr_t>(AMVSetEnabled));
				call(rax);

				add(rsp, 0x28);

				ret();
			}
		} amvSetEnabledStub;

		auto location = hook::get_pattern<char>("4C 69 C0 ? ? ? ? 0F B7 C9");
		hook::jump_rdx(location, amvSetEnabledStub.GetCode());
	}

	// AMV is enabled
	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				// eax = buffer index
				//  cx = AMV index

				sub(rsp, 0x28);

				// cx already correct
				mov(edx, eax);
				mov(rax, reinterpret_cast<uintptr_t>(AMVIsEnabled));
				call(rax);

				add(rsp, 0x28);

				ret();
			}
		} amvIsEnabledStub;

		auto location = hook::get_pattern<char>("0F B7 D1 48 69 C8");
		hook::jump_rdx(location, amvIsEnabledStub.GetCode());
	}

	// AMV present buffer
	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				// r10d = source buffer index
				
				sub(rsp, 0x20);

				mov(ecx, r10d);
				mov(rax, reinterpret_cast<uintptr_t>(AMVPresentEnabledBitset));
				call(rax);

				add(rsp, 0x20);

				ret();
			}
		} amvPresentBufferStub;

		auto location = hook::get_pattern<char>("41 8B C2 45 2B C1 48 69 D0");
		hook::nop(location, 0xAD);
		hook::call(location, amvPresentBufferStub.GetCode());
	}
});
