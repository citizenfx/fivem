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
});