/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include "LauncherInterface.h"

class LauncherInterface : public ILauncherInterface
{
public:
	bool PreLoadGame(void* cefSandbox);

	bool PostLoadGame(HMODULE hModule, void(**entryPoint)());

	bool PreResumeGame();

	bool PreInitializeGame();
};