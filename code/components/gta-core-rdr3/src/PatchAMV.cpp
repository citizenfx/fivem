#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Patterns.h>
#include <Pool.h>

//
// There is a global bitset used to store the enabled/disabled state of every AMV. This bitset has a hardcoded size
// of 6016 entries, once there are more AMVs than the bitset size, it starts reading/writing out-of-bounds. This causes
// the AMVs to start blinking and can potentially crash.
// 
// Here we increase the size of that bitset to fit the capacity of all the AMV pools.
//

static HookFunction hookFunction([]()
{
	// keep in sync with gameconfig
	constexpr size_t CAmbientMaskVolume_PoolSize = 6686;
	constexpr size_t CAmbientMaskVolumeDoor_PoolSize = 1850;
	constexpr size_t CAmbientMaskVolumeEntity_PoolSize = 150;

	constexpr size_t TotalAMVs = CAmbientMaskVolume_PoolSize + CAmbientMaskVolumeDoor_PoolSize + CAmbientMaskVolumeEntity_PoolSize;
	constexpr size_t BitsetNumBlocks = TotalAMVs / 32;
	constexpr size_t BitsetNumBytes = BitsetNumBlocks * sizeof(uint32_t);

	// x2 because the bitset is double-buffered, one for render thread and another one for update thread
	uint32_t* amvEnabledBitsetReplacement =  reinterpret_cast<uint32_t*>(hook::AllocateStubMemory(BitsetNumBytes * 2));
	memset(amvEnabledBitsetReplacement, 0, BitsetNumBytes * 2);

	// AMVInit
	{
		auto location = hook::get_pattern<uint8_t>("41 B8 ? ? ? ? 44 89 25");
		hook::put<int32_t>(location + 2, BitsetNumBytes * 2);
		hook::put<int32_t>(location + 0x10, (intptr_t)amvEnabledBitsetReplacement - (intptr_t)location - 0x10 - 4);
	}

	// AMVIsEnabled
	{
		
		auto location = hook::get_pattern<uint8_t>("48 69 C8 ? ? ? ? 44 8B C2");
		hook::put<int32_t>(location + 3, BitsetNumBlocks);
		hook::put<int32_t>(location + 0xD, (intptr_t)amvEnabledBitsetReplacement - (intptr_t)location - 0xD - 4);
	}

	// AMVSetEnabled
	{
		auto location = hook::get_pattern<uint8_t>("4C 69 C0 ? ? ? ? 0F B7 C9");
		hook::put<int32_t>(location + 3, BitsetNumBytes);
		hook::put<int32_t>(location + 0xD, (intptr_t)amvEnabledBitsetReplacement - (intptr_t)location - 0xD - 4);
	}

	// AMVPresentBuffer
	{
		auto location = hook::get_pattern<uint8_t>("48 69 D0 ? ? ? ? 41 8B C0");
		hook::put<int32_t>(location + 3, BitsetNumBytes);
		hook::put<int32_t>(location + 0x14, BitsetNumBytes);
		hook::put<int32_t>(location + 0x19, BitsetNumBlocks / 32); // number of iterations, each one processes 32 blocks
		hook::put<int32_t>(location + 0xD, (intptr_t)amvEnabledBitsetReplacement - (intptr_t)location - 0xD - 4);
	}
});
