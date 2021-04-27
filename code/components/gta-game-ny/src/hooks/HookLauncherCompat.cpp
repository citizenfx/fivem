// patches for launcher compatibility

#include "StdInc.h"

bool ret1()
{
	return 1;
}

static HookFunction hookFunction([] ()
{
	// /GS check
	hook::return_function(hook::pattern("8B 45 00 A3 ? ? ? ? 8B").count(2).get(0).get<uintptr_t>(-0x6D));

	// is non writable ret
	hook::jump(hook::get_pattern("C7 45 FC 00 00 00 00 68 00 00 40 00", -0x33), ret1);
	//hook::jump(hook::get_pattern("50 83 EC 08 53 56 57", -0x15), ret1);

	// icon resource from launcher exe
	//hook::put<uint8_t>(0x61CE68, 1);
});
