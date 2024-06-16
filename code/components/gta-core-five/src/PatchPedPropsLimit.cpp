#include <StdInc.h>
#include <Hooking.h>
#include "CrossBuildRuntime.h"

//
// Currently, GTA5 has a "soft limit" of 256 global (not per-DLC) ped prop indices (with a maximum index of 0xFF)
// per anchor per model info (255 slots for `mp_f_freemode_01`, 256 for `mp_m_freemode_01`, etc.). This means that
// with every single title update for GTA5, the number of "free" slots significantly decreases. Exceeding the
// limit of 256 will lead to the "overflow" effect, corrupting other ped props. This happens because some specific
// game functions expect the prop index argument to be an 8-bit integer. However, it doesn't seem to be explicitly
// intended, as most functions expect 32-bit integers. This patch set is meant to "fix" this inconsistency and
// make the game work fine with 32-bit global ped prop indices.
//

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("48 8B CD E8 ? ? ? ? 0F B6 D0 3B DA", 8);
		hook::nop(location, 3); // nop "movzx edx, al" instruction.
		hook::put<uint16_t>(location, 0xD08B); // put "mov edx, eax" instead.
	}

	{
		auto location = hook::get_pattern<char>("0F B6 C8 83 C8 FF 3B D8 7D 04");
		hook::nop(location, 3); // nop "movzx ecx, al" instruction.
		hook::put<uint16_t>(location, 0xC88B); // put "mov ecx, eax" instead.
	}

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		auto location = hook::get_pattern<char>("33 D2 E8 ? ? ? ? 0F B6 C0 3B D8", 7);
		hook::nop(location, 3); // nop "movzx eax, al" so it will use a 32-bit register.
	}
	else
	{
		auto location = hook::get_pattern<char>("0F B6 D0 39 53 28 7C 0F");
		hook::nop(location, 3); // nop "movzx edx, al" instruction.
		hook::put<uint16_t>(location, 0xD08B); // put "mov edx, eax" instead.
	}

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		auto location = hook::get_pattern<char>("33 D2 E8 ? ? ? ? 0F B6 C0 3B F8 7D 07", 7);
		hook::nop(location, 3); // nop "movzx eax, al" so it will use 32-bit register.
	}
	else
	{
		auto location = hook::get_pattern<char>("41 3B F8 7D 07 89 7B 18", -4);
		hook::nop(location, 4); // nop "movzx r8d, al" instruction.
		hook::put<uint32_t>(location, 0x90C08B44); // put "mov r8d, eax" and "nop" instead.	
	}

	{
		auto location = hook::get_pattern<char>("44 0F B6 C0 40 0F B6 D6");
		hook::put<uint32_t>(location, 0x90C08B44); // replace "movzx r8d, al" with "mov r8d, eax" and "nop".
	}

	{
		auto location = hook::get_pattern<char>("44 0F B6 C0 0F B6 D3 48 8B CD E8");
		hook::put<uint32_t>(location, 0x90C08B44); // replace "movzx r8d, al" with "mov r8d, eax" and "nop".
	}

	{
		auto matches = hook::pattern("45 0F B6 C4 41 0F B6 D6 E8").count(2);
		for (auto i = 0; i < (int)matches.size(); i++)
		{
			char* location = matches.get(i).get<char>(0);
			hook::put<uint32_t>(location, 0x90C48B45); // replace "movzx r8d, r12b" with "mov r8d, r12d" and "nop".
		}
	}

	{
		auto location = hook::get_pattern<char>("45 0F B6 C0 0F B6 D2 41 8B D9 E8");
		hook::nop(location, 4); // nop "movzx r8d, r8b" so it will use 32-bit register.
	}

	{
		auto location = hook::get_pattern<char>("44 0F B6 F8 44 0F B6 E6 49 8B CE 45 8B C7");
		hook::put<uint32_t>(location, 0x90F88B44); // replace "movzx r15d, al" with "mov r15d, eax" and "nop".
	}

	{
		auto matches =  hook::pattern("45 33 C9 44 8A C5 40 8A D6 48 8B CF").count(2);
		for (auto i = 0; i < (int)matches.size(); i++)
		{
			char* location = matches.get(i).get<char>(3);
			constexpr uint8_t payload[] = { 0x44, 0x8B, 0xC5 };
			memcpy(location, payload, sizeof(payload)); // replace "mov r8b, bpl" with "mov r8d, ebp".
		}
	}

	{
		auto matches =  hook::pattern("41 B9 ? 00 00 00 44 8A C5 40 8A D6").count(4);
		for (auto i = 0; i < (int)matches.size(); i++)
		{
			char* location = matches.get(i).get<char>(6);
			constexpr uint8_t payload[] = { 0x44, 0x8B, 0xC5 };
			memcpy(location, payload, sizeof(payload)); // replace "mov r8b, bpl" with "mov r8d, ebp".
		}
	}

	{
		auto location = hook::get_pattern<char>("45 8A CF 44 8A C7 8A D3 48", 3);
		constexpr uint8_t payload[] = { 0x44, 0x8B, 0xC7 };
		memcpy(location, payload, sizeof(payload)); // replace "mov r8b, dil" with "mov r8d, edi".
	}
});
