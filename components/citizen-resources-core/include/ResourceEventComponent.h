/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ComponentHolder.h>

#include <concurrent_queue.h>

#include <mutex>
#include <queue>
#include <stack>

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
class Resource;

class ResourceEventManagerComponent;

class RESOURCES_CORE_EXPORT ResourceEventComponent : public fwRefCountable, public IAttached<Resource>
{
private:
	Resource* m_resource;

	ResourceEventManagerComponent* m_managerComponent;

private:
	struct EventData
	{
		std::string eventName;
		std::string eventSource;
		std::string eventPayload;
	};

private:
	concurrency::concurrent_queue<EventData> m_eventQueue;

public:
	ResourceEventComponent();

	inline ResourceEventManagerComponent* GetManager()
	{
		return m_managerComponent;
	}

	void HandleTriggerEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled);

	void QueueEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource = std::string());

	virtual void AttachToObject(Resource* object) override;

public:
	//
	// An event to handle event execution externally.
	// Arguments: eventName, eventPayload, eventSource, eventCanceled
	//
	fwEvent<const std::string&, const std::string&, const std::string&, bool*> OnTriggerEvent;
};

class ResourceManager;

class RESOURCES_CORE_EXPORT ResourceEventManagerComponent : public fwRefCountable, public IAttached<ResourceManager>
{
private:
	struct EventData
	{
		std::string eventName;
		std::string eventSource;
		std::string eventPayload;
	};

private:
	ResourceManager* m_manager;

	concurrency::concurrent_queue<EventData> m_eventQueue;

	std::stack<bool*> m_eventCancelationStack;

	bool m_wasLastEventCanceled;

private:
	void Tick();

public:
	ResourceEventManagerComponent();

	//
	// Returns whether or not the last event that completed fully was canceled.
	//
	inline bool WasLastEventCanceled()
	{
		return m_wasLastEventCanceled;
	}

	//
	// Cancel the current event.
	//
	inline void CancelEvent()
	{
		*(m_eventCancelationStack.top()) = true;
	}

	//
	// An event to handle event execution externally.
	// Arguments: eventName, eventPayload, eventSource, eventCanceled
	//
	fwEvent<const std::string&, const std::string&, const std::string&, bool*> OnTriggerEvent;

	//
	// An event to handle event execution externally, before the event is running on the main thread.
	// Arguments: eventName, eventPayload, eventSource
	//
	fwEvent<const std::string&, const std::string&, const std::string&> OnQueueEvent;

	//
	// Triggers an event immediately. Returns whether or not the event was canceled.
	//
	bool TriggerEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource = std::string());

	//
	// Enqueues an event for execution on the next resource manager tick.
	//
	void QueueEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource = std::string());

	virtual void AttachToObject(ResourceManager* object) override;
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceEventComponent);
DECLARE_INSTANCE_TYPE(fx::ResourceEventManagerComponent);