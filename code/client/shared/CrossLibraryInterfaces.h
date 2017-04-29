/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

// library for game-specific network integration code to interface with the CitizenGame network library
class INetLibrary
{
public:
	virtual uint16_t GetServerNetID() = 0;

	virtual uint16_t GetHostNetID() = 0;

	virtual uint32_t GetHostBase() = 0;

	virtual const char* GetPlayerName() = 0;

	virtual void SetPlayerName(const char* name) = 0;

	virtual void SetBase(uint32_t base) = 0;

	virtual void RunFrame() = 0;

	virtual void ConnectToServer(const char* hostname, uint16_t port) = 0;

	virtual void Disconnect(const char* reason) = 0;

	// called from hooks for the actual disconnect event
	virtual void FinalizeDisconnect() = 0;

	virtual bool DequeueRoutedPacket(char* buffer, size_t* length, uint16_t* netID) = 0;

	virtual void RoutePacket(const char* buffer, size_t length, uint16_t netID) = 0;

	virtual void SendReliableCommand(const char* type, const char* buffer, size_t length) = 0;
};

struct WNDPROCARGS
{
	HWND hwnd;
	UINT uMsg;
	LPARAM lParam;
	WPARAM wParam;
	BOOL pass;
	LRESULT lresult;
};

class IGameSpecToHooks
{
public:
	virtual void SetHookCallback(uint32_t hookCallbackId, void(*callback)(void*)) = 0;

	virtual void SetDisconnectSafeguard(bool enable) = 0;

	virtual bool InstallRuntimeHook(const char* key) = 0;

	virtual bool SetLimit(const char* limit, int value) = 0;

	virtual bool SetWorldDefinition(const char* worldDefinition) = 0;
};

class HOOKS_EXPORT HooksDLLInterface
{
public:
	static void PreGameLoad(bool* continueLoad, IGameSpecToHooks** hooksPtr);

	static void PostGameLoad(HMODULE module, bool* continueLoad);

	static void SetNetLibrary(INetLibrary* netLibrary);
};

class GAMESPEC_EXPORT GameSpecDLLInterface
{
public:
	static void SetHooksDLLCallback(IGameSpecToHooks* callback);

	static void SetNetLibrary(INetLibrary* netLibrary);
};

#if defined(COMPILING_HOOKS) || defined(COMPILING_GAMESPEC)
extern INetLibrary* g_netLibrary;
#endif

// the latter is just a temporary hack
#if defined(COMPILING_GAMESPEC) || defined(COMPILING_GAME) || defined(COMPILING_SCRT_LUA)
// hooks dll reverse callbacks
extern IGameSpecToHooks* g_hooksDLL;
#endif