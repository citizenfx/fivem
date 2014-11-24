#include "StdInc.h"
#include "Hooking.h"
#include "nutsnbolts.h"

__declspec(dllexport) fwEvent<> OnGameFrame;

static HookFunction hookFunction([] ()
{
	static auto runFrame = [] ()
	{
		OnGameFrame();
	};

	static hook::inject_call<void, int> call1((uintptr_t)hook::pattern("6A 05 E8 ? ? ? ? 83 C4 04 B9").count(1).get(0).get<void>(2));
	call1.inject([] (int)
	{
		runFrame();
	});

	static hook::inject_call<void, int> call2((uintptr_t)hook::pattern("6A 00 E8 ? ? ? ? 83 C4 04 E8 ? ? ? ? A0").count(1).get(0).get<void>(81));
	call2.inject([] (int)
	{
		runFrame();
	});
});