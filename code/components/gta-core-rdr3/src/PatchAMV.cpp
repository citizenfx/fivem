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

	// Due to how AMVPresentBuffer is compiled, the number of blocks needs to match the form (N*32+28). AMVPresentBuffer copies the bitset
	// from update thread to the render thread bitset. The copy loop is vectorized to copy 32 blocks per iteration, then has an epilogue to
	// copy the remaining 28 blocks, which works out for the original 188 blocks. When resizing the bitset we need to pad the number of blocks
	// so it doesn't read/write out-of-bounds.
	constexpr size_t BitsetNumBlocksNoPadding = (TotalAMVs + 31) / 32; // minimum number of 32-bit blocks needed by the AMVs
	constexpr size_t BitsetPresentCopyNumIterations = (BitsetNumBlocksNoPadding - 28 + 31) / 32; // smallest N such that (N*32+28) >= BitsetNumBlocksNoPadding
	constexpr size_t BitsetNumBlocks = BitsetPresentCopyNumIterations * 32 + 28;
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
		hook::put<int32_t>(location + 0x19, BitsetPresentCopyNumIterations);
		hook::put<int32_t>(location + 0xD, (intptr_t)amvEnabledBitsetReplacement - (intptr_t)location - 0xD - 4);
	}
});
