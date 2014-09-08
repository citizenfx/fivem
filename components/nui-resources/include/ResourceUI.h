#pragma once

#include "CefOverlay.h"
#include "ResourceManager.h"

typedef fwAction<fwString> ResUIResultCallback;
typedef fwAction<fwString, ResUIResultCallback> ResUICallback;

class
#ifdef COMPILING_NUI_RESOURCES
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	ResourceUI : public fwRefCountable
{
private:
	Resource* m_resource;

	std::map<fwString, ResUICallback> m_callbacks;

public:
	ResourceUI(Resource* resource);

	virtual ~ResourceUI();

	bool Create();

	void Destroy();

	void AddCallback(fwString type, ResUICallback callback);

	bool InvokeCallback(fwString type, fwString data, ResUIResultCallback resultCB);

	void SignalPoll();

public:
	static fwRefContainer<ResourceUI> GetForResource(fwRefContainer<Resource> resource);
};