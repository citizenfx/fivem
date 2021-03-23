#pragma once

#ifdef COMPILING_GTA_CORE_NY
#define CORE_NY_EXPORT DLL_EXPORT
#else
#define CORE_NY_EXPORT DLL_IMPORT
#endif

enum class GameFlag
{
	NetworkWalkMode,
	PlayerActivated,
	InstantSendPackets
};

class CORE_NY_EXPORT GameFlags
{
public:
	static bool GetFlag(GameFlag flag);

	static void SetFlag(GameFlag flag, bool value);

	static void ResetFlags();
};
