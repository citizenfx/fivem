#include <StdInc.h>
#include <Hooking.h>

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
		val = 400.0f;
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

	doPatch(&vtblBase[17], GetMaxSpeed<&origBase>, &origBase);
	doPatch(&vtblPhys[17], GetMaxSpeed<&origPhys>, &origPhys);
	doPatch(&vtblDamp[17], GetMaxSpeed<&origDamp>, &origDamp);
});
