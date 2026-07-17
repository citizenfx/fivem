/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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