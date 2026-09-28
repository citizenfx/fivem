#include "StdInc.h"

#include <CoreConsole.h>
#include <ResourceEventComponent.h>
#include <ResourceManager.h>
#include <ServerInstanceBase.h>

#include <msgpack.hpp>

static std::shared_ptr<ConVar<bool>> g_decodeArgsVar;

// Triggers a local, cancelable 'netEventReceived' server event for every event a client sends
// (TriggerServerEvent/TriggerLatentServerEvent), before any resource handles it - even if nothing is registered for it.
//
// AddEventHandler('netEventReceived', function(eventName, eventArgs, payloadSize)
//     -- source is the player that sent the event
//     -- eventArgs is nil if the payload is malformed, or if sv_netEventReceivedDecodeArgs is disabled
//     -- CancelEvent() drops the original event
// end)
static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		// handlers that only need the event name and size (e.g. to ban on trap events) can skip decoding the arguments
		g_decodeArgsVar = instance->AddVariable<bool>("sv_netEventReceivedDecodeArgs", ConVar_None, true);
	});

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

			if (!g_decodeArgsVar || g_decodeArgsVar->GetValue())
			{
				try
				{
					unpacked = msgpack::unpack(eventPayload.data(), eventPayload.size());
					eventArgs = unpacked.get();
				}
				catch (const std::exception&)
				{
					// malformed payload, pass nil args
				}
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
