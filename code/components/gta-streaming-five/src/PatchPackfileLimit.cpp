#include <StdInc.h>
#include <Hooking.h>

#if 0
#include <fiCollectionWrapper.h>

constexpr int NUM_STREAMING_ARCHIVES = 4096;

static rage::fiCollection** rage__fiCollection__sm_Collections;

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

static void RelocateRelative(std::initializer_list<PatternPair> list)
{
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

		*location = (intptr_t)rage__fiCollection__sm_Collections - (intptr_t)location - 4;
	}
}

static void RelocateAbsolute(std::initializer_list<PatternPair> list)
{
	int32_t oldAddress = 0;

	for (auto& entry : list)
	{
		auto location = hook::get_pattern<int32_t>(entry.pattern, entry.offset);

		if (!oldAddress)
		{
			oldAddress = *location;
		}

		auto curTarget = *location;
		assert(curTarget == oldAddress);

		*location = (intptr_t)rage__fiCollection__sm_Collections - hook::get_adjusted(0x140000000);
	}
}

static void(*g_origSetStreamingInterface)(void*);

static void SetStreamingInterface(char* si)
{
	// override gameconfig ConfigStreamingEngine -> ArchiveCount value
	*(uint32_t*)(si + 24) = NUM_STREAMING_ARCHIVES;

	g_origSetStreamingInterface(si);
}

static void(*g_origStreamerTick)(void*, void*);

static void CustomStreamerTick(void* a1, void* a2)
{
	static auto patternLoc = hook::get_pattern<int32_t>("4D 8B A4 C7 ? ? ? ? 4C 8D 45 58 49 8B 04 24", 4);

	auto oldLoc = *patternLoc;
	*patternLoc = (intptr_t)rage__fiCollection__sm_Collections - hook::get_adjusted(0x140000000);

	g_origStreamerTick(a1, a2);

	*patternLoc = oldLoc;
}

static void*(*g_origQueueStrTask)(uint32_t a1, void* a2, int a3, int a4, void* a5, void* a6, void* a7, uint32_t a8, void* a9, int a10);

static void* CustomQueueStrTask(uint32_t a1, void* a2, int a3, int a4, void* a5, void* a6, void* a7, uint32_t a8, void* a9, int a10)
{
	static auto patternLoc = hook::get_pattern<int32_t>("4C 8B BC C1 ? ? ? ? 4D 85 FF 0F 84", 4);

	auto oldLoc = *patternLoc;
	*patternLoc = (intptr_t)rage__fiCollection__sm_Collections - hook::get_adjusted(0x140000000);

	auto rv = g_origQueueStrTask(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);

	*patternLoc = oldLoc;

	return rv;
}

static void*(*g_origQueueSemaTask)(uint32_t a1, void* a2, int a3, int a4, void* a5, uint32_t a6, int a7);

static void* CustomQueueSemaTask(uint32_t a1, void* a2, int a3, int a4, void* a5, uint32_t a6, int a7)
{
	static auto patternLoc = hook::get_pattern<int32_t>("4C 8B BC C1 ? ? ? ? 4D 85 FF 0F 84", 4);

	auto oldLoc = *patternLoc;
	*patternLoc = (intptr_t)rage__fiCollection__sm_Collections - hook::get_adjusted(0x140000000);

	auto rv = g_origQueueSemaTask(a1, a2, a3, a4, a5, a6, a7);

	*patternLoc = oldLoc;

	return rv;
}

