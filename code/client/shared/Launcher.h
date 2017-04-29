/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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