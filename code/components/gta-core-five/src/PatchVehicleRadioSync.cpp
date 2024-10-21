#include <StdInc.h>
#include <Hooking.h>
#include <CrossBuildRuntime.h>

static hook::cdecl_stub<void*(int)> GetRadioStationByArrayIndex([]()
{
	return hook::get_pattern("3B 0D ? ? ? ? 73 ? 48 8B 05 ? ? ? ? 8B C9");
});

static hook::cdecl_stub<int(void*)> GetRadioStationArrayIndex([]()
{
	return hook::get_pattern("44 8B 05 ? ? ? ? 33 C0 45 85 C0 74 ? 48 8B 15");
});

void* GetRadioStationByMetaIndex(int idx, int unk)
{
	return GetRadioStationByArrayIndex(idx);
}

int GetRadioStationMetaIndex(void* station, int unk)
{
	return GetRadioStationArrayIndex(station);
}

static HookFunction hookFunction([]()
{
	if(xbr::IsGameBuildOrGreater<2944>())
	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 48 83 BB ? ? ? ? ? 48 8B F0 48 89 83");
		hook::call(location, GetRadioStationByMetaIndex);

		location = hook::get_pattern<char>("E8 ? ? ? ? 83 C0 ? 89 83");
		hook::call(location, GetRadioStationMetaIndex);
	}
});
