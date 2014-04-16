#pragma once

typedef void(*HookCallback_t)(void* argument);

class HookCallbacks
{
public:
	static void AddCallback(uint32_t hookCallbackId, HookCallback_t callback);

	static void RunCallback(uint32_t hookCallbackId, void* argument);
};