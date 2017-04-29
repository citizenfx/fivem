/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("TRIGGER_EVENT_INTERNAL", [] (fx::ScriptContext& context)
	{
		// TODO: handle multiple resource managers for server
		static fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();
		static fwRefContainer<fx::ResourceEventManagerComponent> eventManager = manager->GetComponent<fx::ResourceEventManagerComponent>();

		// trigger the event
		bool wasCanceled = eventManager->TriggerEvent(context.GetArgument<const char*>(0), std::string(context.GetArgument<const char*>(1), context.GetArgument<uint32_t>(2)));

		// set the result for convenience
		context.SetResult(wasCanceled);
	});

	fx::ScriptEngine::RegisterNativeHandler("CANCEL_EVENT", [] (fx::ScriptContext& context)
	{
		// TODO: handle multiple resource managers for server
		static fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();
		static fwRefContainer<fx::ResourceEventManagerComponent> eventManager = manager->GetComponent<fx::ResourceEventManagerComponent>();

		eventManager->CancelEvent();
	});

	fx::ScriptEngine::RegisterNativeHandler("WAS_EVENT_CANCELED", [] (fx::ScriptContext& context)
	{
		// TODO: handle multiple resource managers for server
		static fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();
		static fwRefContainer<fx::ResourceEventManagerComponent> eventManager = manager->GetComponent<fx::ResourceEventManagerComponent>();

		context.SetResult(eventManager->WasLastEventCanceled());
	});
});