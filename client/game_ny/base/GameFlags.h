#pragma once

enum class GameFlag
{
	NetworkWalkMode,
	PlayerActivated,
	InstantSendPackets
};

class GAMESPEC_EXPORT GameFlags
{
public:
	static bool GetFlag(GameFlag flag);

	static void SetFlag(GameFlag flag, bool value);

	static void ResetFlags();
};