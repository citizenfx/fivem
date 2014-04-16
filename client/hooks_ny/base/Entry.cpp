#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "HookCallbacks.h"

class GameSpecToHooks : public IGameSpecToHooks
{
public:
	virtual void SetHookCallback(uint32_t hookCallbackId, void(*callback)(void*));
};

static GameSpecToHooks gameSpecEntry;

void HooksDLLInterface::PreGameLoad(bool* continueLoad)
{
	
}

sigslot::signal2<HMODULE, bool*> g_signalPostLoad;

void HooksDLLInterface::PostGameLoad(HMODULE module, bool* continueLoad)
{
	GameSpecDLLInterface::SetHooksDLLCallback(&gameSpecEntry);

	InitFunctionBase::RunAll();

	g_signalPostLoad.emit(module, continueLoad);
}

void GameSpecToHooks::SetHookCallback(uint32_t hookCallbackId, void(*callback)(void*))
{
	HookCallbacks::AddCallback(hookCallbackId, callback);
}