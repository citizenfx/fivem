#include <StdInc.h>
#include <Hooking.h>

//
// This patch fixes a bug where 32-bit component indices were clamped to 8 bits in
// ped component expression mods related functions. This issue is similar to the one
// described in "PatchPedPropsLimit.cpp", mainly the "overflow" effect, but it only
// affects ped component expression mods.
//

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("44 0F B6 F7 0F B6 7B 08 48 8B B0");
		constexpr uint8_t payload[] = { 0x44, 0x8B, 0xF7, 0x90 }; // "mov r14d, edi" and "nop".
		memcpy(location, payload, sizeof(payload)); // replace "movzx r14d, dil".
	}

	{
		auto location = hook::get_pattern<char>("44 0F B6 FE 0F B6 77 08 8B D6 45");
		constexpr uint8_t payload[] = { 0x44, 0x8B, 0xFE, 0x90 }; // "mov r15d, esi" and "nop".
		memcpy(location, payload, sizeof(payload)); // replace "movzx r15d, sil".
	}
});
