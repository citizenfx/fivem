// hook for the initial mountpoint of filesystem stuff
#include "StdInc.h"
#include <fiDevice.h>
#include <Hooking.h>

static void CallInitialMount()
{
	rage::fiDevice::OnInitialMount();
}

static HookFunction hookFunction([] ()
{
	/*static hook::inject_call<void, int> injectCall(0x7B2E27);

	injectCall.inject([] (int)
	{
		injectCall.call();

		rage::fiDevice::OnInitialMount();
	});*/

	hook::jump(hook::pattern("C6 05 ? ? ? ? 01 E8 ? ? ? ? 81 C4 00 01").count(1).get(0).get<void>(18), CallInitialMount);
});