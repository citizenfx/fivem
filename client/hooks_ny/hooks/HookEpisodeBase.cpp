// patches/hooks for episode-based loading

#include "StdInc.h"

static HookFunction hookFunction([] ()
{
	// don't load P_e2_landing_marker in case of EP2 (as we are EP2 but don't have EP2 files)
	hook::put<uint8_t>(0x854C9B, 0xEB);
	hook::put<uint8_t>(0x854D3D, 0xEB);

	// don't do the stars and sparkles on loading text
	hook::nop(0x7BC894, 10);
	hook::nop(0x7BC9F3, 2);
	hook::put<uint16_t>(0x7BC9FB, 0xE990);
	hook::nop(0x7BCCA0, 6);
	hook::nop(0x7BCCB1, 6);
	hook::nop(0x7BCDC8, 18);
	hook::nop(0x7BD1A4, 10);
	hook::nop(0x7BD26E, 2);
	hook::put<uint16_t>(0x7BD276, 0xE990);
	hook::nop(0x7BD342, 18);
	hook::nop(0x7BD41B, 2);
	hook::put<uint8_t>(0x7BD423, 0xEB);
	hook::nop(0x7BD466, 2);
	hook::put<uint16_t>(0x7BD46E, 0xE990);

	// DO YOU WANT TO PLAY THE NEWLY DOWNLOADED EPISODE????
	// ^ patch that out
	hook::jump(0x42098B, 0x420A48);

	// we however need to follow the esi == 7 path anyway, or the game will break badly
	hook::nop(0x420ADC, 2);
});