static HookFunction hookFunction([]()
{
	rage__fiCollection__sm_Collections = (rage::fiCollection**)hook::AllocateStubMemory(sizeof(rage::fiCollection*) * NUM_STREAMING_ARCHIVES);

	RelocateRelative({
		{ "FA 48 8B F1 4C 8D 25 ? ? ? ? 89 58 C8 45 85", 7 },
		{ "8B C8 48 8D 35 ? ? ? ? 8B D0 C1 E9 10", 5 },
		{ "4C 8D 0D ? ? ? ? 48 89 01 4C", 3 },
		{ "0F B7 83 92 00 00 00 48 8D 0D", 10 },
		{ "48 8B CB 48 89 3D ? ? ? ? E8 ? ? ? ? 48 8D 47 10  B9 3C 00 00 00", 6 },
		{ "7D 0F 48 8D 0D ? ? ? ? 48 39 1C F9", 5 },
		{ "0F B7 44 33 06 8B 54 33 04 48 8D 0D", 12 },
		{ "C1 E9 10 48 8B 0C C8 48 8B 01 48 FF A0", -4 },
		{ "7D 2A 4C 8D 0D ? ? ? ? 4D 8B 0C C1", 5 },
		{ "8B D9 48 8D 3D ? ? ? ? 83 F9 FF 74", 5 },
		{ "7D 3B 4C 8D 05 ? ? ? ? 4D 8B 04 C0 4D 85 C0", 5 },
		{ "0F 84 D0 00 00 00 0F B7 46 02 48 8D 0D", 13 },
		{ "0F 84 D9 00 00 00 0F B7 46 02 48 8D 0D", 13 },
		{ "8B D0 C1 EA 10 48 8B 0C D1 8B D0 4C 8B 01", -4 },
		{ "C1 E8 10 48 8B 1C C3 48 85 DB 74 1B", -4 },
		{ "0F B7 41 02 48 8B DA 8B 11 48 8D 0D", 12 },
		{ "0F B7 47 12 8B 57 10 4C 8D 35", 10 },
		{ "41 8B CE 48 8D 05 ? ? ? ? C1 E9 10", 6 }
	});

	// NOTE: BOTH OF THESE ARE ARXAN TRIGGERS
	// (it is assumed that Arxan changes any function it 'protects' to `rva` addressing so that
	// it can safely hash the function content even if the OS does image rebasing)
	RelocateAbsolute({
		//{ "4D 8B A4 C7 ? ? ? ? 4C 8D 45 58 49 8B 04 24", 4 }, // arxan trigger(!)
		//{ "4C 8B BC C1 ? ? ? ? 4D 85 FF 0F 84", 4 }
	});

	// first arxan function
	{
		// replace arxan trigger to *only* be patched while it runs
		// and hope there's no cross-thread integrity checking going on

		auto location = hook::get_pattern("48 85 C0 74 E6 48 8B D7 48 8B CB E8", 11);
		hook::set_call(&g_origStreamerTick, location);
		hook::call(location, CustomStreamerTick);
	}

	// second arxan function
	{
		// 6 callers, many of which annoyingly patterned, so hardcoded
		// 1604 unused
		auto location = hook::get_adjusted(0x140D57C62);
		hook::set_call(&g_origQueueStrTask, location);
		hook::call(location, CustomQueueStrTask);
		hook::call(0x141222331, CustomQueueStrTask);
		hook::call(0x1412224B2, CustomQueueStrTask);
		hook::call(0x14127D884, CustomQueueStrTask);
		//hook::call(0x1412807C9, CustomQueueStrTask); // also an arxan trigger
		hook::call(0x14157F677, CustomQueueStrTask);
	}

	// second arxan functions, sublevel variant
	{
		auto location = hook::get_adjusted(0x140D4F852);
		hook::set_call(&g_origQueueSemaTask, location);
		hook::call(location, CustomQueueSemaTask);
		hook::call(0x140D57BED, CustomQueueSemaTask);
		hook::call(0x141219077, CustomQueueSemaTask);
		hook::call(0x14121981F, CustomQueueSemaTask); // risk
		hook::call(0x1412220C7, CustomQueueSemaTask); // risk
		hook::call(0x14122236B, CustomQueueSemaTask);
		hook::call(0x141886A61, CustomQueueSemaTask); // odd section?
	}

	// archive count
	hook::put<int>(hook::get_address<int*>(hook::get_pattern("8B C3 C1 E8 10 3B 05 ? ? ? ? 0F 8D", 7)), NUM_STREAMING_ARCHIVES);

	// direct archive count
	hook::put<int>(hook::get_pattern("4D 39 04 C1 74 10 66 41 03 D2 B8", 11), NUM_STREAMING_ARCHIVES);

	// CGtaStreamingInterface write
	{
		auto location = hook::get_pattern("74 1B 8B 8B 44 03 00", 0x20);
		hook::set_call(&g_origSetStreamingInterface, location);
		hook::call(location, SetStreamingInterface);
	}
});
#endif
