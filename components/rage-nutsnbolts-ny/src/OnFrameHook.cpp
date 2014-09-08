#include "StdInc.h"
#include "Hooking.h"
#include "nutsnbolts.h"

__declspec(dllexport) fwEvent<> OnPreProcessNet;
__declspec(dllexport) fwEvent<> OnPostProcessNet;
__declspec(dllexport) fwEvent<> OnGameFrame;

static HookFunction hookFunction([] ()
{
	static auto runFrame = [] ()
	{
		//g_netLibrary->RunFrame();
		OnGameFrame();
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

	static hook::inject_call<void, int> processNetRegular(0x42131A);
	processNetRegular.inject([] (int a1)
	{
		OnPreProcessNet();

		processNetRegular.call(a1);

		OnPostProcessNet();
	});

	// for long-lasting loads
	static hook::inject_call<void, int> processNetLoader(0x41F652);
	processNetLoader.inject([] (int a1)
	{
		OnPreProcessNet();

		processNetLoader.call(a1);

		OnPostProcessNet();
	});
});