#include "StdInc.h"
#include "HookCallbacks.h"

static HookFunction hookFunction([] ()
{
	static hook::inject_call<void, bool> scriptInit(0x4201B0);

	scriptInit.inject([] (bool dontStartScripts)
	{
		scriptInit.call(dontStartScripts);

		HookCallbacks::RunCallback(StringHash("scrInit"), nullptr);
	});

	// ignore startup/network_startup
	hook::put<uint8_t>(0x809A81, 0xEB);
});