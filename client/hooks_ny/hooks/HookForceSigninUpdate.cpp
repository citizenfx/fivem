#include "StdInc.h"

static int return1()
{
	return 1;
}

static HookFunction hookFunction([] ()
{
	static hook::inject_call<int, int> status1Hook(0x75DE9D);

	status1Hook.inject([] (int)
	{
		return 1;
	});

	hook::call(0x75DEDE, return1);
});