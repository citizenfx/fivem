#ifndef COMPILING_STATE
#error State components should only be built as part of a game state component.
#endif

#pragma once

#include <CoreConsole.h>
#include <GameNames.h>

inline bool IsStateGame()
{
	using fx::GameName;

	static auto gamename = std::make_shared<ConVar<GameName>>("gamename", ConVar_ServerInfo, GameName::GTA5);

#ifdef STATE_RDR3
	if (gamename->GetValue() == GameName::RDR3)
	{
		return true;
	}
#endif

#ifdef STATE_FIVE
	if (gamename->GetValue() == GameName::GTA5)
	{
		return true;
	}
#endif

	return false;
}
