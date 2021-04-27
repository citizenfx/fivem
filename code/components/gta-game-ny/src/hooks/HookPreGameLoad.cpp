#include "StdInc.h"
#include <GameInit.h>

static HookFunction hookFunction([] ()
{
	static hook::inject_call<bool, int> preLoadHook((ptrdiff_t)hook::get_pattern<char>("75 1D 6A 34 E8", 12));

	preLoadHook.inject([] (int)
	{
		bool continueLoad = preLoadHook.call();

		if (!continueLoad)
		{
			return false;
		}

		OnPreGameLoad();

		return true;
	});
});
