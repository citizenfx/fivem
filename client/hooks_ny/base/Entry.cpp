#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "HookCallbacks.h"

class GameSpecToHooks : public IGameSpecToHooks
{
public:
	virtual void SetHookCallback(uint32_t hookCallbackId, void(*callback)(void*));
};

static GameSpecToHooks gameSpecEntry;
INetLibrary* g_netLibrary;

void HooksDLLInterface::PreGameLoad(bool* continueLoad)
{
	
}

sigslot::signal2<HMODULE, bool*> g_signalPostLoad;

void HooksDLLInterface::PostGameLoad(HMODULE module, bool* continueLoad)
{
	GameSpecDLLInterface::SetHooksDLLCallback(&gameSpecEntry);

	InitFunctionBase::RunAll();

	g_signalPostLoad.emit(module, continueLoad);

	DWORD oldProtect;
	VirtualProtect((void*)0x00EB9BE4, 0x50, PAGE_READONLY, &oldProtect);
}

void HooksDLLInterface::SetNetLibrary(INetLibrary* netLibrary)
{
	g_netLibrary = netLibrary;
}

void GameSpecToHooks::SetHookCallback(uint32_t hookCallbackId, void(*callback)(void*))
{
	HookCallbacks::AddCallback(hookCallbackId, callback);
}