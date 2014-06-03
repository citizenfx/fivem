#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "HookCallbacks.h"

void SetDisconnectSafeguard(bool enable);

class GameSpecToHooks : public IGameSpecToHooks
{
public:
	virtual void SetHookCallback(uint32_t hookCallbackId, void(*callback)(void*));

	virtual void SetDisconnectSafeguard(bool enable);
};

static GameSpecToHooks gameSpecEntry;
INetLibrary* g_netLibrary;

void HooksDLLInterface::PreGameLoad(bool* continueLoad, IGameSpecToHooks** hooksPtr)
{
	*hooksPtr = &gameSpecEntry;
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

void GameSpecToHooks::SetDisconnectSafeguard(bool enable)
{
	::SetDisconnectSafeguard(enable);
}