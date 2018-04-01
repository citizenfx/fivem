#include <StdInc.h>
#include <Hooking.h>

#include <netInterface.h>

rage::netInterface_queryFunctions* g_queryFunctions;

auto rage::netInterface_queryFunctions::GetInstance() -> netInterface_queryFunctions*
{
	return g_queryFunctions;
}

static HookFunction hookFunction([]()
{
	auto location = hook::get_address<rage::netInterface_queryFunctions*>(hook::get_pattern("48 8D 0D ? ? ? ? 48 89 44 24 20 E8 ? ? ? ? E8 ? ? ? ? 48 8B", -4));

	g_queryFunctions = location;
});
