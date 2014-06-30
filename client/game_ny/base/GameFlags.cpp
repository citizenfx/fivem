#include "StdInc.h"
#include "GameFlags.h"

static std::unordered_map<GameFlag, bool> g_gameFlags;

void GameFlags::ResetFlags()
{
	SetFlag(GameFlag::NetworkWalkMode, true);
}

bool GameFlags::GetFlag(GameFlag flag)
{
	return g_gameFlags[flag];
}

void GameFlags::SetFlag(GameFlag flag, bool value)
{
	g_gameFlags[flag] = value;
}