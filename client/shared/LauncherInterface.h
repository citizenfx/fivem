#pragma once

class ILauncherInterface
{
public:
	virtual bool PreLoadGame(void* cefSandbox) = 0;

	virtual bool PostLoadGame(HMODULE hModule, void(**entryPoint)()) = 0;
};

typedef ILauncherInterface* (__cdecl * GetLauncherInterface_t)();