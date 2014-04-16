// miscellaneous online support patches

#include "StdInc.h"

static HookFunction hookFunction([] ()
{
	// RGSC userinfo.dat return 1
	hook::put<uint8_t>(0x7B05C0, 0xB8);
	hook::put<uint32_t>(0x7B05C1, 1);
	hook::put<uint8_t>(0x7B05C5, 0xC3);

	// lie about being connected online to prevent slowness
	hook::nop(0x7AF1B7, 2);

	// ignore checking for XNADDR/related data during the connection request
	hook::put<uint8_t>(0x541221, 0xB8);
	hook::put<uint32_t>(0x541222, 1);

	// try not to do dxdiag api stuff, it sucks and is slow
	hook::put<uint32_t>(0x10C4368, 0);
});