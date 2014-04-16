#pragma once

class HOOKS_EXPORT HooksDLLInterface
{
public:
	static void PreGameLoad(bool* continueLoad);

	static void PostGameLoad(HMODULE module, bool* continueLoad);
};

class IGameSpecToHooks
{
public:
	virtual void SetHookCallback(uint32_t hookCallbackId, void(*callback)(void*)) = 0;
};

class GAMESPEC_EXPORT GameSpecDLLInterface
{
public:
	static void SetHooksDLLCallback(IGameSpecToHooks* callback);
};