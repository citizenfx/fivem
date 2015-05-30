/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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