/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include "ISteamClient.h"
#include "IClientEngine.h"

#include "InterfaceMapper.h"

class ISteamComponent
{
public:
	virtual bool IsSteamRunning() = 0;

	virtual ISteamClient* GetPublicClient() = 0;

	virtual IClientEngine* GetPrivateClient() = 0;

	virtual HSteamUser GetHSteamUser() = 0;

	virtual HSteamPipe GetHSteamPipe() = 0;

	virtual int RegisterSteamCallbackRaw(int callbackID, std::function<void(void*)> callback) = 0;

	virtual void RemoveSteamCallback(int registeredID) = 0;

	virtual int GetParentAppID() = 0;

	virtual void Initialize() = 0;

	virtual void SetConnectValue(const std::string& text) = 0;

	virtual void SetRichPresenceTemplate(const std::string& text) = 0;

	virtual void SetRichPresenceValue(int idx, const std::string& value) = 0;

// template bits
public:
	template<typename TCallback>
	int RegisterSteamCallback(std::function<void(TCallback*)> callback)
	{
		return RegisterSteamCallbackRaw(TCallback::k_iCallback, [=] (void* value)
		{
			callback(static_cast<TCallback*>(value));
		});
	}
};

DECLARE_INSTANCE_TYPE(ISteamComponent);