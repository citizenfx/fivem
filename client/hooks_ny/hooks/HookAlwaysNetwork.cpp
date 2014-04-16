// start a network game on the first game launch
#include "StdInc.h"

static HookFunction hookFunction([] ()
{
	hook::nop(0x7B7A9C, 5); // zeroing out the value set below, which is 'next game is network game'
	hook::put<uint32_t>(0x10F8070, 1);

	hook::nop(0x814178, 5); // zeroing out call

	hook::return_function(0x7B7970); // sets the value, so we always have it be '1'!!11111!!

	// set stored network game config
	int* networkGameConfig = (int*)(hook::get_adjusted(0x1088FB8)); // FIXME: THIS SHOULD BE PUT IN GAMENY
	memset(networkGameConfig, 0, 4 * 0x1E);

	networkGameConfig[0] = 16;
	networkGameConfig[2] = 32;

	// don't zero the above out
	hook::nop(0x46012C, 5);
});