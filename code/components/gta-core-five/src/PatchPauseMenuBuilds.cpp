#include <StdInc.h>
#include <Hooking.h>

#include <CrossBuildRuntime.h>

static void (*g_origActivateContexts)();

static hook::cdecl_stub<void(const uint32_t&)> activatePauseMenuContext([]
{
	return hook::get_pattern("48 8D 4C 24 30 89 44 24 30 E8 ? ? ? ? 84 C0 75 ? 0F", -11);
});

static void ActivatePauseMenuContexts()
{
	g_origActivateContexts();

	activatePauseMenuContext(HashString(fmt::sprintf("b%d", xbr::GetRequestedGameBuild())));
}

static HookFunction hookFunction([]
{
	auto location = hook::get_pattern("8A DA 8B F9 E8 ? ? ? ? E8", 4);
	hook::set_call(&g_origActivateContexts, location);
	hook::call(location, ActivatePauseMenuContexts);
});
