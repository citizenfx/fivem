#include <StdInc.h>
#include <Hooking.h>

#include <CrossBuildRuntime.h>

// Constant buried in VehicleScriptGameStateDataNode
static constexpr float kScriptedMaxSpeed = 320.0f;

using OrigPtr = float(*)(void*);

static OrigPtr origBase;
static OrigPtr origPhys;
static OrigPtr origDamp;

template<OrigPtr* orig>
float GetMaxSpeed(void* self)
{
	auto val = (*orig)(self);

	if (val == 150.0f)
	{
		val = kScriptedMaxSpeed;
	}

	return val;
}

static HookFunction hookFunction([]()
{
	auto baseLocation = hook::get_pattern<char>("75 3A 48 8B D1 48 8B CB E8 ? ? ? ? 48 8D 05");
	auto vtblBase = hook::get_address<void**>(hook::get_call(baseLocation + 8) + 17);
	auto vtblPhys = hook::get_address<void**>(baseLocation + 16);
	auto vtblDamp = hook::get_address<void**>(baseLocation + 36);

	auto doPatch = [](void** funcPtr, auto tgtFunc, auto* saveFunc)
	{
		*saveFunc = (decltype(*saveFunc))*funcPtr;
		hook::put(funcPtr, tgtFunc);
	};

	size_t offset = xbr::IsGameBuildOrGreater<2802>() ? 23 : 17;
	doPatch(&vtblBase[offset], GetMaxSpeed<&origBase>, &origBase);
	doPatch(&vtblPhys[offset], GetMaxSpeed<&origPhys>, &origPhys);
	doPatch(&vtblDamp[offset], GetMaxSpeed<&origDamp>, &origDamp);
});
