#include "StdInc.h"
#include "Hooking.h"

static HookFunction hookFunction([] ()
{
	// kill initial legal/intro screens
	hook::nop(hook::get_call(hook::pattern("8D 4C 24 03 E8 ? ? ? ? 8D 4C 24 03 84 C0").count(1).get(0).get<char>(4)) + 0x4F, 5);
});