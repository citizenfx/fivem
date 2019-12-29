/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceEventComponent.h"

#include <Resource.h>
#include <ResourceManager.h>

#include <msgpack.hpp>

#include <DebugAlias.h>

static inline bool IsServer()
{
#ifdef IS_FXSERVER
	return true;
#else
	return false;
#endif
}

namespace fx
{
ResourceEventComponent::ResourceEventComponent()
{

}

void ResourceEventComponent::AttachToObject(Resource* object)
{
	m_resource = object;

	m_managerComponent = m_resource->GetManager()->GetComponent<ResourceEventManagerComponent>().GetRef();

	// start/stop handling events
	object->OnBeforeStart.Connect([=] ()
	{
		// pack the resource name
		msgpack::sbuffer buf;
		msgpack::packer<msgpack::sbuffer> packer(buf);

		// array of a single string
		packer.pack_array(1);
		packer.pack(m_resource->GetName());

		// send the event out to the world
		std::string event(buf.data(), buf.size());

		return m_managerComponent->TriggerEvent("onResourceStarting", event);
	}, -10000);

	object->OnStart.Connect([=] ()
	{
		// on[type]ResourceStart is queued so that clients will only run it during the first tick
		m_managerComponent->QueueEvent2(fmt::sprintf("on%sResourceStart", IsServer() ? "Server" : "Client"), {}, m_resource->GetName());
	});

	object->OnStart.Connect([=]()
	{
		m_managerComponent->TriggerEvent2("onResourceStart", {}, m_resource->GetName());
	}, 99999);

	object->OnStop.Connect([=] ()
	{
		m_managerComponent->QueueEvent2(fmt::sprintf("on%sResourceStop", IsServer() ? "Server" : "Client"), {}, m_resource->GetName());
	});

	object->OnStop.Connect([=] ()
	{
		OnTriggerEvent.Reset();
	}, INT32_MAX);

	object->OnStop.Connect([=]()
	{
		m_managerComponent->TriggerEvent2("onResourceStop", {}, m_resource->GetName());
	}, -99999);

	object->OnTick.Connect([=] ()
	{
		// take queued events and trigger them
		while (!m_eventQueue.empty())
		{
			// get the event
			EventData event;

			if (m_eventQueue.try_pop(event))
			{
				// and trigger it
				bool canceled = false;

				HandleTriggerEvent(event.eventName, event.eventPayload, event.eventSource, &canceled);
			}
		}
	});
}

void ResourceEventComponent::HandleTriggerEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
{
	// save data in case we need to trace this back from dumps
	char resourceNameBit[128];
	char eventNameBit[128];

	strncpy(resourceNameBit, m_resource->GetName().c_str(), std::size(resourceNameBit));
	strncpy(eventNameBit, eventName.c_str(), std::size(eventNameBit));
	
	debug::Alias(resourceNameBit);
	debug::Alias(eventNameBit);

	// trigger event
	OnTriggerEvent(eventName, eventPayload, eventSource, eventCanceled);
}

void ResourceEventComponent::QueueEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource /* = std::string() */)
{
	EventData event;
	event.eventName = eventName;
	event.eventPayload = eventPayload;
	event.eventSource = eventSource;

	{
		m_eventQueue.push(event);
	}
}

ResourceEventManagerComponent::ResourceEventManagerComponent()
{

}

void ResourceEventManagerComponent::Tick()
{
	// take queued events and trigger them
	while (!m_eventQueue.empty())
	{
		// get the event
		EventData event;
		
		if (m_eventQueue.try_pop(event))
		{
			// and trigger it
			TriggerEvent(event.eventName, event.eventPayload, event.eventSource);
		}
	}
}

static thread_local bool g_wasLastEventCanceled;
static thread_local std::stack<bool*> g_eventCancelationStack;

bool ResourceEventManagerComponent::TriggerEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource /* = std::string() */)
{
	// add a value to signify event cancelation
	bool eventCanceled = false;

	g_eventCancelationStack.push(&eventCanceled);

	// trigger global handlers for the event
	OnTriggerEvent(eventName, eventPayload, eventSource, &eventCanceled);

	// trigger local handlers
	m_manager->ForAllResources([&] (const fwRefContainer<Resource>& resource)
	{
		// get the event component
		const fwRefContainer<ResourceEventComponent>& eventComponent = resource->GetComponent<ResourceEventComponent>();

		// if there's none, return
		if (!eventComponent.GetRef())
		{
			trace("no event component for resource %s\n", resource->GetName().c_str());
			return;
		}

		// continue on
		eventComponent->HandleTriggerEvent(eventName, eventPayload, eventSource, &eventCanceled);
	});

	// pop the stack entry
	g_eventCancelationStack.pop();

	// set state
	g_wasLastEventCanceled = eventCanceled;

	// return whether it was *not* canceled
	return !eventCanceled;
}

bool ResourceEventManagerComponent::WasLastEventCanceled()
{
	return g_wasLastEventCanceled;
}

void ResourceEventManagerComponent::CancelEvent()
{
	if (!g_eventCancelationStack.empty())
	{
		*(g_eventCancelationStack.top()) = true;
	}
}

void ResourceEventManagerComponent::QueueEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource /* = std::string() */)
{
	EventData event;
	event.eventName = eventName;
	event.eventPayload = eventPayload;
	event.eventSource = eventSource;

	{
		m_eventQueue.push(event);
	}

	// trigger global handlers for the queued event
	OnQueueEvent(eventName, eventPayload, eventSource);
}

void ResourceEventManagerComponent::AttachToObject(ResourceManager* object)
{
	m_manager = object;

	m_manager->OnTick.Connect([=] ()
	{
		this->Tick();
	});
}

static InitFunction initFunction([] ()
{
	Resource::OnInitializeInstance.Connect([] (Resource* resource)
	{
		resource->SetComponent<ResourceEventComponent>(new ResourceEventComponent());
	});

	ResourceManager::OnInitializeInstance.Connect([] (ResourceManager* manager)
	{
		manager->SetComponent<ResourceEventManagerComponent>(new ResourceEventManagerComponent());
	});
});
}
