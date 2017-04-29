/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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