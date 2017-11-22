/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

class ILauncherInterface
{
public:
	virtual bool PreInitializeGame() = 0;

	virtual bool PreLoadGame(void* cefSandbox) = 0;

	virtual bool PostLoadGame(HMODULE hModule, void(**entryPoint)()) = 0;

	virtual bool PreResumeGame() = 0;
};

typedef ILauncherInterface* (__cdecl * GetLauncherInterface_t)();