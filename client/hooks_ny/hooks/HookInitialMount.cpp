// hook for the initial mountpoint of filesystem stuff
#include "StdInc.h"
#include "HookCallbacks.h"

static HookFunction hookFunction([] ()
{
	static hook::inject_call<void, int> injectCall(0x7B2E27);

	injectCall.inject([] (int)
	{
		injectCall.call();

		HookCallbacks::RunCallback(StringHash("initMount"), nullptr);
	});
});