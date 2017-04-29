// patches from xliveless

#include "StdInc.h"

static HookFunction hookFunction([] ()
{
	// don't sleep during launch
	hook::put<int>(0x401835, 0);

	// don't load reportfault.dll/such
	hook::return_function(0xD356D0);

	// digital signature validity checks on files
	hook::return_function(0x403F10, 8);

	// some initialization checks including EFC
	hook::put<uint16_t>(0x40262D, 0xC033);	// xor eax, eax
	hook::put<uint8_t>(0x40262F, 0xE9);		// jmp
	hook::put<uint32_t>(0x402630, 0x24A);	// end target 40287E

	// at the end of this, place the xor'd eax into the variable someplace after the jump
	hook::put<uint8_t>(0x402883, 0x90);		// nop
	hook::put<uint8_t>(0x402884, 0xA3);		// mov mem32, eax

	// VDS102 check
	hook::nop(0x4028ED, 0x2a);

	// some 'RGSC init check' long conditional jump
	hook::nop(0x40291D, 6);

	// modification checks
	hook::nop(0x402B12, 14);
	hook::nop(0x402D17, 14);

	hook::put<uint32_t>(0x403870, 0x90C3C033); // xor eax, eax; retn
	hook::put<uint32_t>(0x404250, 0x90C3C033); // xor eax, eax; retn

	// securom spot checks
	hook::put<uint32_t>(0xBAC160, 0x90C301B0);
	hook::put<uint32_t>(0xBAC180, 0x90C301B0);
	hook::put<uint32_t>(0xBAC190, 0x90C301B0);
	hook::put<uint32_t>(0xBAC1C0, 0x90C301B0);
});