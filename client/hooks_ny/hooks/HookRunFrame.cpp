#include "StdInc.h"
#include "CrossLibraryInterfaces.h"

static HookFunction hookFunction([] ()
{
	static auto runFrame = [] ()
	{
		g_netLibrary->RunFrame();
	};

	static hook::inject_call<void, int> call1(0x420DBC);
	call1.inject([] (int)
	{
		runFrame();
	});

	static hook::inject_call<void, int> call2(0x402D81);
	call2.inject([] (int)
	{
		runFrame();
	});
});