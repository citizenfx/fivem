#include "StdInc.h"

static bool PopCycle_IsNetworkGame()
{
	// 'allow population group cycling in multiplayer' native flag; if true we shouldn't be a network game
	return !(*(uint8_t*)0x1354750);
}

static HookFunction hookFunction([] ()
{
	// ignore population cycle ignorance if network games (e.g. enable population cycle changes for networked games)
	hook::call(0x8F3673, PopCycle_IsNetworkGame);

	// use regular population methods in MP
	hook::call(0x8F3CFA, PopCycle_IsNetworkGame);

	// more ignorance of stuff in network disabled (pedgrp reading?)
	hook::call(0x8F1CB5, PopCycle_IsNetworkGame);
});