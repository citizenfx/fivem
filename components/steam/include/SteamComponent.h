/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "SteamLoader.h"
#include "IClientEngine.h"
#include "ISteamClient.h"

#include "SteamComponentAPI.h"

#include <mutex>

class SteamComponent : public ISteamComponent
{
private:
	SteamLoader m_steamLoader;

	ISteamClient* m_client;

	IClientEngine* m_clientEngine;

	HSteamPipe m_steamPipe;
	HSteamUser m_steamUser;

	std::recursive_mutex m_callbackMutex;

	std::multimap<int, std::pair<int, std::function<void(void*)>>> m_userCallbacks;

	int m_callbackIndex;

	int m_parentAppID;

private:
	void InitializeClientAPI();

	void InitializePublicAPI();

	bool RunPresenceDummy();

	void InitializePresence();

	void RunThread();

public:
	SteamComponent();

	void Initialize();

	// ISteamComponent implementation
public:
	virtual bool IsSteamRunning() override;

	virtual ISteamClient* GetPublicClient() override;

	virtual IClientEngine* GetPrivateClient() override;

	virtual HSteamUser GetHSteamUser() override;

	virtual HSteamPipe GetHSteamPipe() override;

	virtual int RegisterSteamCallbackRaw(int callbackID, std::function<void(void*)> callback) override;

	virtual void RemoveSteamCallback(int registeredID) override;

	virtual int GetParentAppID() override;
};