#include "StdInc.h"

static bool PopCycle_IsNetworkGame()
{
	static auto allowPopCycle = *(uintptr_t*)(hook::get_call(*hook::get_pattern<uintptr_t>("68 ? ? ? ? 68 F5 00 C1 61", 1) + 0x13) + 5);

	// 'allow population group cycling in multiplayer' native flag; if true we shouldn't be a network game
	return !(*(uint8_t*)allowPopCycle);
}

static HookFunction hookFunction([] ()
{
	// ignore population cycle ignorance if network games (e.g. enable population cycle changes for networked games)
	hook::call(hook::get_pattern("56 8B F1 E8 ? ? ? ? 84 C0 75 1D 8B CE E8", 3), PopCycle_IsNetworkGame);

	// use regular population methods in MP
	hook::call(hook::get_pattern("E8 ? ? ? ? 8B CE 84 C0 74 1B E8"), PopCycle_IsNetworkGame);

	// more ignorance of stuff in network disabled (pedgrp reading?)
	hook::call(hook::get_pattern("51 53 55 56 57 8B F1 E8 ? ? ? ? 84 C0", 7), PopCycle_IsNetworkGame);
});
