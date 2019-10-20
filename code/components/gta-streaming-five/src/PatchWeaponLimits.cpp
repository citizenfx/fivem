#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>

class CWeaponComponentInfo
{
public:
	void* vtable; // +0
	int32_t hash; // +8
	int32_t model; // +12
	int32_t name; // +16
	int32_t description; // +20
private:
	char pad[168];
}; // 192

struct PatternPair
{
	std::string_view pattern;
	int count;
	int index;
	int offset;
};

// Should be synced with CWeaponComponentInfo in gameconfig.xml
constexpr int NUM_WEAPON_COMPONENT_INFOS = 1024;

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

		*location = (intptr_t)weaponComponentInfoCollection - (intptr_t)location - 4;
		++index;
	}
}

static HookFunction initFunction([]()
{
	weaponComponentInfoCollection = (CWeaponComponentInfo * *)hook::AllocateStubMemory(sizeof(CWeaponComponentInfo*) * NUM_WEAPON_COMPONENT_INFOS);

	RelocateRelative({
		{ "42 8D 0C 02 4C 8D 0D ? ? ? ? D1 F9", 1, 0, 7 },
		{ "45 8D 1C 1A 4C 8D 0D ? ? ? ? 41 D1 FB", 1, 0, 7 },
		{ "42 8D 14 01 4C 8D 0D ? ? ? ? D1 FA", 1, 0, 7 },
		{ "47 8D 0C 1A 4C 8D 05 ? ? ? ? 41 D1 F9", 1, 0, 7 },
		{ "43 8D 0C 08 48 8D 35 ? ? ? ? D1 F9", 1, 0, 7 },
		{ "43 8D 14 1A 48 8D 0D ? ? ? ? D1 FA", 1, 0, 7 },
		{ "42 8D 14 01 48 8D 1D ? ? ? ? D1 FA", 1, 0, 7 },
		{ "47 8D 14 0B 4C 8D 05 ? ? ? ? 41 D1 FA", 1, 0, 7 },
		{ "46 8D 04 0A 4C 8D 15 ? ? ? ? 41 D1 F8", 1, 0, 7 }, // 0x6CF598A2957C2BF8
		{ "46 8D 04 11 48 8D 15 ? ? ? ? 41 D1 F8", 1, 0, 7 }, // 0x5CEE3DF569CECAB0
		{ "47 8D 0C 10 48 8D 0D ? ? ? ? 41 D1 F9", 1, 0, 7 }, // 0xB3CAF387AE12E9F8 
		{ "46 8D 04 0A 48 8D 1D ? ? ? ? 41 D1 F8 49 63 C0", 2, 0, 7 }, // 0x0DB57B41EC1DB083
		{ "46 8D 04 0A 48 8D 1D ? ? ? ? 41 D1 F8 49 63 C0", 2, 1, 7 }, // 0x6558AC7C17BFEF58
		{ "43 8D 14 08 48 8D 1D ? ? ? ? D1 FA", 1, 0, 7 }, // 0x4D1CB8DC40208A17
		{ "46 8D 04 0A 48 8D 0D ? ? ? ? 41 D1 F8", 1, 0, 7 }, // 0x3133B907D8B32053 
		{ "33 DB 48 8D 3D ? ? ? ? 8B D3", 1, 0, 5 } // _loadWeaponComponentInfos
	});
});
