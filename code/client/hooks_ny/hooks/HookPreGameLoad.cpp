#include "StdInc.h"
#include "HookCallbacks.h"

static HookFunction hookFunction([] ()
{
	static hook::inject_call<bool, int> preLoadHook(0x402BDF);

	preLoadHook.inject([] (int)
	{
		bool continueLoad = preLoadHook.call();

		if (!continueLoad)
		{
			return false;
		}

		HookCallbacks::RunCallback(StringHash("preGmLoad"), nullptr);

		return true;
	});
});