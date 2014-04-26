#pragma once

class GAMESPEC_EXPORT GameInit
{
public:
	static bool GetGameLoaded();

	static void LoadGameFirstLaunch();

	static void ReloadGame();
};