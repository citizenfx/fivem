#include <StdInc.h>
#include <Hooking.h>

#include <MinHook.h>
#include <ICoreGameInit.h>

#include <LaunchMode.h>

template<typename T, T Value>
static T Return()
{
	return Value;
}

static int(*g_origGetPedStatIndex)(void* ped);

static int GetPedStatIndex(void* ped)
{
	if (Instance<ICoreGameInit>::Get()->HasVariable("storyMode"))
	{
		return g_origGetPedStatIndex(ped);
	}

	return 3;
}

static HookFunction hookFunction([]()
{
	// patch 'are stats pending load/save' functions to always return false
	{
		auto location = hook::get_pattern<char>("75 3F 33 C9 E8 ? ? ? ? 84 C0 75 16", 4);
		hook::jump(hook::get_call(location), Return<int, 0>);
		hook::jump(hook::get_call(location + 11), Return<int, 0>);
	}

	// patch 'get ped stat index' function to return MP0 at all times
	if (!CfxIsSinglePlayer())
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("83 C8 FF 48 85 C9 74 04", -4), GetPedStatIndex, (void**)&g_origGetPedStatIndex);
		MH_EnableHook(MH_ALL_HOOKS);
		//hook::jump(hook::get_pattern("83 C8 FF 48 85 C9 74 04", -4), Return<int, 3>);
	}
});
