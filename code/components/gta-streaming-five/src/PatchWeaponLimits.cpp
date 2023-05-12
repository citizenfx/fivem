#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>

#include <CrossBuildRuntime.h>

class CWeaponComponentInfo
{
public:
	char pad_0[0x10]; // +0
	uint32_t name; // +16
	uint32_t model; // +20
	uint32_t locName; // +24
	uint32_t locDesc; // +28
	char pad_1[0x4]; // +36
	float accuracyModifier; // +40
	char pad_2[0x4]; // +44
	float damageModifier; // +48
	char pad_3[0x4]; // +52
	float rangeModifier; // +56
	char pad_4[0x4]; // +60
	bool shownOnWheel; // +64
	bool createObject; // +65
	bool applyWeaponTint; // +66
	int8_t hudDamage; // +67
	int8_t hudSpeed; // +68
	int8_t hudCapacity; // +69
	int8_t hudAccuracy; // +70
	int8_t hudRange; // +71
	char pad_5[0x78]; // +72
}; // 192

struct PatternPair
{
	std::string_view pattern;
	int count;
	int index;
	int offset;
};

// should be synced with CWeaponComponentInfo in gameconfig.xml
constexpr int kNumWeaponComponentInfos = 2048;

static CWeaponComponentInfo** weaponComponentInfoCollection;

static void RelocateRelative(std::initializer_list<PatternPair> list)
{
	void* oldAddress = nullptr;
	auto index = 0;

	for (auto& entry : list)
	{
		auto location = hook::pattern(entry.pattern).count(entry.count).get(entry.index).get<int32_t>(entry.offset);

		if (!oldAddress)
		{
			oldAddress = hook::get_address<void*>(location);
		}

		auto curTarget = hook::get_address<void*>(location);
		assert(curTarget == oldAddress);

		hook::put<int32_t>(location, (intptr_t)weaponComponentInfoCollection - (intptr_t)location - 4);
		++index;
	}
}

static HookFunction initFunction([]()
{
	// adjust CWeaponInfoBlob limit
	hook::put<uint32_t>(hook::get_pattern("44 89 40 FE 48 8D 40 10 79 ? 44 89 83", 0x12), kNumWeaponInfoBlobs);

	weaponComponentInfoCollection = (CWeaponComponentInfo**)hook::AllocateStubMemory(sizeof(CWeaponComponentInfo*) * kNumWeaponComponentInfos);

	RelocateRelative({
		{ "42 8D 0C 02 4C 8D 0D ? ? ? ? D1 F9", 1, 0, 7 },
		{ "45 8D 1C 1A 4C 8D 0D ? ? ? ? 41 D1 FB", 1, 0, 7 },
		{ "42 8D 14 01 4C 8D 0D ? ? ? ? D1 FA", 1, 0, 7 },
		{ "47 8D 0C 1A 4C 8D 05 ? ? ? ? 41 D1 F9", 1, 0, 7 },
		{ "43 8D 0C 08 48 8D 35 ? ? ? ? D1 F9", 1, 0, 7 },
		{ "43 8D 14 1A 48 8D 0D ? ? ? ? D1 FA", 1, 0, 7 },
		{ "42 8D 14 01 48 8D 1D ? ? ? ? D1 FA", 1, 0, 7 },
		{ "46 8D 04 0A 4C 8D 15 ? ? ? ? 41 D1 F8", 1, 0, 7 }, // 0x6CF598A2957C2BF8
		{ "46 8D 04 11 48 8D 15 ? ? ? ? 41 D1 F8", 1, 0, 7 }, // 0x5CEE3DF569CECAB0
		{ "47 8D 0C 10 48 8D 0D ? ? ? ? 41 D1 F9", 1, 0, 7 }, // 0xB3CAF387AE12E9F8
		{ "46 8D 04 0A 48 8D 1D ? ? ? ? 41 D1 F8 49 63 C0", 2, 0, 7 }, // 0x0DB57B41EC1DB083
		{ "46 8D 04 0A 48 8D 1D ? ? ? ? 41 D1 F8 49 63 C0", 2, 1, 7 }, // 0x6558AC7C17BFEF58
		{ "43 8D 14 08 48 8D 1D ? ? ? ? D1 FA", 1, 0, 7 }, // 0x4D1CB8DC40208A17
		{ "46 8D 04 0A 48 8D 0D ? ? ? ? 41 D1 F8", 1, 0, 7 }, // 0x3133B907D8B32053
		{ "33 DB 48 8D 3D ? ? ? ? 8B D3", 1, 0, 5 } // _loadWeaponComponentInfos
	});

	if (xbr::IsGameBuildOrGreater<2060>())
	{
		RelocateRelative({
			{ "48 8D 1D ? ? ? ? 41 39 BE ? 01 00 00", 1, 0, 3 },
			{ "48 8D 1D ? ? ? ? 45 3B C8 7E", 1, 0, 3 },
			{ "48 8D 1D ? ? ? ? 41 3B BE ? ? ? ? 72", 1, 0, 3 },
			{ "48 8D 1D ? ? ? ? 33 FF 41 39 BE", 1, 0, 3 },
			{ "48 8D 1D ? ? ? ? 41 3B BE ? ? ? ? 0F", 1, 0, 3 },
		});
	}
	else
	{
		RelocateRelative({
			{ "47 8D 14 0B 4C 8D 05 ? ? ? ? 41 D1 FA", 1, 0, 7 },
		});
	}
});
