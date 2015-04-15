// hook for the initial mountpoint of filesystem stuff
#include "StdInc.h"
#include <fiDevice.h>
#include <Hooking.h>

static hook::cdecl_stub<void()> originalMount([] ()
{
	return hook::pattern("48 81 EC E0 03 00 00 48 B8 63 6F 6D 6D").count(1).get(0).get<void>(-0x1A);
});

static void CallInitialMount()
{
	// do pre-initial mount
	originalMount();

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

	hook::call(hook::pattern("0F B7 05 ? ? ? ? 48 03 C3 44 88 3C 38 66").count(1).get(0).get<void>(0x15), CallInitialMount);
});