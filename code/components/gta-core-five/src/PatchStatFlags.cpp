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

static HookFunction hookFunction([]()
{
	// patch 'are stats pending load/save' functions to always return false
	{
		auto location = hook::get_pattern<char>("75 3F 33 C9 E8 ? ? ? ? 84 C0 75 16", 4);
		hook::jump(hook::get_call(location), Return<int, 0>);
		hook::jump(hook::get_call(location + 11), Return<int, 0>);
	}

});
