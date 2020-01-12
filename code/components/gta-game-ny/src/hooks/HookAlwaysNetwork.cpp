// start a network game on the first game launch
#include "StdInc.h"

static HookFunction hookFunction([] ()
{
	{
		auto location = hook::get_pattern<char>("68 ? ? ? ? b9 ? ? ? ? c7 05 ? ? ? ? 00 00 00 00 C7 05 ? ? ? ? 00 00 00 00 C6");
		hook::put<uint32_t>(*(char**)(location + 12), 1);
		hook::nop(location + 10, 10); // zeroing out the value set above, which is 'next game is network game'
	}

	//hook::nop(0x814178, 5); // zeroing out call

	hook::return_function(hook::get_call(hook::get_pattern("83 c4 04 e8 ? ? ? ? 6a 02", -5))); // sets the value, so we always have it be '1'!!11111!!

	// set stored network game config
	int* networkGameConfig = *hook::get_pattern<int*>("b9 1e 00 00 00 bf ? ? ? ? f3 a5 5f 5e c3", 6); // FIXME: THIS SHOULD BE PUT IN GAMENY
	memset(networkGameConfig, 0, 4 * 0x1E);

	networkGameConfig[0] = 16;
	networkGameConfig[2] = 32;

	// don't zero the above out
	hook::nop(hook::get_call(hook::get_pattern<char>("6a 08 6a 3b ff 15 ? ? ? ? e8", 20)) + 0x76, 5);
});
