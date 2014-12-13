/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "CefOverlay.h"
#include "ResourceManager.h"

typedef std::function<void(std::string)> ResUIResultCallback;
typedef std::function<void(std::string, ResUIResultCallback)> ResUICallback;

class ResourceUI
{
private:
	Resource* m_resource;

	std::map<std::string, ResUICallback> m_callbacks;

public:
	ResourceUI(Resource* resource);

	virtual ~ResourceUI();

	bool Create();

	void Destroy();

	void AddCallback(std::string type, ResUICallback callback);

	bool InvokeCallback(std::string type, std::string data, ResUIResultCallback resultCB);

	void SignalPoll();
};