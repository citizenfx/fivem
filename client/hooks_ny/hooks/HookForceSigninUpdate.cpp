#include "StdInc.h"

DEFINE_INJECT_HOOK(status2AlHook, 0x75DEDE)
{
	Eax(1);

	return DoNowt();
}

static HookFunction hookFunction([] ()
{
	static hook::inject_call<int, int> status1Hook(0x75DE9D);

	status1Hook.inject([] (int)
	{
		return 1;
	});

	status2AlHook.injectCall();
});