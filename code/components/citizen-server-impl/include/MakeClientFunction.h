#pragma once

#include <ScriptEngine.h>

#include <ClientRegistry.h>
#include <ResourceManager.h>
#include <ServerInstanceBaseRef.h>

template<typename TFn>
inline auto MakeClientFunction(TFn fn, uintptr_t defaultValue = 0)
{
	return [=](fx::ScriptContext & context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's client registry
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

		// parse the client ID
		const char* id = context.CheckArgument<const char*>(0);

		if (!id)
		{
			context.SetResult(defaultValue);
			return;
		}

		uint32_t netId = atoi(id);

		auto client = clientRegistry->GetClientByNetID(netId);

		if (!client)
		{
			context.SetResult(defaultValue);
			return;
		}

		context.SetResult(fn(context, client));
	};
};
