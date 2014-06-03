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

	// don't do some random 'TV health check' over HTTP
	hook::return_function(0x87FC60);

	// ignore checking for XNADDR/related data during the connection request
	hook::put<uint8_t>(0x541221, 0xB8);
	hook::put<uint32_t>(0x541222, 1);

	// try not to do dxdiag api stuff, it sucks and is slow
	hook::put<uint32_t>(0x10C4368, 0);

	// single-use/instance mutex
	hook::nop(0x5AACB5, 2);
	hook::put<uint8_t>(0x5AACBC, 0xEB);

	// don't unload streamed fonts
	hook::return_function(0x7F9260);

	// ignore initial loading screens
	hook::put<uint8_t>(0x402B49, 0xEB);

	// emergency streaming safeguard related to process address space or something; this is
	// typically a sign of something else being wrong but I can't be bothered to fix it
	//hook::put<uint8_t>(0xAC1F93, 0xEB);

	// try getting rid of CStreaming::IsEnoughMemoryFor recalibrating VRAM specification in streaming subsystem
	//hook::nop(0xAC1F5C, 5); // call with 'retn 4' in it
	//hook::nop(0xAC1F57, 1); // push
});