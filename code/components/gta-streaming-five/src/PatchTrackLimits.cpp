/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/
#include "StdInc.h"

#include "CrossBuildRuntime.h"
#include <Local.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

struct ClampPatternPair : PatternPair
{
	bool clamp;
};

struct CTrainTrack
{
	char pad[0x250];
};

struct CTrackData
{
	char pad[0xA0];
};

constexpr int kNumTracks = 127;

static CTrackData** trackStore2;
static CTrainTrack** trackStore;

static HookFunction initFunction([]()
{
	if (!xbr::IsGameBuildOrGreater<2545>())
	{
		return;
	}
	
	// patching hardcoded max available train tracks
	{
		std::initializer_list<ClampPatternPair> list = {
			{"41 80 FC ? 0F 8D", 3, false},
			{"83 FF ? 7D ? 44 88 3B", 2, false},
			{"45 8D 77 ? 88 05", 3, false},
			{"7D ? 48 8B C8 8B 05", -1, false},
			{"48 8B C1 48 69 C0 ? ? ? ? 44 88 74 30", -3, false},
			{"41 83 FE ? 0F 8C ? ? ? ? 44 8B 64 24", 3, false},
			{"41 80 F9 ? 7C", 3, false},
			{"83 F8 ? 7D ? 48 8B C8 8B 05", 2, false},
			{"83 FB ? 0F 8C ? ? ? ? 48 83 C4 ? 41 5F 41 5E 41 5C", 2, false},
			{"83 F9 ? 7D ? 4C 8D 05", 2, false},
			{"83 F9 ? 7D ? 42 88 54 00", 2, false},
			{"83 FA ? 7C ? 83 C8 ? C3 8B C2", 2, false},
			{"48 3B C2 7C ? F3 C3 ? 48 8B C4", 2, false},
			{"83 F9 ? 77 ? 48 63 C1 48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 44 39 48", 2, true},
			{"FF C3 83 FB ? 0F 8C ? ? ? ? 48 83 C4", 4, true},
			{"83 FA ? 0F 87 ? ? ? ? F3 0F 10 0E", 2, true},
			{"83 FA ? 0F 87 ? ? ? ? F3 0F 10 35",2, true},
			{"80 FA ? 77 ? 45 85 C0", 2, true},
		};

		for (auto& entry : list)
		{
			auto location = hook::pattern(entry.pattern).count(1).get(0).get<uint8_t>(entry.offset);
			hook::put<uint8_t>(location, (entry.clamp ? (kNumTracks - 1) : kNumTracks));
		}
	}

	// patching pointers to static track array
	{
		trackStore = (CTrainTrack**)hook::AllocateStubMemory(sizeof(CTrainTrack) * kNumTracks);

		std::initializer_list<PatternPair> list = {
			{"48 8D 0D ? ? ? ? 45 8A C7 48 8B C7", 3},
			{"4B 8D 14 64", 7},
			{"48 8D 05 ? ? ? ? 48 69 C9 ? ? ? ? 48 8B 44 01", 3},
			{"4C 8D 15 ? ? ? ? 33 FF", 3},
			{"66 0F 6E 4C 08", -4},
			{"42 38 44 02", -11},
			{"48 8D 35 ? ? ? ? 44 8B 78", 3},
			{"49 0F BE CC", 7},
			{"48 8D 35 ? ? ? ? 41 BB", 3},
			{"F3 0F 11 4D ? F3 0F 10 54 C8", -14},
			{"89 44 11 ? 48 83 7C 24", -11},
			{"8B CB E8 ? ? ? ? 83 25 ? ? ? ? ? E9", -21},
			{"48 8D 2D ? ? ? ? 49 8B D3", 3},
			{"48 69 FF ? ? ? ? 44 8B 64 07", -6},
			{"48 8D 2D ? ? ? ? 48 85 F6 74 ? 85 DB", 3},
			{"48 89 5C 24 ? 48 0F BE C1", 5 + 4 + 3},
			{"49 63 C9 48 8D 35", 6},
			{"84 C9 78 ? 48 0F BE C1 4C 8D 05", 11},
			{"4C 0F BE C9", 7},
			{"74 ? F3 43 0F 10 5C 3B", -17},
			{"41 83 38 ? 74 ? 48 63 C2", 12},
			{"48 0F BE D2 4C 8D 15", 7},
			{"48 8D 3D ? ? ? ? 0F 29 74 24 ? 4C 8D 14 40", 3},
			{"74 ? 48 0F BE 81", 13},
			{"4C 8D 25 ? ? ? ? 44 0F 28 C6", 3},
			{"48 8D 05 ? ? ? ? 48 69 FF ? ? ? ? 48 03 F8 0F 84", 3},
			{"48 85 DB 0F 84 ? ? ? ? 48 8D 3D", 12},
			{"4C 8D 3D ? ? ? ? 48 8D 15 ? ? ? ? 84 C0", 3},
			{"48 8D 3D ? ? ? ? 48 8B 5B", 3},
			{"48 8D 0D ? ? ? ? 83 B8", 3},
			{"48 8D 0D ? ? ? ? 8A 83 ? ? ? ? 84 C0", 3},
			{"EB ? 33 C9 84 C9 75 ? 8A 83", -15},
			{"48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 8A 4C 08 ? EB ? 0F 2F F2", 3},
			{"48 83 EC ? 48 63 F9 48 8D 35", 10},
			{"4C 8D 05 ? ? ? ? 48 69 C0 ? ? ? ? 42 C6 44 00", 3},
			{"48 69 C9 ? ? ? ? 42 88 54 01", -7},
			{"33 F6 45 8B D8 48 8B F9", 11},
			{"48 03 C1 83 78", -11},
			{"33 C0 84 C9 78 ? 38 05", 19},
			{"48 03 C1 44 39 48", -11}
		};

		void* oldAddress = nullptr;
		for (auto& entry : list)
		{
			auto location = hook::get_pattern<int32_t>(entry.pattern, entry.offset);

			if (!oldAddress)
			{
				oldAddress = hook::get_address<void*>(location);
			}

			auto curTarget = hook::get_address<void*>(location);
			assert(curTarget == oldAddress);

			hook::put<int32_t>(location, (intptr_t)trackStore - (intptr_t)location - 4);
		}

		// Patch getTrainTrack function
		{
			uint32_t* location = hook::pattern("48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 C3 ?").count(3).get(1).get<uint32_t>(3);
			auto curTarget = hook::get_address<void*>(location);
			assert(curTarget == oldAddress);
			hook::put<int32_t>(location, (intptr_t)trackStore - (intptr_t)location - 4);
		}
	}
	
	// Patch another static array related to trains
	{
		trackStore2 = (CTrackData**)hook::AllocateStubMemory((sizeof(CTrackData) * kNumTracks) * 20);
		auto location = hook::get_pattern<int32_t>("48 8D 05 ? ? ? ? 49 C1 E0 ? 48 69 F6", 3);
		hook::put<int32_t>(location, (intptr_t)trackStore2 - (intptr_t)location - 4);
	}

	// Patch pointer to track array (used for ambient trains)
	{
		auto location = hook::get_pattern("48 8D 1D ? ? ? ? F2 48 0F 2A C0", 3);
		hook::put<int32_t>(location, (intptr_t)trackStore - (intptr_t)location - 4);
	}
});
