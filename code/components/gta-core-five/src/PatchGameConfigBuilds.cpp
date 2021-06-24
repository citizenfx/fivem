#include <StdInc.h>
#include <Hooking.h>

#include <CrossBuildRuntime.h>

namespace rage
{
struct fwConfigWithFilter
{
	const char* Build;
	const char* Platforms;
};
}

static bool (*g_orig_rage__fwConfigManager__MatchFilter)(void* self, const rage::fwConfigWithFilter& config);

static bool rage__fwConfigManager__MatchFilter(void* self, const rage::fwConfigWithFilter& config)
{
	auto buildNum = fmt::sprintf("b%d", xbr::GetGameBuild());
	if (config.Build && strcmp(config.Build, buildNum.c_str()) == 0)
	{
		return true;
	}

	return g_orig_rage__fwConfigManager__MatchFilter(self, config);
}

static HookFunction hookFunction([]()
{
	auto location = hook::get_pattern("48 8B CF 48 8D 14 D8 E8 ? ? ? ? 84 C0 0F", 7);
	hook::set_call(&g_orig_rage__fwConfigManager__MatchFilter, location);
	hook::call(location, rage__fwConfigManager__MatchFilter);
});
