#include <StdInc.h>
#include <Hooking.h>

#include <netInterface.h>

rage::netInterface_queryFunctions** g_queryFunctions;

auto rage::netInterface_queryFunctions::GetInstance() -> netInterface_queryFunctions*
{
	return *g_queryFunctions;
}

static HookFunction hookFunction([]()
{
	g_queryFunctions = hook::get_address<rage::netInterface_queryFunctions**>(hook::get_pattern("72 23 48 8B 0D ? ? ? ? 48 85 C9 74", 5));
});
