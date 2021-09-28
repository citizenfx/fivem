/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "CefOverlay.h"
#include "ResourceManager.h"

using fx::Resource;

typedef std::function<void(int statusCode, const std::multimap<std::string, std::string>& headers, const std::string& body)> ResUIResultCallback;
typedef std::function<void(const std::string& path, const std::string& query, const std::multimap<std::string, std::string>& headers, const std::string& body, ResUIResultCallback)> ResUICallback;

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

	bool m_hasFrame;

	bool m_hasCallbacks;

	std::map<std::string, ResUICallback> m_callbacks;

public:
	ResourceUI(Resource* resource);

	virtual ~ResourceUI();

	inline bool HasFrame()
	{
		return m_hasFrame;
	}

	inline bool HasCallbacks()
	{
		return m_hasCallbacks;
	}

	inline void SetHasCallbacks(bool value)
	{
		m_hasCallbacks = value;
	}

	bool Create();

	void Destroy();

	void AddCallback(const std::string& type, ResUICallback callback);

	void RemoveCallback(const std::string& type);

	bool InvokeCallback(const std::string& type, const std::string& query, const std::multimap<std::string, std::string>& headers, const std::string& data, ResUIResultCallback resultCB);

	void SignalPoll();
};

DECLARE_INSTANCE_TYPE(ResourceUI);
