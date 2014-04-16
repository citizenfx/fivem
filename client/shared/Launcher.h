#pragma once

#include "LauncherInterface.h"

class LauncherInterface : public ILauncherInterface
{
public:
	bool PreLoadGame();

	bool PostLoadGame(HMODULE hModule, void(**entryPoint)());
};