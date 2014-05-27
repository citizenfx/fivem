#pragma once

#include "../ui/CefOverlay.h"
#include "ResourceManager.h"

typedef std::function<void(std::string)> ResUIResultCallback;
typedef std::function<void(std::string, ResUIResultCallback)> ResUICallback;

class ResourceUI
{
private:
	Resource* m_resource;

	std::shared_ptr<NUIWindow> m_window;

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