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
#include <IteratorView.h>

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
		/*NETEV onResourceStarting SHARED
		/#*
		 * An event that is triggered when a resource is trying to start.
		 *
		 * This can be canceled to prevent the resource from starting.
		 *
		 * @param resource - The name of the resource that is trying to start.
		 #/
		declare function onResourceStarting(resource: string): void;
		*/
		return m_managerComponent->TriggerEvent2("onResourceStarting", {}, m_resource->GetName());
	}, -10000);

	object->OnStart.Connect([=] ()
	{
		/*NETEV onClientResourceStart CLIENT
		/#*
		 * An event that is *queued* after a resource has started.
		 *
		 * @param resource - The name of the resource that has started.
		 #/
		declare function onClientResourceStart(resource: string): void;
		*/
		/*NETEV onServerResourceStart SERVER
		/#*
		 * An event that is *queued* after a resource has started.
		 *
		 * @param resource - The name of the resource that has started.
		 #/
		declare function onServerResourceStart(resource: string): void;
		*/

		// on[type]ResourceStart is queued so that clients will only run it during the first tick
		m_managerComponent->QueueEvent2(fmt::sprintf("on%sResourceStart", IsServer() ? "Server" : "Client"), {}, m_resource->GetName());
	});

	object->OnStart.Connect([=]()
	{
		/*NETEV onResourceStart SHARED
		/#*
		 * An event that is triggered *immediately* when a resource has started.
		 *
		 * @param resource - The name of the resource that just started.
		 #/
		declare function onResourceStart(resource: string): void;
		*/
		m_managerComponent->TriggerEvent2("onResourceStart", {}, m_resource->GetName());
	}, 99999);

	object->OnStop.Connect([=] ()
	{
		/*NETEV onClientResourceStop CLIENT
		/#*
		 * An event that is triggered after a resource has stopped.
		 *
		 * @param resource - The name of the resource that has stopped.
		 #/
		declare function onClientResourceStop(resource: string): void;
		*/
		/*NETEV onServerResourceStop SERVER
		/#*
		 * An event that is triggered after a resource has stopped.
		 *
		 * @param resource - The name of the resource that has stopped.
		 #/
		declare function onServerResourceStop(resource: string): void;
		*/
		m_managerComponent->QueueEvent2(fmt::sprintf("on%sResourceStop", IsServer() ? "Server" : "Client"), {}, m_resource->GetName());
	});

	object->OnStop.Connect([=] ()
	{
		OnTriggerEvent.Reset();
	}, INT32_MAX);

	object->OnStop.Connect([=]()
	{
		/*NETEV onResourceStop SHARED
		/#*
		 * An event that is triggered *immediately* when a resource is stopping.
		 *
		 * @param resource - The name of the resource that is stopping.
		 #/
		declare function onResourceStop(resource: string): void;
		*/
		m_managerComponent->TriggerEvent2("onResourceStop", {}, m_resource->GetName());
	}, -99999);
}

void ResourceEventComponent::HandleTriggerEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
{
	// trigger event
	OnTriggerEvent(eventName, eventPayload, eventSource, eventCanceled);
}

void ResourceEventComponent::QueueEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource /* = std::string() */)
{
	m_managerComponent->QueueEvent(eventName, eventPayload, eventSource, this);
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
			TriggerEvent(event.eventName, event.eventPayload, event.eventSource, event.filter);
		}
	}
}

static thread_local bool g_wasLastEventCanceled;
static thread_local std::stack<bool*> g_eventCancelationStack;

void ResourceEventManagerComponent::AddResourceHandledEvent(const std::string& resourceName, const std::string& eventName)
{
	// try checking if this pair already exists
	for (const auto& pair : fx::GetIteratorView(m_eventResources.equal_range(eventName)))
	{
		if (pair.second == resourceName)
		{
			return;
		}
	}

	m_eventResources.emplace(eventName, resourceName);
}

bool ResourceEventManagerComponent::TriggerEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource /* = std::string() */, ResourceEventComponent* filter /* = nullptr*/)
{
	// add a value to signify event cancelation
	bool eventCanceled = false;

	g_eventCancelationStack.push(&eventCanceled);

	// trigger global handlers for the event
	if (OnTriggerEvent(eventName, eventPayload, eventSource, &eventCanceled))
	{
		// trigger local handlers
		auto forResource = [&](const fwRefContainer<Resource>& resource)
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
		};

		if (!filter)
		{
			for (const auto& eventKey : { std::string{ "*" }, eventName })
			{
				for (const auto& resourcePair : fx::GetIteratorView(m_eventResources.equal_range(eventKey)))
				{
					auto resource = m_manager->GetResource(resourcePair.second, false);

					if (resource.GetRef())
					{
						forResource(resource);
					}
				}
			}
		}
		else
		{
			filter->HandleTriggerEvent(eventName, eventPayload, eventSource, &eventCanceled);
		}
	}

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

void ResourceEventManagerComponent::QueueEvent(const std::string& eventName, const std::string& eventPayload, const std::string& eventSource /* = std::string() */, ResourceEventComponent* filter)
{
	EventData event;
	event.eventName = eventName;
	event.eventPayload = eventPayload;
	event.eventSource = eventSource;
	event.filter = filter;

	{
		m_eventQueue.push(event);
	}

	if (!filter)
	{
		// trigger global handlers for the queued event
		OnQueueEvent(eventName, eventPayload, eventSource);
	}
}

void ResourceEventManagerComponent::AttachToObject(ResourceManager* object)
{
	m_manager = object;

	m_manager->OnTick.Connect([=] ()
	{
		this->Tick();
	});

	m_manager->OnAfterReset.Connect([this]()
	{
		m_eventQueue.clear();
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
