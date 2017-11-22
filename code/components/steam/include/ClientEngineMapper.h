/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#ifdef COMPILING_STEAM
#define STEAM_EXPORT __declspec(dllexport)
#else
#define STEAM_EXPORT
#endif

#include <IClientEngine.h>
#include <InterfaceMapperBase.h>

#include <mutex>

class ClientEngineMapper : public InterfaceMapperBase
{
public:
	ClientEngineMapper(void* clientEngine, HSteamPipe steamPipe, HSteamUser steamUser);

private:
	HSteamPipe m_pipe;

	HSteamUser m_user;

	std::unordered_map<std::string, void*> m_interfaces;

	std::recursive_mutex m_mutex;

private:
	bool IsMethodAnInterface(void* method, bool* isUser);

	void LookupMethods();

public:
	void* GetInterface(const std::string& interfaceName);
};