#include "StdInc.h"

#include <ResourceEventComponent.h>
#include <ResourceManager.h>

#include <msgpack.hpp>

// Triggers a local, cancelable 'netEventReceived' server event for every event a client sends
// (TriggerServerEvent/TriggerLatentServerEvent), before any resource handles it - even if nothing is registered for it.
//
// AddEventHandler('netEventReceived', function(eventName, eventArgs, payloadSize)
//     -- source is the player that sent the event
//     -- CancelEvent() drops the original event
// end)
static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		auto eventManager = manager->GetComponent<fx::ResourceEventManagerComponent>();

		eventManager->OnTriggerEvent.Connect([eventManager](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
		{
			if (eventSource.rfind("net:", 0) != 0)
			{
				return true;
			}

			// don't decode the payload if no resource listens to 'netEventReceived'
			if (!eventManager->HasResourceHandledEvent("netEventReceived"))
			{
				return true;
			}

			msgpack::object eventArgs;
			msgpack::unpacked unpacked;

			try
			{
				unpacked = msgpack::unpack(eventPayload.data(), eventPayload.size());
				eventArgs = unpacked.get();
			}
			catch (const std::exception&)
			{
				// malformed payload, pass nil args
			}

			const bool allowed = eventManager->TriggerEvent2(
				"netEventReceived",
				{ "internal-" + eventSource },
				eventName,
				eventArgs,
				static_cast<uint32_t>(eventPayload.size())
			);

			if (!allowed)
			{
				*eventCanceled = true;
				return false;
			}

			return true;
		},
		// run before any other global handler, so it can't be bypassed
		INT32_MIN);
	});
});
