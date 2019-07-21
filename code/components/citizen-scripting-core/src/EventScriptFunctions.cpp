/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceScriptingComponent.h>

#include <fxScripting.h>

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("TRIGGER_EVENT_INTERNAL", [] (fx::ScriptContext& context)
	{
		static fx::ResourceManager* manager = fx::ResourceManager::GetCurrent(false);
		static fwRefContainer<fx::ResourceEventManagerComponent> eventManager = manager->GetComponent<fx::ResourceEventManagerComponent>();

		// trigger the event
		bool wasCanceled = eventManager->TriggerEvent(context.GetArgument<const char*>(0), std::string(context.GetArgument<const char*>(1), context.GetArgument<uint32_t>(2)));

		// set the result for convenience
		context.SetResult(wasCanceled);
	});

	fx::ScriptEngine::RegisterNativeHandler("CANCEL_EVENT", [] (fx::ScriptContext& context)
	{
		static fx::ResourceManager* manager = fx::ResourceManager::GetCurrent(false);
		static fwRefContainer<fx::ResourceEventManagerComponent> eventManager = manager->GetComponent<fx::ResourceEventManagerComponent>();

		eventManager->CancelEvent();
	});

	fx::ScriptEngine::RegisterNativeHandler("WAS_EVENT_CANCELED", [] (fx::ScriptContext& context)
	{
		static fx::ResourceManager* manager = fx::ResourceManager::GetCurrent(false);
		static fwRefContainer<fx::ResourceEventManagerComponent> eventManager = manager->GetComponent<fx::ResourceEventManagerComponent>();

		context.SetResult(eventManager->WasLastEventCanceled());
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_RESOURCE_AS_EVENT_HANDLER", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				resource->GetComponent<fx::ResourceScriptingComponent>()->AddHandledEvent(context.CheckArgument<const char*>(0));
			}
		}
	});
});
