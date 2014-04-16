#include "StdInc.h"
#include "HookCallbacks.h"

static HookFunction hookFunction([] ()
{
	static hook::inject_call<void, int> scriptInit(0x809E92);

	scriptInit.inject([] (int numScripts)
	{
		scriptInit.call(numScripts);

		HookCallbacks::RunCallback(StringHash("scrInit"), nullptr);
	});
});