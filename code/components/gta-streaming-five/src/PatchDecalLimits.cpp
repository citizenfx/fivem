#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

struct DecalPatternPair : PatternPair
{
	bool clamp;
};

struct DecalDef
{
	char pad[80];
};

constexpr int kNumDecalDefs = 512;

static void* decalDefStore;

static HookFunction initFunction([]()
{
	// patching hardcoded max available decal defs
	{
		std::initializer_list<DecalPatternPair> list = {
			{ "8B 52 34 E8 ? ? ? ? 8B F0 3D", 11, false },
			{ "83 F9 FF 7E ? E8 ? ? ? ? 44 8B F8 3D", 14, false },
			{ "41 8B D0 8B C8 E8 ? ? ? ? 8B F8 3D", 13, false },
			{ "41 8B C8 E8 ? ? ? ? 8B D8 3D", 11, false },
			{ "8B CB E8 ? ? ? ? 8B D8 3D", 10, false },
			{ "41 8B C9 E8 ? ? ? ? 8B D8 3D", 11, false },
			{ "8B 54 24 ? E8 ? ? ? ? 44 8B F0 3D", 13, false },
			{ "44 8B C0 3D ? ? ? ? 7D ? 44 3B 05", 4, false },
			{ "0F 45 F1 8B C8 E8 ? ? ? ? 8B F8 3D", 13, false },
			{ "44 8B CA BA ? ? ? ? 3B CA 73", 4, false },
			{ "8B D9 85 C9 74 ? C7 02", 8, false },
			{ "48 03 C9 39 1C C8 75 ? 81 3A", 10, false },
			{ "41 FF C2 45 3B D3 72 ? 81 3A", 10, false },
			{ "45 85 FF 75 ? 81 3D", 11, false },
			{ "83 E0 07 3C 07 0F 84 ? ? ? ? 81", 13, true },
			{ "81 FB ? ? ? ? 0F 87 ? ? ? ? 3B", 2, true },
		};

		for (auto& entry : list)
		{
			auto location = hook::pattern(entry.pattern).count(1).get(0).get<char>(entry.offset);
			hook::put(location, (entry.clamp ? (kNumDecalDefs - 1) : kNumDecalDefs));
		}
	}

	// patching pointers to static decal def store array
	{
		std::initializer_list<PatternPair> list = {
			{ "8B 52 34 E8 ? ? ? ? 8B F0 3D", 39 },
			{ "83 F9 FF 7E ? E8 ? ? ? ? 44 8B F8 3D", 36 },
			{ "41 8B D0 8B C8 E8 ? ? ? ? 8B F8 3D", 38 },
			{ "41 8B C8 E8 ? ? ? ? 8B D8 3D", 36 },
			{ "8B CB E8 ? ? ? ? 8B D8 3D", 35 },
			{ "83 E0 07 3C 07 0F 84 ? ? ? ? 81", 38 },
			{ "0F B7 46 ? 3B 05 ? ? ? ? 73 ? 48 8D 1C 80 48", 19 },
			{ "81 FB ? ? ? ? 0F 87 ? ? ? ? 3B", 29 },
			{ "41 8B C9 E8 ? ? ? ? 8B D8 3D", 36 },
			{ "8B 54 24 ? E8 ? ? ? ? 44 8B F0 3D", 33 },
			{ "44 8B C0 3D ? ? ? ? 7D ? 44 3B 05", 26 },
			{ "0F 45 F1 8B C8 E8 ? ? ? ? 8B F8 3D", 38 },
			{ "0F B7 80 ? ? ? ? 3B 05 ? ? ? ? 73 ? 48 8D 0C 80 48", 22 },
			{ "73 ? 48 8D 0C 92 48", 9 },
			{ "0F B7 84 ? ? ? ? ? 3B 05 ? ? ? ? 73 ? 48 8D 0C 80 48 8D 05 ? ? ? ? 48", 23 },
			{ "74 ? 0F B7 81 ? ? ? ? 3B 05 ? ? ? ? 73 ? 48 8D 14 80 48", 24 },
			{ "48 03 C9 39 1C C8 75 ? 81 3A", -4 },
			{ "41 8B C3 73 ? 4C 8D 14 80 48", 12 },
			{ "0F B7 47 ? 3B 05 ? ? ? ? 73 ? 48 8D 0C 80 48", 19 },
			{ "E8 ? ? ? ? 4C 8D 25 ? ? ? ? 0F", 8 },
			{ "48 8D ? ? 48 C1 E7 ? E8 ? ? ? ? 48", 16 },
			{ "8B 05 ? ? ? ? 48 8D 3C 80 48", 13 },
			{ "0F 2F 35 ? ? ? ? 44 88 64 07 ? 4C", 15 },
			{ "4C 8B F2 4C 8B F9 4C 8D 2D", 9 },
			{ "3B 05 ? ? ? ? 73 ? 48 8D 34 80 48", 15 },
			{ "0F B7 87 ? ? ? ? 3B 05 ? ? ? ? 73 ? 48 8D 1C 80 48", 22 },
			{ "48 8D 3D ? ? ? ? F3 0F 10 C8", 3 },
			{ "0F B7 41 ? 66 0F 6E C0 0F 5B C0 F3 0F 59 05 ? ? ? ? F3 0F 5C C1", -27 },
			{ "0F 28 F9 73 ? 48", 12 },
			{ "48 83 C3 50 FF CF 79 F0 48 8D", -17 },
		};

		decalDefStore = hook::AllocateStubMemory(sizeof(DecalDef) * kNumDecalDefs);
		void* oldAddress = nullptr;

		for (auto& entry : list)
		{
			auto location = hook::pattern(entry.pattern).count(1).get(0).get<int32_t>(entry.offset);

			if (!oldAddress)
			{
				oldAddress = hook::get_address<void*>(location);
			}

			auto curTarget = hook::get_address<void*>(location);
			assert(curTarget == oldAddress);

			*location = (intptr_t)decalDefStore - (intptr_t)location - 4;
		}
	}

	// patching pointer to decal def store +8 (some sort of rage optimizations)
	{
		auto location = hook::pattern("4C 8D 3D ? ? ? ? 48 03 F6 49 8B 1C").count(1).get(0).get<int32_t>(3);

		*location = ((intptr_t)decalDefStore + 8) - (intptr_t)location - 4;
	}
});
