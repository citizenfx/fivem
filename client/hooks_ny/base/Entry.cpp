#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "HookCallbacks.h"
#include "RuntimeHooks.h"

void SetDisconnectSafeguard(bool enable);

class GameSpecToHooks : public IGameSpecToHooks
{
public:
	virtual void SetHookCallback(uint32_t hookCallbackId, void(*callback)(void*));

	virtual void SetDisconnectSafeguard(bool enable);

	virtual bool InstallRuntimeHook(const char* key);

	virtual bool SetLimit(const char* limit, int value);

	virtual bool SetWorldDefinition(const char* worldDefinition);
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

bool GameSpecToHooks::InstallRuntimeHook(const char* key)
{
	RuntimeHookFunction::Run(key);

	return true;
}

void AdjustLimit(std::string limit, int value);
void SetWorldDefinition(std::string worldDefinition);

bool GameSpecToHooks::SetLimit(const char* limit, int value)
{
	::AdjustLimit(limit, value);

	return true;
}

bool GameSpecToHooks::SetWorldDefinition(const char* worldDefinition)
{
	::SetWorldDefinition(worldDefinition);

	return true;
}