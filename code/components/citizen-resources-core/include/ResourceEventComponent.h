/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ComponentHolder.h>

#include <tbb/concurrent_queue.h>

#include <mutex>
#include <queue>
#include <stack>
#include <optional>

#include <msgpack.hpp>

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

public:
	ResourceEventComponent();

	inline ResourceEventManagerComponent* GetManager()
	{
		return m_managerComponent;
	}

	void HandleTriggerEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled);

	void QueueEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource = std::string());

public:
	//
	// Enqueues a formatted event on the resource.
	//
	template<typename... TArg>
	inline void QueueEvent2(const std::string_view& eventName, const std::optional<std::string_view>& targetSrc, const TArg&... args)
	{
		msgpack::sbuffer buf;
		msgpack::packer<msgpack::sbuffer> packer(buf);

		// pack the argument pack as array
		packer.pack_array(sizeof...(args));
		(packer.pack(args), ...);

		QueueEvent(std::string(eventName), std::string(buf.data(), buf.size()), std::string(targetSrc.value_or("")));
	}

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
		ResourceEventComponent* filter = nullptr;
	};

private:
	ResourceManager* m_manager;

	tbb::concurrent_queue<EventData> m_eventQueue;

private:
	void Tick();

public:
	ResourceEventManagerComponent();

	//
	// Returns whether or not the last event that completed fully was canceled.
	//
	bool WasLastEventCanceled();

	//
	// Cancel the current event.
	//
	void CancelEvent();

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
	// Triggers an event immediately. Returns a value indicating whether the event was not canceled.
	//
	bool TriggerEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource = std::string(), ResourceEventComponent* filter = nullptr);

	//
	// Enqueues an event for execution on the next resource manager tick.
	//
	void QueueEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource = std::string(), ResourceEventComponent* filter = nullptr);

public:
	//
	// Triggers a formatted event on the manager.
	//
	template<typename... TArg>
	inline bool TriggerEvent2(const std::string_view& eventName, const std::optional<std::string_view>& targetSrc, const TArg&... args)
	{
		msgpack::sbuffer buf;
		msgpack::packer<msgpack::sbuffer> packer(buf);

		// pack the argument pack as array
		packer.pack_array(sizeof...(args));
		(packer.pack(args), ...);

		return TriggerEvent(std::string(eventName), std::string(buf.data(), buf.size()), std::string(targetSrc.value_or("")));
	}

	//
	// Enqueues a formatted event on the manager.
	//
	template<typename... TArg>
	inline void QueueEvent2(const std::string_view& eventName, const std::optional<std::string_view>& targetSrc, const TArg&... args)
	{
		msgpack::sbuffer buf;
		msgpack::packer<msgpack::sbuffer> packer(buf);

		// pack the argument pack as array
		packer.pack_array(sizeof...(args));
		(packer.pack(args), ...);

		QueueEvent(std::string(eventName), std::string(buf.data(), buf.size()), std::string(targetSrc.value_or("")));
	}

	virtual void AttachToObject(ResourceManager* object) override;
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceEventComponent);
DECLARE_INSTANCE_TYPE(fx::ResourceEventManagerComponent);
