#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>

#include <CrossBuildRuntime.h>

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

static uint16_t* _vehicleModKitArray;

static void RelocateRelative(std::initializer_list<PatternPair> list)
{
	void* oldAddress = nullptr;

	for (auto& entry : list)
	{
		// Search for 16-bit integer pattern instead of 32-bit
		auto location = hook::get_pattern<int32_t>(entry.pattern, entry.offset);

		if (!oldAddress)
		{
			oldAddress = hook::get_address<void*>(location);
		}

		auto curTarget = hook::get_address<void*>(location);
		assert(curTarget == oldAddress);

		hook::put<int32_t>(location, (intptr_t)_vehicleModKitArray - (intptr_t)location - 4);
	}
}

static void RelocateAbsolute(std::initializer_list<PatternPair> list)
{
	int32_t oldAddress = 0;

	for (auto& entry : list)
	{
		// Search for 16-bit integer pattern instead of 32-bit
		auto location = hook::get_pattern<int32_t>(entry.pattern, entry.offset);

		if (!oldAddress)
		{
			oldAddress = *location;
		}

		auto curTarget = *location;
		assert(curTarget == oldAddress);

		hook::put<int32_t>(location, (intptr_t)_vehicleModKitArray - hook::get_adjusted(0x140000000));
	}
}

constexpr int NUM_MODKIT_INDICES = 65536;

static HookFunction hookFunction([]()
{
	_vehicleModKitArray = (uint16_t*)hook::AllocateStubMemory(sizeof(uint16_t) * NUM_MODKIT_INDICES);
	RelocateRelative(
	{ { "66 3B F0 66 73 ?? 48 8D", 8 },  // Adjusted to 16-bit search: 66 3B F0 66 73 ?? 48 8D
	{ "66 41 3B C0 66 73 ?? 48 8D", 9 },  // Adjusted to 16-bit search: 66 41 3B C0 66 73 ?? 48 8D
	{ "45 33 C0 4C 8D 0D ?? ?? ?? ?? B9", 6 },  // Adjusted for 16-bit match (??)
	{ "66 B8 FF FF 00 00 48 8D 3D", 8 },  // Adjusted to 16-bit search: 66 B8 FF FF 00 00 48 8D 3D
	{ "7D ?? 41 BC FF FF 00 00 4C 8D 3D", 11 }  // Adjusted for 16-bit match (??)
	});
	RelocateAbsolute(
	{ { "66 3B D1 66 73 ?? 8B C2", 11 },  // Adjusted to 16-bit search: 66 3B D1 66 73 ?? 8B C2
	{ "66 39 4B 2A 66 73 ?? 0F", 14 }  // Adjusted to 16-bit search: 66 39 4B 2A 66 73 ?? 0F
	});
	hook::nop(hook::get_pattern("66 3B F0 66 73 ?? 48 8D", 0), 5);
	hook::nop(hook::get_pattern("66 41 3B C0 66 73 ?? 48 8D", 0), 6);
	hook::nop(hook::get_pattern("66 3B D1 66 73 ?? 8B C2", 0), 5);
	hook::nop(hook::get_pattern("66 39 4B 2A 66 73 ?? 0F", 0), 6);
	hook::nop(hook::get_pattern("41 81 F8 00 04 00 00 7C", 0), 9);

	{
		auto location = hook::get_pattern("66 B8 FF FF 00 00 48 8D 3D", 13);
		hook::put<int32_t>(location, NUM_MODKIT_INDICES);
	}
});
