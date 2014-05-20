#pragma once

#include "LauncherInterface.h"

class LauncherInterface : public ILauncherInterface
{
public:
	bool PreLoadGame(void* cefSandbox);

	bool PostLoadGame(HMODULE hModule, void(**entryPoint)());
};